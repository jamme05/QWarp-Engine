

#include "Shader_Reflection.h"

#include <sk/Assets/Shader.h>
#include <sk/Math/Matrix.h>
#include <sk/Math/Matrix3x3.h>
#include <sk/Math/Matrix4x4.h>
#include <sk/Math/Vector2.h>
#include <sk/Math/Vector3.h>
#include <sk/Math/Vector4.h>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

using namespace sk::Graphics::Utils;

// Shader reflection:
cShader_Reflection::cShader_Reflection( const gl::GLuint _program )
: m_program_( _program )
{
    init();
    
    fetch_attributes();

    fetch_blocks();
    fetch_uniforms();

    fetch_buffer_bindings();
    
    cleanup();
}

void cShader_Reflection::init()
{
    gl::GLint out;
    gl::glGetProgramiv( m_program_, gl::GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &out );
    m_max_attribute_name_size_ = std::max( static_cast< uint32_t >( out ), 128u );

    gl::glGetProgramiv( m_program_, gl::GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &out );
    m_max_block_name_size_ = std::max( static_cast< uint32_t >( out ), 128u );

    gl::glGetProgramiv( m_program_, gl::GL_ACTIVE_UNIFORM_MAX_LENGTH, &out );
    m_max_uniform_name_size_ = std::max( static_cast< uint32_t >( out ), 128u );
    m_uniform_name_buffer_   = static_cast< gl::GLchar* >( SK_ALLOC( m_max_uniform_name_size_ ) );
}

void cShader_Reflection::fetch_attributes()
{
    gl::GLint nr_of_attributes;
    gl::glGetProgramiv( m_program_, gl::GL_ACTIVE_ATTRIBUTES, &nr_of_attributes );

    const auto a_name_buffer = static_cast< gl::GLchar* >( SK_ALLOC( m_max_attribute_name_size_ ) );
    
    for( int_fast32_t i = 0; i < nr_of_attributes; i++ )
    {
        auto& attribute = m_attributes_.emplace_back();
        
        gl::GLsizei name_length;
        gl::GLsizei size;
        gl::GLenum  type;
        
        gl::glGetActiveAttrib( m_program_, i, static_cast< gl::GLint >( m_max_attribute_name_size_ ),
            &name_length, &size, &type, a_name_buffer );
        
        attribute.name   = std::string_view( a_name_buffer, name_length );
        attribute.index  = gl::glGetAttribLocation( m_program_, a_name_buffer );
        // Size may be array size here.
        attribute.elements = static_cast< uint8_t >( size );
        attribute.flags  = a_name_buffer[ 0 ] == '_' ? kHidden : 0;

        type_info_t sk_type;
        gl::GLenum  gl_type;
        uint16_t    components;
        switch( type )
        {
        case gl::GL_FLOAT:      sk_type = kTypeInfo< float >;     gl_type = gl::GL_FLOAT; components = 1; break;
        case gl::GL_FLOAT_VEC2: sk_type = kTypeInfo< cVector2f >; gl_type = gl::GL_FLOAT; components = 2; break;
        case gl::GL_FLOAT_VEC3: sk_type = kTypeInfo< cVector3f >; gl_type = gl::GL_FLOAT; components = 3; break;
        case gl::GL_FLOAT_VEC4: sk_type = kTypeInfo< cVector4f >; gl_type = gl::GL_FLOAT; components = 4; break;
            
        case gl::GL_INT:       sk_type = kTypeInfo< int32_t >;     gl_type = gl::GL_INT; components = 1; break;
        case gl::GL_INT_VEC2:  sk_type = kTypeInfo< cVector2i32 >; gl_type = gl::GL_INT; components = 2; break;
        case gl::GL_INT_VEC3:  sk_type = kTypeInfo< cVector3i32 >; gl_type = gl::GL_INT; components = 3; break;
        case gl::GL_INT_VEC4:  sk_type = kTypeInfo< cVector4i32 >; gl_type = gl::GL_INT; components = 4; break;
            
        case gl::GL_UNSIGNED_INT:      sk_type = kTypeInfo< uint32_t >;    gl_type = gl::GL_UNSIGNED_INT; components = 1; break;
        case gl::GL_UNSIGNED_INT_VEC2: sk_type = kTypeInfo< cVector2u32 >; gl_type = gl::GL_UNSIGNED_INT; components = 2; break;
        case gl::GL_UNSIGNED_INT_VEC3: sk_type = kTypeInfo< cVector3u32 >; gl_type = gl::GL_UNSIGNED_INT; components = 3; break;
        case gl::GL_UNSIGNED_INT_VEC4: sk_type = kTypeInfo< cVector4u32 >; gl_type = gl::GL_UNSIGNED_INT; components = 4; break;
            
        case gl::GL_FLOAT_MAT2: sk_type = kTypeInfo< Math::cMatrix< 2, 2, float > >; gl_type = gl::GL_FLOAT; components = 1; break;
        case gl::GL_FLOAT_MAT3: sk_type = kTypeInfo< Math::cMatrix3x3f >;            gl_type = gl::GL_FLOAT; components = 2; break;
        case gl::GL_FLOAT_MAT4: sk_type = kTypeInfo< cMatrix4x4f >;                  gl_type = gl::GL_FLOAT; components = 3; break;
            
        default: sk_type = nullptr; gl_type = gl::GL_INVALID_ENUM; components = 0; break;
        }
        
        SK_ERR_IF( sk_type == nullptr,
            "Error: Invalid attribute type." )
        
        attribute.type       = sk_type;
        attribute.gl_type    = gl_type;
        attribute.components = static_cast< uint8_t >( components );
        attribute.stride     = static_cast< uint32_t >( sk_type->size );
    }
    
    SK_FREE( a_name_buffer );
}

void cShader_Reflection::fetch_uniforms()
{
    gl::GLint nr_of_uniforms;
    gl::glGetProgramiv( m_program_, gl::GL_ACTIVE_UNIFORMS, &nr_of_uniforms );

    for( int_fast32_t i = 0; i < nr_of_uniforms; i++ )
    {
        gl::GLsizei name_length;
        gl::glGetActiveUniformName( m_program_, i, static_cast< gl::GLsizei >( m_max_block_name_size_ ),
            &name_length, m_uniform_name_buffer_ );
        
        if( m_locked_uniforms_.contains( std::string_view( m_uniform_name_buffer_, name_length ) ) )
            continue;

        auto uniform = get_uniform( i );
        
        std::visit( [ this ]< class Ty >( Ty& _value )
        {
            if constexpr( !std::is_same_v< Ty, bool > )
            {
                add( _value );
            }
            else
            {
                SK_FATAL( "This shouldn't happen..." )
            }
        }, uniform );
        
    }
}

void cShader_Reflection::fetch_blocks()
{
    gl::GLint nr_of_blocks;
    gl::glGetProgramiv( m_program_, gl::GL_ACTIVE_UNIFORM_BLOCKS, &nr_of_blocks );
    
    if( nr_of_blocks == 0 )
        return;
    
    m_block_map_.reserve( nr_of_blocks );
    m_block_vec_.resize( nr_of_blocks );

    const auto b_name_buffer = static_cast< char* >( SK_ALLOC( m_max_block_name_size_ ) );
    
    for( int_fast32_t i = 0; i < nr_of_blocks; i++ )
    {
        gl::GLsizei name_length;
        gl::glGetActiveUniformBlockName( m_program_, i, static_cast< gl::GLsizei >( m_max_block_name_size_ ),
            &name_length, b_name_buffer );
        
        auto  name    = std::string_view( b_name_buffer, name_length );
        auto& block   = m_block_map_[ name ];
        block.name    = name;
        block.pretty_name = make_pretty( name );
        block.binding = static_cast< uint16_t >( i );
        block.flags   = b_name_buffer[ 0 ] == '_' ? kHidden : 0;
        
        gl::GLint nr_of_uniforms;
        gl::glGetActiveUniformBlockiv( m_program_, i, gl::GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &nr_of_uniforms );

        auto indices = std::vector< gl::GLint >{};
        indices.resize( static_cast< size_t >( nr_of_uniforms ) ); 
        
        gl::glGetActiveUniformBlockiv( m_program_, i, gl::GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data() );
        
        block.uniforms.reserve( static_cast< size_t >( nr_of_uniforms ) );

        std::vector< gl::GLint > offsets( nr_of_uniforms );

        gl::glGetActiveUniformsiv( m_program_, nr_of_uniforms, reinterpret_cast< const gl::GLuint* >( indices.data() ), gl::GL_UNIFORM_OFFSET, offsets.data() );

        // TODO: Handle multi dimensional arrays which are received as array[ N ][ 0 ]
        uint32_t block_size = 0;
        for( int_fast32_t u = 0; u < nr_of_uniforms; u++ )
        {
            auto uniform = std::get< sUniform >( get_uniform( indices[ u ] ) );
            
            uniform.location = offsets[ u ];
            block_size      += sk::Memory::get_aligned( uniform.byte_size * uniform.size, Memory::eAlignment::kShaderAlign );
            
            m_locked_uniforms_.insert( uniform.name );
            block.uniforms[ uniform.name.hash() ] = std::move( uniform );
        }
        
        block.size = block_size;
        
        m_block_vec_[ i ] = &block;
    }
    
    SK_FREE( b_name_buffer );
}

void cShader_Reflection::fetch_buffer_bindings()
{
    gl::GLint nr_of_buffers;
    gl::glGetProgramInterfaceiv( m_program_, gl::GL_SHADER_STORAGE_BLOCK, gl::GL_ACTIVE_RESOURCES, &nr_of_buffers );

    for( int_fast32_t i = 0; i < nr_of_buffers; i++ )
    {
        static constexpr std::array kProperties{
            gl::GL_BUFFER_DATA_SIZE
        };
        char        name_buffer[ 128 ]{};
        gl::GLsizei name_size;

        gl::glGetProgramResourceName( m_program_, gl::GL_SHADER_STORAGE_BLOCK, i,
            sizeof( name_buffer ), &name_size, name_buffer );

        gl::GLsizei minimum_buffer_size;

        gl::glGetProgramResourceiv(
            m_program_, gl::GL_SHADER_STORAGE_BLOCK, i,
            kProperties.size(), kProperties.data(),
            kProperties.size(), nullptr, &minimum_buffer_size
        );
        gl::glShaderStorageBlockBinding( m_program_, i, i );

        sBuffer buffer;
        buffer.name        = std::string_view( name_buffer, name_size );
        buffer.pretty_name = make_pretty( buffer.name );
        buffer.binding     = i;
        buffer.size        = minimum_buffer_size;
        buffer.flags       = name_buffer[ 0 ] == '_' ? kHidden : 0;

        m_buffers_.emplace( buffer.name, std::move( buffer ) );
    }
}

void cShader_Reflection::add( sUniform& _uniform )
{
    m_global_uniforms_[ _uniform.name ] = std::move( _uniform );
}

void cShader_Reflection::add( sTexture& _texture )
{
    m_textures_[ _texture.name ] = std::move( _texture );
}

void cShader_Reflection::add( sSampler& _sampler )
{
    m_samplers_[ _sampler.name ] = std::move( _sampler );
}

auto cShader_Reflection::get_uniform( const gl::GLuint _index ) const -> std::variant< bool, sUniform, sTexture, sSampler >
{
    // TODO: Handle arrays.
    gl::GLsizei name_length;
    gl::GLsizei size;
    gl::GLenum  type;
    
    gl::glGetActiveUniform( m_program_, _index, static_cast< gl::GLsizei >( m_max_block_name_size_ ),
        &name_length, &size, &type, m_uniform_name_buffer_ );

    switch( type )
    {
    // Supported uniform formats:
    case gl::GL_INT:
    case gl::GL_INT_VEC2:
    case gl::GL_INT_VEC3:
    case gl::GL_INT_VEC4:
    case gl::GL_UNSIGNED_INT:
    case gl::GL_UNSIGNED_INT_VEC2:
    case gl::GL_UNSIGNED_INT_VEC3:
    case gl::GL_UNSIGNED_INT_VEC4:
    case gl::GL_FLOAT:
    case gl::GL_FLOAT_VEC2:
    case gl::GL_FLOAT_VEC3:
    case gl::GL_FLOAT_VEC4:
    case gl::GL_FLOAT_MAT2:
    case gl::GL_FLOAT_MAT3:
    case gl::GL_FLOAT_MAT4:
        return create_uniform( type, name_length, _index, size );
    
    // TODO: Support more samplers and textures
    case gl::GL_SAMPLER_2D:
        return create_sampler( type, name_length, _index );
        
    case gl::GL_IMAGE_2D:
        return create_texture( type, name_length, _index );
        
    default:
        SK_BREAK_RET_IF( sk::Severity::kGraphics, true,
            "Error: Got invalid uniform type", false )
    }
    
    return false;
}

auto cShader_Reflection::create_uniform( const gl::GLenum _type, const gl::GLsizei _name_length, const gl::GLuint _index, const gl::GLsizei _size ) const -> sUniform
{
    sUniform::eType type;
    uint16_t        byte_size;
    switch( _type )
    {
    case gl::GL_INT:               type = sUniform::eType::kInt;    byte_size = sizeof( int32_t );      break;
    case gl::GL_INT_VEC2:          type = sUniform::eType::kInt2;   byte_size = sizeof( int32_t ) * 2;  break;
    case gl::GL_INT_VEC3:          type = sUniform::eType::kInt3;   byte_size = sizeof( int32_t ) * 3;  break;
    case gl::GL_INT_VEC4:          type = sUniform::eType::kInt4;   byte_size = sizeof( int32_t ) * 4;  break;
    case gl::GL_UNSIGNED_INT:      type = sUniform::eType::kUInt;   byte_size = sizeof( uint32_t ); break;
    case gl::GL_UNSIGNED_INT_VEC2: type = sUniform::eType::kUInt2;  byte_size = sizeof( uint32_t ) * 2; break;
    case gl::GL_UNSIGNED_INT_VEC3: type = sUniform::eType::kUInt3;  byte_size = sizeof( uint32_t ) * 3; break;
    case gl::GL_UNSIGNED_INT_VEC4: type = sUniform::eType::kUInt4;  byte_size = sizeof( uint32_t ) * 4; break;
    case gl::GL_FLOAT:             type = sUniform::eType::kFloat;  byte_size = sizeof( float );    break;
    case gl::GL_FLOAT_VEC2:        type = sUniform::eType::kFloat2; byte_size = sizeof( float ) * 2;    break;
    case gl::GL_FLOAT_VEC3:        type = sUniform::eType::kFloat3; byte_size = sizeof( float ) * 3;    break;
    case gl::GL_FLOAT_VEC4:        type = sUniform::eType::kFloat4; byte_size = sizeof( float ) * 4;    break;
    // TODO: Support Matrix2x2f and Matrix3x3f
    // case gl::GL_FLOAT_MAT2:        type = sUniform::eType::kFloat2x2; break;
    // case gl::GL_FLOAT_MAT3:        type = sUniform::eType::kFloat3x3; break;
    case gl::GL_FLOAT_MAT4:        type = sUniform::eType::kFloat4x4; byte_size = sizeof( float ) * 4 * 4; break;
    default:
        SK_BREAK_RET_IF( sk::Severity::kGraphics, true, "Error: Invalid type provided.", sUniform{} )
        return sUniform{};
    }

    sUniform uniform;
    uniform.name     = std::string_view( m_uniform_name_buffer_, _name_length );
    uniform.pretty_name = make_pretty( uniform.name );
    uniform.type     = type;
    uniform.gl_type  = _type;
    uniform.location = _index;
    uniform.size     = static_cast< uint16_t >( _size );
    uniform.byte_size = byte_size;
    uniform.flags    = m_uniform_name_buffer_[ 0 ] == '_' ? kHidden : 0;
    
    return uniform;
}

auto cShader_Reflection::create_sampler( const gl::GLenum _type,
    const gl::GLsizei _name_length, const gl::GLuint _index ) const -> sSampler
{
    sSampler sampler;
    sampler.type = _type;
    sampler.name = std::string_view( m_uniform_name_buffer_, _name_length );
    sampler.pretty_name = make_pretty( sampler.name );
    sampler.location = static_cast< uint16_t >( gl::glGetUniformLocation( m_program_, sampler.name.string().c_str() ) );
    sampler.flags    = m_uniform_name_buffer_[ 0 ] == '_' ? kHidden : 0;
    return sampler;
}

auto cShader_Reflection::create_texture( const gl::GLenum _type,
    const gl::GLsizei _name_length, const gl::GLuint _index ) const -> sTexture
{
    SK_WARNING( sk::Severity::kGraphics, "Warning: Textures are deprecated. Do not use this!!" )
    SK_BREAK;
    sTexture texture;
    texture.type = _type;
    texture.name = std::string_view( m_uniform_name_buffer_, _name_length );
    texture.pretty_name = make_pretty( texture.name );
    texture.location = static_cast< uint16_t >( _index );
    texture.flags    = m_uniform_name_buffer_[ 0 ] == '_' ? kHidden : 0;
    return texture;
}

auto cShader_Reflection::make_pretty( const std::string_view _name ) -> std::string
{
    // TODO: Make it pretty.
    return std::string{ _name };
}

void cShader_Reflection::cleanup()
{
    SK_FREE( m_uniform_name_buffer_ );
    m_uniform_name_buffer_ = nullptr;
}
