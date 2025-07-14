#pragma once

#include <scenegraph/utils/MurmurHash.h>

#include <string_view>
#include <cstdint>

using HashType = uint32_t;

enum class HashNamespace {
	Default,
	Type,
	Message
};

enum class ComponentType : HashType;
enum class ComponentMessage : HashType;

constexpr ComponentType MakeComponentType(std::string_view name) noexcept
	{ return static_cast<ComponentType>(Murmur3Hash32(name, static_cast<uint32_t>(HashNamespace::Type))); }

constexpr ComponentMessage MakeComponentMessage(std::string_view name) noexcept
	{ return static_cast<ComponentMessage>(Murmur3Hash32(name, static_cast<uint32_t>(HashNamespace::Message))); }

#define DEFINE_COMPONENT_MESSAGE(Msg) \
	constexpr auto Msg = MakeComponentMessage(#Msg);

namespace ComponentMessages {
	DEFINE_COMPONENT_MESSAGE(Added)      // Added to SceneObject
	DEFINE_COMPONENT_MESSAGE(Removed)    // Removed from SceneObject
	DEFINE_COMPONENT_MESSAGE(Apply)      // Apply to SceneObject
}

struct ComponentMessageParams {
	class SceneObject* sceneObject;
};
