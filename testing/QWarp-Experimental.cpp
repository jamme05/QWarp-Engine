// QWarp-Experimental.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cinttypes>

#include <map>
#include <ranges>
#include <unordered_map>

#include <Runtime/cRuntimeClass.h>
#include <Misc/Hashing.h>
#include <Runtime/types.h>

#include <Runtime/cRuntimeStruct.h>

#include "start_test.h"
#include "Misc/print.h"

MAKE_STRUCT( Testing,
     M( uint32_t, Count ),
     M( size_t, Size )
)

MAKE_STRUCT( Testing2,
    M( Testing, Value ),
    M( uint8_t, Value2 )
)

static void print_types( void )
{
    for( const auto& val : qw::type_map | std::views::values )
    {
        switch( val->type )
        {
        case qw::sType_Info::eType::kStandard:
        {
            printf( "Type: %s Name: %s Size: %llu \n", val->raw_name, val->name, TO_LLU( val->size ) );
        }
        break;
        case qw::sType_Info::eType::kStruct:
        {
            printf( "Struct: %s Name: %s Size: %llu \n", val->raw_name, val->name, TO_LLU( val->size ) );
            for( const auto struct_info = val->as_struct_info(); const auto& member : struct_info->members | std::views::values )
            {
                if( const auto member_type = member.get_type() )
                {
                    printf( "   ::%s Type: %s Size: %llu Offset: %llu \n", member.display_name, member_type->raw_name, TO_LLU( member.size ), TO_LLU( member.offset ) );
                }
            }
        }
        break;
        }
    }
}

namespace qw
{
    static_assert( registry::type_registry< 0 >::valid, "No types registered." );
    
    constexpr static auto& types = registry::type_registry< unique_id() - 1 >::registered;
    const unordered_map< type_hash, const sType_Info* > type_map = { types.begin(), types.end() };
} // qw::

constexpr static size_t requirements_display_name( qw::array_ref< char > _to_parse )
{
    return 5;
}
constexpr static void handle_display_name( qw::array_ref< char > _to_parse, char* _out )
{
    
}

enum parser_arg_type : uint8_t
{
    kString,
    kBool,
    kInt,
    kFloat,
};

struct parser_data
{
    uint8_t requires_arg : 1;
    uint8_t type : 7;
    size_t( *requirements_func )( qw::array_ref< char > );
    void( *parser_func )( qw::array_ref< char >, char* );
};

constexpr static qw::const_map parser_map{ qw::array{
    std::pair< qw::str_hash, parser_data >{ "DisplayName", parser_data{ .requires_arg = 1, .type = kString, .requirements_func = &requirements_display_name, .parser_func = &handle_display_name } },
} };

template< size_t Index, class Ty, const Ty& Strings >
static consteval auto get_requirements( void )
{
    constexpr auto val = std::get< Index >( Strings );
    typedef qw::str::find< val, "=" > search;
    // Make sure it hashes the correct part
    constexpr qw::str_hash hash = qw::str_hash( val.get(), search::kIndex );

    if constexpr( constexpr auto p_index = parser_map.find( hash ); p_index != -1 )
    {
        const auto& parser = parser_map.get( p_index ).second;
        auto req = parser.requirements_func( val );
        return qw::array{ std::pair< size_t, size_t >{ Index, req } };
    }
    return qw::array{ std::pair< size_t, size_t >{ Index, 0 } };
}

template< size_t Index, size_t Count, class Ty, const Ty& AsTuple >
struct iterate;

template< bool Valid, class Instance >
struct fetcher
{
    static constexpr auto requirements = qw::array< std::pair< size_t, size_t >, 0 >{};
};
template< class Instance >
struct fetcher< true, Instance >
{
    typedef typename Instance::tuple_t tuple_t;
    typedef iterate< Instance::kIndex + 1, Instance::kCount, tuple_t, Instance::kTuple > next_t;
    static constexpr auto requirements = get_requirements< Instance::kIndex, tuple_t, Instance::kTuple >() + next_t::requirements; //  +
};

template< size_t Index, size_t Count, class Ty, const Ty& AsTuple >
struct iterate_package
{
    static constexpr auto kIndex = Index;
    static constexpr auto kCount = Count;
    typedef Ty tuple_t;
    static constexpr auto& kTuple = AsTuple;
};

template< size_t Index, size_t Count, class Ty, const Ty& AsTuple >
struct iterate
{
    typedef iterate_package< Index, Count, Ty, AsTuple > package_t;
    typedef fetcher< Index < Count, package_t > fetcher_t;
    static constexpr auto requirements = fetcher_t::requirements;
};

template< qw::array... Arrays >
struct test_str
{
    constexpr static auto size         = sizeof...( Arrays );
    constexpr static auto as_tuple     = std::make_tuple( Arrays... );
    typedef decltype( as_tuple ) tuple_t;
    typedef iterate< 0, size, tuple_t, as_tuple > iterate_t;
    constexpr static auto requirements = iterate_t::requirements;
};

struct filter_arr
{
    
};

int main()
{
    print_types();
    
    typedef test_str< "DisplayName=\"Test\"", "Hidden=true", "ReadOnly" >::iterate_t::package_t save_t;
    constexpr static auto t1 = get_requirements< save_t::kIndex, save_t::tuple_t, save_t::kTuple >();
    constexpr static auto t = std::get< 0 >( save_t::kTuple );
    typedef qw::str::find< t, "=" > search;
    constexpr qw::str_hash h1{ t.get(), search::kIndex };
    constexpr auto r1 = parser_map.find( h1 );
    constexpr qw::str_hash h2{ "DisplayName" };
    constexpr qw::str_hash h3{ "DisplayName=\"Test\"", search::kIndex };
    constexpr bool b1 = h1 == h2;
    
    test_str< "DisplayName=\"Test\"", "Hidden=true", "ReadOnly" >::iterate_t::fetcher_t::next_t::fetcher_t::next_t::fetcher_t::requirements;

    return 0;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
