/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Memory/cInternal.h"

namespace qw
{
	template< class Ty >
	class cSingleton
	{
	private:
		static Ty* m_instance;

	protected:
		         cSingleton( void ){ m_instance = static_cast< Ty* >( this ); }
		virtual ~cSingleton( void ){ m_instance = nullptr; }

	public:

		template< typename... Args >
		static Ty&   initialize  ( Args&&... _args ){ m_instance = Memory::Internal::alloc< Ty >( std::forward< Args >( _args )... ); return *m_instance; }
		static void  deinitialize( void )         { Memory::Internal::free( m_instance ); }
		static Ty&   get         ( void )         { return *m_instance; } // TODO: Add error/assert checks
		static auto  getPtr      ( void )         { return  m_instance; }

	};

	template< class Ty > Ty* cSingleton< Ty >::m_instance = nullptr;

} // qw::
