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
        friend class sk::cSceneManager;
    public:
		[[ nodiscard ]] auto& GetUUID() const { return m_uuid_; }

    private:
		cUUID m_uuid_;
    };
} // sk::Object::

SK_DECLARE_CLASS( sk::Object::SceneItem )