/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Misc/Hashing.h"

#include "Macros/manipulation.h"

#include "types.h"

namespace qw
{
	class iRuntimeClass;
	template<>
	class hash< iRuntimeClass > : public Hashing::iHashed
	{
	public:
	constexpr hash( const char* _str, const uint64_t _val )
	: iHashed( Hashing::fnv1a_64( _str, _val ) )
	{}

	HASH_REQUIREMENTS( hash )
	};

	class iClass;

	class iRuntimeClass
	{
	public:
		constexpr iRuntimeClass( const char* _name, const char* _file, const uint32_t _line = 0, const uint64_t& _parent_hash = Hashing::val_64_const )
		: m_hash( _name, _parent_hash )
		, m_raw_name( _name )
		, m_file_path( _file )
		, m_line( _line )
		{
		} // iClass

		constexpr auto& getType    ( void ) const { return m_hash; }
		constexpr auto  getRawName ( void ) const { return m_raw_name; }
		constexpr auto  getFileName( void ) const { return m_file_path; }
		auto            getName    ( void ) const { return std::string( m_raw_name ); }
		auto            getLine    ( void ) const { return m_line; }

		virtual constexpr bool isDerivedFrom( const iRuntimeClass& _base    ) const { return false; } // Has to be set so iClass isn't a pure virtual
		virtual constexpr bool isBaseOf     ( const iRuntimeClass& _derived ) const { return false; } // Has to be set so iClass isn't a pure virtual

		constexpr bool operator==( const iRuntimeClass& _right ) const { return m_hash == _right.m_hash; }

	private:
		hash< iRuntimeClass > m_hash;
		const char*    m_raw_name;
		const char*    m_file_path;
		size_t         m_line;
	};

	constexpr static iRuntimeClass kInvalidClass{ "Invalid", __FILE__ };

	template< bool Select, class Ty, class Ty2 >
	struct select_class_type{};

	template< class Ty, class Ty2 >
	struct select_class_type< true, Ty, Ty2 >
	{
		typedef std::remove_cvref_t< decltype( Ty::getStaticClass() ) > type;
	};
	template< class Ty, class Ty2 >
	struct select_class_type< false, Ty, Ty2 >
	{
		typedef Ty2 type;
	};


	template< bool Select, class Ty, class Ty2 > // TODO: Move to a more global space, might be liked. Also maybe make an int variant?
	struct select_type{};

	template< class Ty, class Ty2 >
	struct select_type< true, Ty, Ty2 >
	{
		typedef Ty type;
	};
	template< class Ty, class Ty2 >
	struct select_type< false, Ty, Ty2 >
	{
		typedef Ty2 type;
	};

	template< bool Select, class Ty >
	struct select_class_ref{};

	template< class Ty >
	struct select_class_ref< true, Ty >
	{
		static constexpr auto& class_ref = Ty::getStaticClass();
	};
	template< class Ty >
	struct select_class_ref< false, Ty >
	{
		static constexpr auto& class_ref = kInvalidClass;
	};

	template< class Ty >
	struct get_parent_class
	{
	private:
		template< class Ty2 >
		static uint8_t  test( decltype( &Ty2::getStaticClass ) );
		template< class >
		static uint16_t test( ... );
	public:
		static constexpr bool has_value = sizeof( decltype( test< Ty >( 0 ) ) ) == 1;
	private:
		typedef typename select_class_type< has_value, Ty, iRuntimeClass >::type pre_type;
	public:
		static constexpr bool is_valid  = std::is_base_of_v< iRuntimeClass, pre_type >;
		static constexpr bool is_base   = std::is_same_v< pre_type, iRuntimeClass >;
		static constexpr bool uses_own  = is_valid && !is_base;
		typedef typename select_class_type< uses_own, Ty, iRuntimeClass >::type   class_type;
		typedef typename select_type< uses_own, Ty, iClass >::type inherits_type;
	};

	template< class Ty >
	constexpr auto& get_class_ref = select_class_ref< get_parent_class< Ty >::uses_own, Ty >::class_ref;

	template< class Ty >
	using get_parent_class_t = typename get_parent_class< Ty >::class_type;
	template< class Ty = iClass >
	using get_inherits_t     = typename get_parent_class< Ty >::inherits_type;

	template< class Ty, class Pa = iRuntimeClass, const get_parent_class_t< Pa >& Parent = get_class_ref< Pa >, bool ForceShared = true >
	requires std::is_base_of_v< iRuntimeClass, get_parent_class_t< Pa > >
	class cRuntimeClass : public get_parent_class_t< Pa >
	{
	public:
		typedef Ty                       value_type;
		typedef get_parent_class_t< Pa > parent_type;

		// TODO: Move
		typedef typename get_parent_class< Pa >::inherits_type inherits_type;

		constexpr cRuntimeClass( const char* _name, const char* _file = nullptr, const uint32_t _line = 0, const uint64_t& _parent_hash = Parent.getType().getHash() )
		: parent_type( _name, _file, _line, _parent_hash )
		{} // cClass

		// Use std::is_base_of / std::is_base_of_v instead of this in case both types are known.
		constexpr bool isDerivedFrom( const iRuntimeClass& _base ) const override
		{
			if( *this == _base )
				return true;

			// The placeholder parent will always be of the type iClass
			if constexpr( std::is_same_v< iRuntimeClass, parent_type > )
				return false;
			else
				return Parent.isDerivedFrom( _base );
		} // isDerivedFrom

		constexpr bool isBaseOf( const iRuntimeClass& _derived ) const override
		{
			return _derived.isDerivedFrom( *this );
		} // isBaseOf

		template< class... Args >
		requires ( !ForceShared )
		Ty* create( Args&&... ){ return nullptr; } // TODO: Create function
	};

	class iClass // Switch names between iClass and iRuntrimeClass?
	{
	public:
		iClass( void ) = default;
		virtual ~iClass( void ) = default;
		virtual constexpr const iRuntimeClass&         getClass    ( void ) const = 0;
		virtual constexpr const hash< iRuntimeClass >& getClassType( void ) = 0;
		virtual constexpr const std::string     getClassName( void ) = 0;
	};

	template< class Ty >
	requires std::is_base_of_v< iClass, Ty >
	struct get_type_id< Ty >
	{
		constexpr static auto&     kClass  = Ty::getStaticClass();
		constexpr static type_hash kId     = kClass.getType().getHash();
		constexpr static char      kName[] = kClass.getRawName();
	};

} // qw::

// TODO: Rename getStaticClassType to getStaticType

// Required to make a runtime class functional.
#define CREATE_CLASS_IDENTIFIERS( RuntimeClass ) public: \
	typedef decltype( RuntimeClass ) class_type;           \
	constexpr const qw::iClass&             getClass    ( void ) const override { return m_class;     } \
	constexpr const qw::hash< qw::iClass >& getClassType( void ) override { return m_class.getType(); } \
	const std::string                       getClassName( void ) override { return m_class.getName(); } \
	static constexpr auto&  getStaticClass    ( void ){ return RuntimeClass;           } \
	static constexpr auto&  getStaticClassType( void ){ return RuntimeClass .getType(); } \
	static auto             getStaticClassName( void ){ return RuntimeClass .getName(); } \
	protected:                              \
	constexpr static auto& m_class = RuntimeClass; \
	private:

#define CREATE_CLASS_BODY( Class ) CREATE_CLASS_IDENTIFIERS( runtime_class_ ## Class )

#define CREATE_RUNTIME_CLASS_VALUE( Class, Name, ... ) static constexpr auto CONCAT( runtime_class_, Name ) = qw::cRuntimeClass< Class __VA_OPT__(,) FIRST( __VA_ARGS__ ) >( #Name, __FILE__, __LINE__ );

// Requires you to manually add CREATE_CLASS_IDENTIFIERS inside the body. But gives greater freedom. First inheritance will always have to be public. Unable to function with templated classes.
#define GENERATE_CLASS( Class, ... ) \
class Class ; \
CREATE_RUNTIME_CLASS_VALUE( Class, Class, __VA_ARGS__ ) \
class Class : public qw::get_inherits_t< FIRST( __VA_ARGS__ ) > \

// Generates both runtime info and start of body body, but removes most of your freedom. Unable to function with templated classes.
#define GENERATE_ALL_CLASS( Class, ... ) GENERATE_CLASS( Class __VA_OPT__(,) __VA_ARGS__ ) AFTER_FIRST( __VA_ARGS__ ) { CREATE_CLASS_BODY( Class )