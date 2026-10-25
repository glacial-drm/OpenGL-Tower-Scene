#version 330 core
layout (location = 0) in vec3 vertex; // <vec2 position, vec2 texCoords>
layout (location = 1) in vec2 tex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 offset;
uniform vec4 color;

void main()
{
    float scale = 10.0f;

    TexCoords = tex;
    ParticleColor = color;
    gl_Position = projection * view * model * vec4((vertex * scale) + offset, 1.f); // edited, didn't have view and model before
}