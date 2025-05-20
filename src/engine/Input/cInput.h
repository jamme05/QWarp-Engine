/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Types/system_types.h"

#include <queue>

#include "Math/cVector2.h"
#include "Math/Math.h"

namespace qw::Platform
{
	class cPad;
} // qw::Platform::

namespace qw::Input
{
	class iListener;

	enum eType : uint32_t
	{
		kKey_Down    = 0x00000001,
		kKey_Up      = 0x00000002,
		kKey         = kKey_Down | kKey_Up,
		kMouse       = 0x00000004,
		kMouse_Down  = 0x00000008,
		kMouse_Up    = 0x00000010,
		kButton_Down = 0x00000020,
		kButton_Up   = 0x00000040,
		kButton      = kButton_Down | kButton_Up,
		kStick       = 0x00000080,
		kAnalog      = 0x00000100,
		kAll         = 0xffffffff,

	}; // eType

	enum class eAnalog : uint8_t
	{
		kMouse,
		kLeft,
		kRight,

	}; // eAnalog

	enum class eButton : uint32_t
	{
		kNone        = 0x00,
		PLATFORM_BUTTONS // TODO: Make something more practical.
	}; // eButton

	struct sPadEvent
	{
		eButton         button          = eButton::kNone;
		eAnalog         analog          = eAnalog::kMouse;
		cVector2f       current_stick   = { };
		cVector2f       previous_stick  = { };
		float           current_analog  = { };
		float           previous_analog = { };
		Platform::cPad* pad             = nullptr;

	};

	// Only use data from event TEMPORARILY, it'll be removed after the event.
	// Exceptions for pads, but you'd rather want to get a user for that.
	struct sEvent
	{
		const sPadEvent* pad;

	};

	// Disabled in debug
	extern void setLogInputs  ( const bool _log_inputs );
	extern void addListener   (       iListener* _listener );
	extern void removeListener( const iListener* _listener );
	extern void event         ( const eType _type, const sPadEvent& _event );
	extern void event         ( const eType _type, const sEvent&    _event );

	class iListener
	{
		uint32_t m_filter;
		uint16_t m_priority;
		bool     m_enabled  = false;

	public:

		         iListener( const uint32_t _filter, const uint16_t _priority, const bool _enabled = false );
		virtual ~iListener( void );

		// No setting the priority after creation... For now
		auto getPriority( void ) const                { return m_priority;    }
		bool getEnabled ( void ) const                { return m_enabled;     }
		auto getFilter  ( void ) const -> uint32_t    { return m_filter;      }
		void setEnabled ( const bool _enabled )       { m_enabled = _enabled; }
		void setFilter  ( const uint32_t _filter )    { m_filter = _filter;   }

		virtual bool onInput( const eType _type, const sEvent& _event ) = 0;

	}; // iListener

} // qw::Input::
