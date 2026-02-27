/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Scene.h"

#include <sk/Seralization/SerializedObject.h>

namespace sk
{
	cScene::cScene( cSerializedObject& _object )
	: cAsset( _object.GetBase< cAsset >().value() )
	{
		_object.BeginRead( this );

		for( auto& obj : _object.ReadValue< cSerializedObject* >( "objects" )->GetArray< cSerializedObject >() )
		{
			auto& object = m_objects.emplace_back( obj.ConstructSharedClass().Cast< Object::cObject >() );
			object->setSceneRecursive( *this );
		}

		_object.EndRead();
	}

	cScene::~cScene()
	{
		m_objects.clear();
	} // ~cScene

	void cScene::AddObject( const cShared_ptr< Object::cObject >& _obj )
	{
		_obj->setSceneRecursive( *this );
		m_objects.emplace_back( _obj );
		_obj->registerRecursive();
		_obj->enableRecursive();
	}

	void cScene::force_render()
	{
		// TODO: Get rid of this

		for( auto& obj : m_objects )
		{
			obj->render();
		}
	} // render

	void cScene::force_update()
	{
		for( auto& obj : m_objects )
			obj->update();
	} // update

	auto cScene::Serialize() -> cSerializedObject
	{
		cSerializedObject object( this );
		object.AddBase( cAsset::Serialize() );

		std::vector< cSerializedObject > objects_vec;
		for( auto& obj : m_objects )
			objects_vec.emplace_back( obj->Serialize() );

		object.WriteValue( "objects", cSerializedObject::ConsumeArray( objects_vec.data(), objects_vec.size() ) );

		object.EndWrite();
		return object;
	}

	void cScene::cleanItems()
	{
		for( auto& target : m_marked_for_removal_ )
		{
			// Skip casting.
			if( target == nullptr )
				continue;

			if( auto object = target.DynCast< Object::cObject >() )
			{
				auto& uuid = object->GetUUID();
				auto pred = [ &uuid ]( auto& _object ){ return _object->GetUUID() == uuid; };
				if( auto itr = std::ranges::find_if( m_objects, pred ); itr != m_objects.end() )
					m_objects.erase( itr );
			}
			else if( auto component = target.DynCast< Object::iComponent >() )
			{
				auto& parent_object = component->m_object_;

				SK_BREAK_RET_IF( sk::Severity::kEngine, parent_object == nullptr, "Warning: This component isn't tied to any object so shouldn't exist." )

				parent_object->RemoveComponent( component.Lock() );
			}
		}

		m_marked_for_removal_.clear();
	}
} // sk::
