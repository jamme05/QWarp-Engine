/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Type_Manager.h"

#include <sk/Reflection/RuntimeClass.h>

#include <ranges>

namespace sk::Reflection
{
    cType_Manager::cType_Manager()
    {
        auto& storage = *register_type( nullptr, true );
        // Empty the storage and feed it into the actual instance.
        for( const auto& info : storage )
            register_type( info );

        storage = {};
    } // cType_Manager

    bool cType_Manager::RegisterType( const type_info_t _type_info )
    {
        if( const auto inst = getPtr() )
            return inst->register_type( _type_info );

        register_type( _type_info, false );

        return false;
    } // RegisterType

    auto cType_Manager::GetDerivedTypes( const iRuntimeClass& _base_class ) -> std::vector< const iRuntimeClass* >
    {
        std::vector< const iRuntimeClass* > derived;

        for( const auto& type : m_types_ | std::views::values )
        {
            if( type->type != sType_Info::eType::kClass )
                continue;

            const auto runtime_class = type->as_class_info()->runtime_class;
            if( runtime_class->isDerivedFrom( _base_class ) )
                derived.emplace_back( type->as_class_info()->runtime_class );
        }

        return derived;
    }

    cType_Manager::type_storage_t* cType_Manager::register_type( type_info_t _type_info, const bool _extract )
    {
        static type_storage_t inst = {};

        // We don't expect any addition when we're extracting.
        if( _extract )
            return &inst;

        inst.emplace_back( _type_info );
        return nullptr;
    } // register_type

    bool cType_Manager::register_type( type_info_t _type_info )
    {
        auto [ _, success ] = m_types_.emplace( _type_info->hash, _type_info );
        return success;
    } // register_type

} // sk::Reflection::

inline auto CONCAT( type_registry_, __COUNTER__ ) = sk::Reflection::cType_Manager::RegisterType< void >();