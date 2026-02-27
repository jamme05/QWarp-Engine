//
// Created by willness on 2026-02-25.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#pragma once

#include <sk/Misc/UUID.h>
#include <sk/Reflection/RuntimeClass.h>

namespace sk
{
    class cScene;
    class cSceneManager;
} // sk::

// TODO: Redo this. Like. Really redo this. Like. What was I smoking kinda shi????
namespace sk::Object
{
    class cObject;

    SK_CLASS( SceneItem )
    {
        SK_CLASS_BODY( SceneItem )

		friend class sk::cScene;
        friend class sk::Object::cObject;
        friend class sk::Object::iComponent;
        friend class sk::cSceneManager;
    public:
        cSceneItem();

        // Destroy this Object/Component
        void Destroy();
        // Destroy a target Object/Component
        static void Destroy( const cWeak_Ptr< cSceneItem >& _target );

        // Will not be assigned during the construction.
		[[ nodiscard ]] auto& GetUUID () const { return m_uuid_; }
        // Will not be assigned during the construction.
        [[ nodiscard ]] auto& GetScene() const { return *m_scene_; }

    private:
        // As this doesn't have a way to get itself shared. We're gonna have the object and component do it themselves.
        virtual void destroySelf() = 0;
        virtual void setSceneRecursive( cScene& _scene ) = 0;

        cScene* m_scene_;
		cUUID   m_uuid_;
    };
} // sk::Object::

SK_DECLARE_CLASS( sk::Object::SceneItem )