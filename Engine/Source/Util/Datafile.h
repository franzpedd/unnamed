#pragma once

#include "Core/Defines.h"
#include <sstream>
#include <functional>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector> 
#include <stack>

namespace Cosmos
{
	class COSMOS_API Datafile
	{
	private:

		// helper to write to files
		struct Writer
		{
			std::ofstream& file;
			int32_t indentationLevel;
			std::string indentation;
			char separator;

			// constructor
			Writer(std::ofstream& file, int32_t indentationLevel = 0, const std::string indentation = "\t", char separator = ',')
				: file(file), indentation(indentation), separator(separator), indentationLevel(0)
			{
			}
		};

	public: // functions

		// writes a data file to a file  (THIS IS CURRENTLY NOT PORTABLE)
		static inline bool Write(const Datafile& dataFile, const std::string& path, char separator = ',')
		{
			std::ofstream file(path);
			if (file.is_open()) {
				Writer writer(file, 0, "\t", separator);
				WriteRecursively(dataFile, writer);

				file.close();
				return true;
			}
			return false;
		}

		// reads data from a file
		static inline bool Read(Datafile& dataFile, const std::string& path, char separator = ',')
		{
			std::ifstream file(path);
			if (file.is_open()) {
				// variables may be outside the loop and we may need to refer to previous iterations
				std::string propName = {};
				std::string propValue = {};

				// using a stack to handle the reading
				// may re-factor this later
				std::stack<std::reference_wrapper<Datafile>> dfStack;
				dfStack.push(dataFile);

				while (!file.eof()) {
					std::string line;
					std::getline(file, line);

					RemoveWhiteSpaces(line);

					// line is not empty
					if (line.empty()) {
						continue;
					}

					// test if it's a comment
					if (line[0] == '#') {
						Datafile comment;
						comment.mIsComment = true;
						dfStack.top().get().mObjectVec.push_back({ line, comment });

						continue;
					}

					// check if equals symbol exists, if it does it is a property
					size_t x = line.find_first_of('=');
					if (x != std::string::npos) {
						propName = line.substr(0, x);
						RemoveWhiteSpaces(propName);

						propValue = line.substr(x + 1, line.size());
						RemoveWhiteSpaces(propValue);

						// elements may contain quotes and separations, must deal with this particularity
						bool inQuotes = false;
						std::string token = {};
						size_t tokenCount = 0;

						for (const auto c : propValue) {
							if (c == '\"') {
								inQuotes = true;
							}

							else {
								// it's in quotes, appends to a string
								if (inQuotes) {
									token.append(1, c);
								}

								else {
									// char is the separator, register the new property
									if (c == separator) {
										RemoveWhiteSpaces(token);
										dfStack.top().get()[propName].SetString(token, tokenCount);

										token.clear();
										tokenCount++;
									}

									// char is part of the token, appends to a string
									else {
										token.append(1, c);
									}
								}
							}
						}

						// any left char makes the final token, used to handle any mistake in the file avoiding crashes
						if (!token.empty()) {
							RemoveWhiteSpaces(token);
							dfStack.top().get()[propName].SetString(token, tokenCount);
						}
					}

					else { // no ' = ' sign 
						// the previous property is the new node
						if (line[0] == '{') {
							dfStack.push(dfStack.top().get()[propName]);
						}

						else {
							// node has been finished, pop it from the stack
							if (line[0] == '}') {
								dfStack.pop();
							}

							// line is a property with no assignment 
							else {
								propName = line;
							}
						}
					}
				}
				file.close();
				return true;
			}
			return false;
		}

	public: // operator overloading

		inline Datafile& operator[](const std::string& name)
		{
			// node map already does not contains an object with this name
			if (mObjectMap.count(name) == 0)
			{
				// create the object in the map and create a new empty DataFile on the object vector
				mObjectMap[name] = mObjectVec.size();
				mObjectVec.push_back({ name, Datafile() });
			}

			// returns the object by it's map index
			return mObjectVec[mObjectMap[name]].second;
		}

		// trying to access a children node
		inline Datafile& operator[](const size_t& index)
		{
			if (index > mObjectVec.size())
			{
				std::cerr << "Out of bounds, children doesnt exists" << std::endl;
			}

			return mObjectVec[index].second;
		}

	public: // utils

		// returns the number of values a property has
		inline size_t GetValueCount() const
		{
			return mContent.size();
		}

		// returns the number of children this node has
		inline size_t GetChildrenCount() const
		{
			return mObjectVec.size();
		}

		// returns if either a property of this node exists or not
		inline bool Exists(std::string property) const
		{
			return mObjectMap.find(property) != mObjectMap.end() ? true : false;
		}

	public: // getters and setters

		// sets a new string value of a property
		inline void SetString(const std::string& str, const size_t count = 0)
		{
			if (count >= mContent.size()) {
				mContent.resize(count + 1);
			}

			mContent[count] = str;
		}

		// returns a string value of a property
		inline const std::string GetString(const size_t count = 0) const
		{
			if (count >= mContent.size()) {
				return "";
			}

			return mContent[count];
		}

		// sets a new double value of a property
		inline void SetDouble(const double d, const size_t count = 0)
		{
			SetString(std::to_string(d), count);
		}

		// returns the double value of a property
		inline const double GetDouble(const size_t count = 0) const
		{
			return std::atof(GetString(count).c_str());
		}

		// sets a new integer value of a property
		inline void SetInt(const int32_t i, size_t count = 0)
		{
			SetString(std::to_string(i), count);
		}

		// returns the integer value of a property 
		const int32_t GetInt(size_t count = 0) const
		{
			return std::atoi(GetString(count).c_str());
		}

	private:

		// recursively writes to a data file to a file
		static inline void WriteRecursively(const Datafile& dataFile, Writer& writer)
		{
			const std::string separatorStr = std::string(1, writer.separator) + " ";

			// iterate through each property of tthis DataFile node
			for (auto const& prop : dataFile.mObjectVec) {
				// property doesnt contain any children, so it's an assignment
				if (prop.second.mObjectVec.empty()) {
					writer.file << Indentation(writer.indentation, writer.indentationLevel) << prop.first << (prop.second.mIsComment ? "" : " = ");

					size_t nItems = prop.second.GetValueCount();
					for (size_t i = 0; i < prop.second.GetValueCount(); i++) {
						// ensures the separator is written in quotations if it exists in the list of elements
						size_t x = prop.second.GetString(i).find_first_of(writer.separator);

						if (x != std::string::npos) {
							writer.file << "\"" << prop.second.GetString(i) << "\"" << ((nItems > 1) ? separatorStr : "");
						}

						else {
							writer.file << prop.second.GetString(i) << ((nItems > 1) ? separatorStr : "");
						}

						nItems--;
					}

					// property handled, move to next line
					writer.file << "\n";
				}

				// property has children
				else {
					writer.file << Indentation(writer.indentation, writer.indentationLevel) << prop.first << "\n";
					writer.file << Indentation(writer.indentation, writer.indentationLevel) << "{\n";
					writer.indentationLevel++;

					// recusisvely writes that node
					WriteRecursively(prop.second, writer);

					writer.file << Indentation(writer.indentation, writer.indentationLevel) << "}\n";
				}
			}

			// decrease indentation for the node
			if (writer.indentationLevel > 0) {
				writer.indentationLevel--;
			}
		}

		// returns the indentation level stringified
		static inline std::string Indentation(const std::string& str, const size_t count)
		{
			std::string res = {};

			for (size_t i = 0; i < count; i++) {
				res += str;
			}

			return res;
		}

		// removes the white spaces of a string
		static void RemoveWhiteSpaces(std::string& str)
		{
			str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
			str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
		}

	protected:

		bool mIsComment = false; // used to identify if the property is a comment or not

	private:

		std::vector<std::string> mContent; // the items of this serializer
		std::vector<std::pair<std::string, Datafile>> mObjectVec; // child nodes of this datafile
		std::unordered_map<std::string, size_t>  mObjectMap;
	};
}