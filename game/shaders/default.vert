#version 450 core

layout ( location = 0 ) in vec3 aPosition;
layout ( location = 1 ) in vec3 aNormal;
layout ( location = 2 ) in vec2 aTexCoord;

out vec4 WorldPos;
out vec3 WorldNormal;
out vec2 TexCoord;

uniform _Camera
{
    mat4 view_proj;
};

uniform _Object
{
    mat4 inverse_world;
    mat4 world;
};

mat4 WorldViewProj()
{
    return view_proj * world;
}

void main()
{
    vec4 pos = WorldViewProj() * vec4( aPosition, 1.0f );
    WorldPos = world * vec4( aPosition, 1.0f );

    WorldNormal = mat3( transpose( inverse_world ) ) * aNormal;

    TexCoord = aTexCoord;

    gl_Position = pos;
}