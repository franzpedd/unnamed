#include "Entity.h"

#include <cren_error.h>

namespace Cosmos
{
	Entity::Entity(const char* name, ID id)
		: mName(name), mID(id)
	{
	}

	Entity::~Entity()
	{
		if (mComponents.size() > 0) {
			CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Components were not removed before entity destructor");
		}
	}
}