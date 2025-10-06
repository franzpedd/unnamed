#pragma once

#include "Core/Defines.h"
#include <unordered_map>
#include <string>
#include <utility>
#include <optional>

namespace Cosmos
{
    template<typename Key, typename Value>
    class COSMOS_API Library
    {
    public:
        
        /// @brief constructor
        Library() = default;

        /// @brief destructor
        ~Library() = default;
        
        /// @brief returns a const reference to all objects inside the library
        inline const std::unordered_map<Key, Value>& GetAllRef() const { return mContent; }

        /// @brief returns the library size
        inline size_t Size() const { return mContent.size(); }

        /// @brief returns if the library is currently empty
        inline bool Empty() const { return mContent.empty(); }
        
        /// @bief returns if the library has a given key
        inline bool Contains(const Key& key) const { return mContent.find(key) != mContent.end();  }
        
        /// @brief returns the content's reference at a specific key
        inline Value& At(const Key& key) {  return mContent.at(key); }
        
        /// @brief returns the content's const reference at a specific key
        inline const Value& At(const Key& key) const { return mContent.at(key); }

        /// @brief erases a value with associated key, returns true uppon success
        inline bool Erase(const Key& key) { return mContent.erase(key) > 0; }
        
        /// @brief erases all contents from the library
        inline void Clear() { mContent.clear(); }

    public:

        /// @brief access with bounds checking
        inline std::optional<Value> Get(const Key& key) const
        {
            auto it = mContent.find(key);
            return it != mContent.end() ? std::optional<Value>(it->second) : std::nullopt;
        }
        
        /// @brief returns the object pointer or null if not existent
        inline Value* TryGet(const Key& key)
        {
            auto it = mContent.find(key);
            return it != mContent.end() ? &it->second : nullptr;
        }

        // @brief inserts an item into the library
        inline bool Insert(const Key& key, Value value)
        {
            return mContent.emplace(key, std::move(value)).second;
        }
        
        /// @brief inserts or modifies a value into a key, returns true if inserted, false if assigned
        inline bool InsertOrAssign(const Key& key, Value value) {
            auto result = mContent.insert_or_assign(key, std::move(value));
            return result.second;
        }

    public:

        /// @brief iterators
        inline auto begin() { return mContent.begin(); }
        inline auto end() { return mContent.end(); }

        /// @brief const iterators
        inline auto cbegin() const { return mContent.cbegin(); }
        inline auto cend() const { return mContent.cend(); }

    private:
        std::unordered_map<Key, Value> mContent;
    };
}