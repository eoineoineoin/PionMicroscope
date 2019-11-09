#pragma once
#include <stdint.h>
#include <assert.h>

namespace Packets
{
enum class Type : uint8_t
{
	NONE,
	BEAM_STATE,
	RESOLUTION_CHANGED,

	SET_RESOLUTION,
	SET_TARGET_MODE,
};

struct BasePacket
{
	Type m_type { Type::NONE };

	inline struct BeamState const * asBeamState() const;
	inline struct ResolutionChanged const * asResolutionChanged() const;
	inline struct SetResolution const * asSetResolution() const;
	inline struct SetTargetMode const * asSetTargetMode() const;
};

// Utility to set BasePacket::m_type more easily
template<Type TYPE>
struct TypedPacket : BasePacket
{
	TypedPacket() { m_type = TYPE; }
};


struct BeamState : public TypedPacket<Type::BEAM_STATE>
{
	uint16_t m_input0;
	uint16_t m_x;
	uint16_t m_y;
};
static_assert(sizeof(BeamState) == sizeof(uint64_t), "Expect 8 byte commands");

struct ResolutionChanged : public TypedPacket<Type::RESOLUTION_CHANGED>
{
	uint8_t m_padding[3];
	uint16_t m_resolutionX;
	uint16_t m_resolutionY;
};
static_assert(sizeof(ResolutionChanged) == sizeof(uint64_t), "Expect 8 byte commands");

struct SetResolution : public TypedPacket<Type::SET_RESOLUTION>
{
	uint8_t m_padding[3];
	uint16_t m_resolutionX;
	uint16_t m_resolutionY;
};
static_assert(sizeof(SetResolution) == sizeof(uint64_t), "Expect 8 byte commands");

struct SetTargetMode : public TypedPacket<Type::SET_TARGET_MODE>
{
	enum class Mode : uint8_t
	{
		LOCK_X,
		UNLOCK_X,
	} m_mode;

	uint16_t m_targetData; // Fixed coordinate in LOCK modes
	uint32_t m_padding;
};
static_assert(sizeof(SetTargetMode) == sizeof(uint64_t), "Expect 8 byte commands");



struct CurrentState
{
	uint16_t m_x;
	uint16_t m_y;
	uint32_t m_input0;
};

struct ControlCommand
{
	enum class Type
	{
		NONE,
		LOCK_X,
		UNLOCK_X,
		SET_RESOLUTION,
	};

	Type m_type;
	uint32_t m_extraData;
};

}

inline Packets::BeamState const * Packets::BasePacket::asBeamState() const
{
	assert(m_type == Type::BEAM_STATE);
	return static_cast<Packets::BeamState const *>(this);
}

inline Packets::ResolutionChanged const * Packets::BasePacket::asResolutionChanged() const
{
	assert(m_type == Type::RESOLUTION_CHANGED);
	return static_cast<Packets::ResolutionChanged const *>(this);
}

inline Packets::SetTargetMode const * Packets::BasePacket::asSetTargetMode() const
{
	assert(m_type == Type::SET_TARGET_MODE);
	return static_cast<Packets::SetTargetMode const *>(this);
}

inline Packets::SetResolution const * Packets::BasePacket::asSetResolution() const
{
	assert(m_type == Type::SET_RESOLUTION);
	return static_cast<Packets::SetResolution const *>(this);
}

