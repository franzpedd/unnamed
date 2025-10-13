#pragma once

#include "Core/Defines.h"
#include "Components.h"
#include "Util/ID.h"
#include <any>
#include <unordered_map>
#include <typeindex>

namespace Cosmos
{
    class COSMOS_API Entity
    {
    public:

        /// @brief constructor
        Entity(const char* name = "Empty Entity", ID id = 0);

        /// @brief destructor
        ~Entity();

        /// @brief returns the entity name
        inline const char* GetName() { return mName; }

        /// @brief returns the entity id
        inline uint32_t GetIDValue() { return mID.GetValue(); }

    public:

        /// @brief returns if the entity has a given component
        template<typename T>
        bool HasComponent()
        {
            return mComponents.find(typeid(T)) != mComponents.end();
        }

        /// @brief returns desired the component's memory address, nullptr otherwise
        template<typename T>
        T* GetComponent()
        {
            auto it = mComponents.find(typeid(T));
            if (it != mComponents.end()) {
                try {
                    return std::any_cast<T>(&it->second);
                }
                catch (const std::bad_any_cast&) {
                    return nullptr;
                }
            }
            return nullptr;
        }

        /// @brief adds a unique type of component to the entity
        template<typename T, typename... Args>
        void AddComponent() 
        {
            if (HasComponent<T>()) return;

            // construct in-place with perfect forwarding
            auto [it, inserted] = mComponents.try_emplace(typeid(T), std::in_place_type<T>, std::forward<Args>(args)...);
        }

        /// @brief erases the component from the entity
        template<typename T>
        void RemoveComponent()
        {
            auto it = mComponents.find(typeid(T));
            if (it != mComponents.end()) {
                mComponents.erase(it);
                return true;
            }
            return false;
        }

    private:

        const char* mName = nullptr;
        ID mID = {};
        std::unordered_map<std::type_index, std::any> mComponents;
    };
}