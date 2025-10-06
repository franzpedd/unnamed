#pragma once

#include "Core/Defines.h"
#include <algorithm>
#include <mutex>
#include <optional>
#include <vector>

namespace Cosmos
{
    template<typename T>
    class COSMOS_API DualContainer
    {
    public:

        /// @brief constructor
        DualContainer() = default;

        /// @brief destructor
        ~DualContainer() = default;

    public:

        /// @brief returns if container is empty
        inline bool Empty() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mElements.empty();
        }

        /// @brief returns the container size
        inline size_t Size() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mElements.size();
        }

        /// @brief adds an element to the top half (move version)
        inline void PushToTop(const T& element)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mElements.emplace_back(element);
        }

        /// @brief adds an element to the bottom half (move version)
        inline void PushToBottom(T&& element) {
            std::lock_guard<std::mutex> lock(mMutex);
            mElements.emplace(mElements.begin() + mMiddlePos, std::move(element));
            mMiddlePos++;
        }

        /// @brief removes an element from anywhere in the container
        inline bool Remove(const T& element)
        {
            std::lock_guard<std::mutex> lock(mMutex);

            // search in bottom half first
            auto bottomIt = std::find(mElements.begin(), mElements.begin() + mMiddlePos, element);
            if (bottomIt != mElements.begin() + mMiddlePos) {
                mElements.erase(bottomIt);
                mMiddlePos--;
                return true;
            }

            // search in top half
            auto topIt = std::find(mElements.begin() + mMiddlePos, mElements.end(), element);
            if (topIt != mElements.end()) {
                mElements.erase(topIt);
                return true;
            }

            return false;
        }

        /// @brief checks if element exists anywhere in container
        inline bool Contains(const T& element) const
        {
            std::lock_guard<std::mutex> lock(mMutex);

            // search bottom half
            if (std::find(mElements.begin(), mElements.begin() + mMiddlePos, element) != mElements.begin() + mMiddlePos) return true;

            // search top half
            return std::find(mElements.begin() + mMiddlePos, mElements.end(), element) != mElements.end();
        }

        /// @brief clears the entire container
        inline void Clear()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mElements.clear();
            mMiddlePos = 0;
        }

    public:

        /// @brief finds an element by predicate in both halves
        template<typename Predicate>
        std::optional<T> FindIf(Predicate pred) const
        {
            std::lock_guard<std::mutex> lock(mMutex);

            // Search bottom half first
            auto bottomIt = std::find_if(mElements.begin(), mElements.begin() + mMiddlePos, pred);
            if (bottomIt != mElements.begin() + mMiddlePos) {
                return *bottomIt;
            }

            // Search top half
            auto topIt = std::find_if(mElements.begin() + mMiddlePos, mElements.end(), pred);
            if (topIt != mElements.end()) {
                return *topIt;
            }

            return std::nullopt;
        }

        /// calls a function for each element
        template<typename Function>
        inline void ForEach(Function func)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            for (auto& element : mElements) {
                func(element);
            }
        }

    private:

        mutable std::mutex mMutex;
        std::vector<T> mElements;
        size_t mMiddlePos = 0;
    };
}