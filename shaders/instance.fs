#version 330 core
out vec4 FragColor;

in vec3 instance;

void main()
{             
    float b = float(instance.z / 255.0);
    float g = float(instance.y / 255.0);
    float r = float(instance.x / 255.0);

    FragColor = vec4(vec3(r, g, b), 1.0);
}
