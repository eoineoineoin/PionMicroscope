#pragma once
#include <stdint.h>
#include <assert.h>
#include <algorithm>

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
	/// This corresponds to a float value, rescaled and packed to fit
	/// 16 bits. A value of INT16_MAX indicates a reading of 5V.
	/// Use packVoltage() and unpackVoltage() to convert between float.
	int16_t m_input0;
	uint16_t m_x;
	uint16_t m_y;

	static float maxVoltage() { return 5.0f; }

	// Helper to pack a voltage into m_input0
	void packVoltage(float v)
	{
		const float clamped = std::max(-maxVoltage(), std::min(maxVoltage(), v));
		const float frac = clamped / maxVoltage();
		m_input0 = frac * INT16_MAX;
	}

	// Unpack the stored voltage, returning float in the range +/- maxVoltage()
	float unpackVoltage() const
	{
		return (maxVoltage() * m_input0) / float(INT16_MAX);
	}
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
	enum class AxisMode : uint8_t
	{
		LOCK,
		UNLOCK,
	};
	
	AxisMode m_modeX;
	AxisMode m_modeY;

	uint8_t m_padding;

	uint16_t m_fracX;
	uint16_t m_fracY;
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

