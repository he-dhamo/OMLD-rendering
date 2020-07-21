#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aLabel;

out vec2 TexCoord;
out vec3 instance;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
        gl_Position = projection * view * model * vec4(aPos, 1.0f);
        TexCoord = vec2(aTexCoords.x, aTexCoords.y);
        instance = aLabel;
}
