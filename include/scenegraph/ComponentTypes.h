#pragma once

#include <scenegraph/utils/MurmurHash.h>

#include <string_view>
#include <cstdint>

enum class ComponentType : uint32_t {};

constexpr ComponentType ComponentTypeFromName(std::string_view name) noexcept
	{ return static_cast<ComponentType>(Murmur3Hash32(name)); }

enum class ComponentMessage {
	Added,        // Added to SceneObject
	Removed,      // Removed from SceneObject
	Apply         // Applying to SceneObject
};

class SceneObject;

struct ComponentMessageParams {
	SceneObject* sceneObject;
};
