/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Shader.h"

#include <sk/Graphics/Renderer_Impl.h>
#include <sk/Graphics/Buffer/Dynamic_Buffer.h>
#include <sk/Graphics/Utils/Shader_Reflection.h>

#include <glbinding/gl/functions.h>

namespace sk::Assets
{
    // Shader
    cShader::cShader( const eType _type, const void* _buffer, const size_t _size )
    {
        m_type_ = gl::GL_INVALID_VALUE;
        switch( _type )
        {
        case eType::kVertex:   m_type_ = gl::GL_VERTEX_SHADER;   break;
        case eType::kFragment: m_type_ = gl::GL_FRAGMENT_SHADER; break;
        case eType::kGeometry: m_type_ = gl::GL_GEOMETRY_SHADER; break;
        case eType::kCompute:  m_type_ = gl::GL_COMPUTE_SHADER;  break;
        }

        SK_ERR_IF( m_type_ == gl::GL_INVALID_VALUE,
            "ERROR: Invalid shader type." )

        SK_ERR_IF( m_type_ == gl::GL_GEOMETRY_SHADER,
            "ERROR: Geometry shaders aren't supported yet." )

        SK_ERR_IF( m_type_ == gl::GL_COMPUTE_SHADER,
            "ERROR: Compute shaders aren't supported yet." )

        
        Graphics::cGLRenderer::AddGLTask( [ & ]
        {
            m_shader_ = gl::glCreateShader( m_type_ );
            const auto str = static_cast< const gl::GLchar* >( _buffer );
            const auto size = static_cast< const gl::GLsizei >( _size );
            gl::glShaderSource( m_shader_, 1, &str, &size );
            gl::glCompileShader( m_shader_ );

            char buffer[ 512 ];
            gl::GLsizei length;
            gl::glGetShaderInfoLog( m_shader_, 512, &length, buffer );
            SK_BREAK_IF( sk::Severity::kGraphics, length != 0,
                TEXT( "Shader Info: {}", std::string_view{ buffer, static_cast< size_t >( length ) } ) )
        } );
    } // cShader

    cShader::~cShader()
    {
        Graphics::cGLRenderer::AddGLTask( [ & ]
        {
            gl::glDeleteShader( m_shader_ );
        } );
    } // ~cShader
} // sk::Assets