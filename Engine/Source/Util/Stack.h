#pragma once

#include <algorithm>
#include <mutex>
#include <vector>

namespace Cosmos
{
	// thread safe implementation of a stack using std::vector
	template<typename T>
	class Stack
	{
	public:

		/// @brief constructor
		Stack() = default;

		/// @brief destructor
		~Stack() = default;

		/// @brief returns a reference to the elements vector
		inline std::vector<T>& GetElementsRef() { return mElements; }

	public:

		/// @brief returns if queue is empty
		bool Empty()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			return mElements.empty();
		}

		/// @brief returns the queue size
		int Size()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			return mElements.size();
		}

		/// @brief an element to the top half of the stack
		void PushOver(T ent)
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mElements.emplace_back(ent);
		}

		/// @brief pops an element from the top half of the stack
		void PopOver(T ent)
		{
			std::unique_lock<std::mutex> lock(mMutex);

			auto it = std::find(mElements.begin() + mMiddlePos, mElements.end(), ent);
			if (it != mElements.end())
				mElements.erase(it);
		}

		/// @brief pushes an element to the bottom half of the stack
		void Push(T ent)
		{
			std::unique_lock<std::mutex> lock(mMutex);

			mElements.emplace(mElements.begin() + mMiddlePos, ent);
			mMiddlePos++;
		}

		/// @brief pops an element from the bottom half of the stack
		void Pop(T ent)
		{
			std::unique_lock<std::mutex> lock(mMutex);

			auto it = std::find(mElements.begin(), mElements.begin() + mMiddlePos, ent);
			if (it != mElements.begin() + mMiddlePos)
			{
				mElements.erase(it);
				mMiddlePos--;
			}
		}

	private:

		std::mutex mMutex;
		std::vector<T> mElements;
		uint32_t mMiddlePos = 0;
	};
}