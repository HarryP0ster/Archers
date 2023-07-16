#version 460 core
layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragNorm;
layout (location = 2) in vec3 position;

out vec4 outColor;

void main()
{
    const vec3 lightPos = vec3(25, 50, 0);

    float diffuse =  max( 0.65, dot( normalize(fragNorm),  normalize(lightPos - position) ) );
    outColor = vec4(fragColor * diffuse, 1.0);
} 