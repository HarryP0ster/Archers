#version 460 core
layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec3 vecNorm;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragNorm;
layout (location = 2) out vec3 position;

layout (location = 0) uniform mat4 mWorld;
layout (location = 1) uniform mat4 mProj;
layout (location = 2) uniform mat4 mView;
layout (location = 3) uniform vec3 scale;
layout (location = 4) uniform vec3 color;

void main()
{
    gl_Position = mProj * mView * mWorld * vec4(scale * vecPos, 1.0);
    fragColor = color;
    fragNorm = vecNorm;
    position = vecPos;
}