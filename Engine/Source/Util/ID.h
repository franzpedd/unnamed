#pragma once

#include "Core/Defines.h"
#include <atomic>
#include <mutex>
#include <unordered_set>

namespace Cosmos
{
    class COSMOS_API ID
    {
    public:
        
        /// @brief constructor
        constexpr ID(uint32_t val = 0) : value(val) {}

        /// @brief destructor
        ~ID() = default;

        // returns the id value
        uint32_t GetValue() const { return value; }

        /// @brief comparison operators for usability
        bool operator==(const ID& other) const { return value == other.value; }
        bool operator!=(const ID& other) const { return value != other.value; }

        /// @brief returns the null id
        static constexpr ID Null() { return ID{ 0 }; }

    private:
        uint32_t value;
    };

    class COSMOS_API IDGenerator
    {
    public:

        /// @brief creates an ID
        ID Create()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            uint32_t newId = ++mCounter;

            // prevent overflow
            if (newId == 0 || newId > MAX_ID) throw std::runtime_error("ID overflow");

            mActiveIds.insert(newId);
            return ID{ newId };
        }

        /// @brief creates an ID with a specific value
        bool Create(uint32_t idValue)
        {
            if (idValue == 0 || idValue > MAX_ID) return false;

            std::lock_guard<std::mutex> lock(mMutex);
            if (!mActiveIds.insert(idValue).second) return false; // already exists

            // update counter to avoid reusing lower IDs
            if (idValue > mCounter) mCounter = idValue;
            return true;
        }

        /// @brief removes an id
        bool Destroy(ID id)
        {
            if (id.GetValue() == 0) return false;

            std::lock_guard<std::mutex> lock(mMutex);
            return mActiveIds.erase(id.GetValue()) > 0;
        }

        /// @brief checks if id is active
        bool IsValid(ID id) const
        {
            if (id.GetValue() == 0) return false;

            std::lock_guard<std::mutex> lock(mMutex);
            return mActiveIds.find(id.GetValue()) != mActiveIds.end();
        }

        /// @brief clear all id's in use
        void Reset()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCounter = 0;
            mActiveIds.clear();
        }

    private:

        static constexpr uint32_t MAX_ID = std::numeric_limits<uint32_t>::max() - 1;
        uint32_t mCounter = 0;
        std::unordered_set<uint32_t> mActiveIds;
        mutable std::mutex mMutex;
    };
}