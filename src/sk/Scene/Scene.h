/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once


#include <sk/Assets/Asset.h>
#include <sk/Scene/Object.h>

namespace sk
{
	namespace Object::Components
	{
		class cCamera;
	} // Objects::Components

	SK_ASSET_CLASS( Scene )
	{
		SK_CLASS_BODY( Scene )

		friend class sk::Object::cSceneItem;
		friend class sk::cSceneManager;
	public:
		cScene() = default;
		explicit cScene( cSerializedObject& _object );

		~cScene() override;

		// Creates an empty object, essentially an empty
		template< class Ty, class... Args >
		requires ( std::is_base_of_v< Object::cObject, Ty > && std::constructible_from< Ty, const std::string&, Args... > )
		cShared_ptr< Ty > create_object( const std::string& _name, Args... _args )
		{
			cShared_ptr< Ty > shared = sk::MakeShared< Ty >( _name, _args... );
			shared->m_uuid_  = GenerateRandomUUID();
			shared->m_scene_ = this;
			m_objects.emplace_back( shared );
			return shared;
		} // create_object

		[[ nodiscard ]] auto& GetObjects() const { return m_objects; }

		void force_render();
		void force_update();

		auto Serialize() -> cSerializedObject override;

	private:
		void cleanItems();
		// TODO: Replace this with a map.

		std::vector< cWeak_Ptr< Object::cSceneItem > > m_marked_for_removal_;
		vector< cShared_ptr< Object::cObject > >       m_objects = {};
	};

} // sk::

SK_DECLARE_CLASS( sk::Scene )