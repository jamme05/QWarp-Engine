/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once
#include "cMatrix4x4.h"

namespace qw
{
	class cTransform
	{
	public:
		constexpr cTransform( const cVector3f& _position = kZero, const cVector3f& _rotation = kZero, const cVector3f& _scale = kOne )
		: m_position( _position )
		, m_rotation( _rotation )
		, m_scale   ( _scale )
		{
			update();
		}

		auto& getPosition( void )       { return m_position; }
		auto& getPosition( void ) const { return m_position; }

		auto& getRotation( void )       { return m_rotation; }
		auto& getRotation( void ) const { return m_rotation; }

		auto& getScale   ( void )       { return m_scale; }
		auto& getScale   ( void ) const { return m_scale; }

		constexpr auto& getLocal( void ) const { return m_local; }

		auto& getLocalFront( void ) const { return reinterpret_cast< const cVector3f& >( m_local.front ); }
		auto& getLocalRight( void ) const { return reinterpret_cast< const cVector3f& >( m_local.right ); }
		auto& getLocalUp   ( void ) const { return reinterpret_cast< const cVector3f& >( m_local.up ); }

		auto& getLocalPosition( void ) const { return reinterpret_cast< const cVector3f& >( m_local.position ); }

		constexpr auto& getWorld( void ) const { return m_world; }

		auto& getWorldFront( void ) const { return reinterpret_cast< const cVector3f& >( m_world.front ); }
		auto& getWorldRight( void ) const { return reinterpret_cast< const cVector3f& >( m_world.right ); }
		auto& getWorldUp   ( void ) const { return reinterpret_cast< const cVector3f& >( m_world.up ); }

		auto& getWorldPosition( void ) const { return reinterpret_cast< const cVector3f& >( m_world.position ); }

		void setParent( const cTransform& _parent ){ m_parent = &_parent; }
		void setParent( const cTransform* _parent ){ m_parent =  _parent; }

		// TODO: Move to cpp file
		cTransform& update( void )
		{
			m_local = Matrix4x4::scale_rotate_translate( m_scale, m_rotation * Math::kDegToRad< float >, m_position );

			if( m_parent )
				m_world = m_parent->getWorld() * m_local;
			else
				m_world = m_local;

			return *this;
		} // update

	private:
		cVector3f m_position;
		cVector3f m_rotation;
		cVector3f m_scale;

		cMatrix4x4f m_local;
		cMatrix4x4f m_world;

		const cTransform* m_parent = nullptr;
	};
} // qw::

