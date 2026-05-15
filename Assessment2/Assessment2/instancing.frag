#version 330 core
out vec4 FragColor;

in vec3 col;

uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = vec4(col, 1.f);
}