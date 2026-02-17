
#pragma once


#include <sk/Containers/Vector.h>
#include <sk/Misc/StringID.h>
#include <sk/Reflection/Types.h>

#include <glbinding/gl/types.h>

#include <cstdint>
#include <set>
#include <variant>

namespace sk::Graphics::Utils
{
    // TODO: Add a Reflection namespace
    enum eFlags : uint8_t
    {
        kNone   = 0,
            
        // If the variable is hidden
        kHidden  = 0x01,
        // ???
        kPartial = 0x02,
    };
        
    struct sAttribute
    {
        cStringID   name;
        std::string pretty_name;
            
        type_info_t type;
        
        gl::GLenum gl_type;
            
        uint32_t index;
        uint32_t stride;
        // Vector components
        uint8_t  components;
        // Array elements
        uint8_t  elements;
        uint16_t flags;
    };

    struct sUniform
    {
        enum class eType : uint8_t
        {
            kFloat,
            kFloat2,
            kFloat3,
            kFloat4,
            kFloat2x2,
            kFloat3x3,
            kFloat4x4,
            kInt,
            kInt2,
            kInt3,
            kInt4,
            kUInt,
            kUInt2,
            kUInt3,
            kUInt4,
        };
        cStringID   name;
        std::string pretty_name;

        gl::GLenum gl_type;
        // Either the index or bytewise offset.
        uint32_t location;
        // Number of elements
        uint16_t byte_size;
        uint16_t size;
        uint8_t  flags;
        eType    type;
    };
        
    struct sBlock
    {
        using uniform_map_t = std::unordered_map< str_hash, sUniform >;
            
        cStringID   name;
        std::string pretty_name;
            
        uniform_map_t uniforms;
        uint32_t      size;
        uint16_t      binding;
        uint16_t      flags;
    };

    struct sTexture
    {
        cStringID   name;
        std::string pretty_name;
        
        gl::GLenum type;
        uint16_t location;
        uint16_t flags;
    };

    struct sSampler
    {
        cStringID   name;
        std::string pretty_name;
        
        gl::GLenum type;
        uint16_t location;
        uint16_t flags;
    };

    struct sBuffer
    {
        cStringID   name;
        std::string pretty_name;

        // 0 = "infinite"
        uint32_t size;
        uint16_t binding;
        uint16_t flags;
    };

    class cShader_Reflection
    {
    public:
        explicit cShader_Reflection( gl::GLuint _program );
        
        [[ nodiscard ]] auto& GetAttributes() const { return m_attributes_; }
        [[ nodiscard ]] auto& GetBlockMap  () const { return m_block_map_; }
        [[ nodiscard ]] auto& GetBlockVec  () const { return m_block_vec_; }
        [[ nodiscard ]] auto& GetUniforms  () const { return m_global_uniforms_; }
        [[ nodiscard ]] auto& GetSamplers  () const { return m_samplers_; }
        [[ nodiscard ]] auto& GetBuffers   () const { return m_buffers_; }
        
    private:
        void init();
        
        void fetch_attributes();
        void fetch_uniforms();
        void fetch_blocks();
        void fetch_buffer_bindings();
        
        void add( sUniform& _uniform );
        void add( sTexture& _texture );
        void add( sSampler& _sampler );

        [[ nodiscard ]]
        auto get_uniform( gl::GLuint _index ) const -> std::variant< bool, sUniform, sTexture, sSampler >;

        [[ nodiscard ]]
        auto create_uniform( gl::GLenum _type, gl::GLsizei _name_length, gl::GLuint _index, gl::GLsizei _size ) const -> sUniform;
        auto create_sampler( gl::GLenum _type, gl::GLsizei _name_length, gl::GLuint _index ) const -> sSampler;
        auto create_texture( gl::GLenum _type, gl::GLsizei _name_length, gl::GLuint _index ) const -> sTexture;

        static auto make_pretty( std::string_view _name ) -> std::string;
        
        void cleanup();
        
        // For the reflection only.
        gl::GLuint m_program_;
        uint32_t   m_max_attribute_name_size_;
        uint32_t   m_max_block_name_size_;
        uint32_t   m_max_uniform_name_size_;
        
        gl::GLchar* m_uniform_name_buffer_;      
        
        std::set< str_hash > m_locked_uniforms_;
        
        std::vector< sAttribute > m_attributes_;
        
        unordered_map< str_hash, sUniform > m_global_uniforms_;
        unordered_map< str_hash, sBlock >   m_block_map_;
        vector< sBlock* >                   m_block_vec_;

        unordered_map< str_hash, sTexture > m_textures_;
        unordered_map< str_hash, sSampler > m_samplers_;
        unordered_map< str_hash, sBuffer >  m_buffers_;
    };
} // sk::Graphics::Utils::
