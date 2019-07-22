#ifndef GL_GRAPHICS_H
#define GL_GRAPHICS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct
{
    u32 id = 0;
    
    void apply();
    void create(const char* const vsrc, const char* const fsrc);
} GLShader;

typedef struct
{
    u32 texture_id;
    i32 width, height;
    i32 rows, cols;
}  GLTilemapTexture;

typedef struct
{
    GLTilemapTexture const* texture;
} Tilemap;


internal GLTilemapTexture
gl_load_rgba_tilemap(
u8* data,
i32 width,
i32 height,
i32 tilemap_rows,
i32 tilemap_cols);

internal void
gl_slow_tilemap_draw(
GLTilemapTexture const* tm,
glm::vec2 pos,
glm::vec2 size,
float rotate = 0.f,
i32 tm_index = 0,
b32 mirrorx = false,
b32 mirrory = false);

const char* const g_2d_vs =
R"EOS(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}

)EOS";
const char* const g_2d_fs =
R"EOS(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2DArray sampler;
uniform int layer = 0;

void main()
{    
      color =  texture(sampler, vec3(TexCoords,layer));
      //color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    
)EOS";
///////////////////////////////////////////////////////////
const char* const g_debug_line_vs =
R"EOS(
#version 330 core
layout (location = 0) in vec2 vertex; // <vec2 position>

uniform mat4 model;
uniform mat4 projection;

void main()
{
    gl_Position = projection * model * vec4(vertex, 0.0, 1.0);
}

)EOS";
const char* const g_debug_line_fs =
R"EOS(
#version 330 core
out vec4 color;

void main()
{    
      color = vec4(1.0, 0.0, 0.0, 1.0);
    }
    
)EOS";

#endif //! GL_GRAPHICS_H