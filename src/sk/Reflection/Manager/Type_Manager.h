/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Misc/Singleton.h>

#define SK_ALLOW_DIRECT_REGISTRY_ACCESS
#include <sk/Reflection/Type_Registry.h>
#undef SK_ALLOW_DIRECT_REGISTRY_ACCESS

#include <unordered_map>

namespace sk::Reflection
{
    class cType_Manager : public cSingleton< cType_Manager >
    {
        using type_map_t     = std::unordered_map< type_hash, type_info_t >;
        using type_storage_t = std::vector< type_info_t >;
    public:
        cType_Manager();

        static bool RegisterType( type_info_t _type_info );
        template< reflected Ty >
        static bool RegisterType()
        {
            return RegisterType( &kTypeInfo< Ty > );
        } // RegisterType

        [[ nodiscard ]]
        auto& GetTypes() const { return m_types_; }

        auto GetDerivedTypes( const iRuntimeClass& _base_class ) -> std::vector< const iRuntimeClass* >;

    private:
        static type_storage_t* register_type( type_info_t _type_info, bool _extract );
        bool register_type( type_info_t _type_info );

        type_map_t m_types_;
    };
} // sk::Reflection::
