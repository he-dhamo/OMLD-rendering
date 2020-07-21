#version 330 core
out vec4 FragColor;

uniform float near; 
uniform float far;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	

}

void main()
{             
    float depth = LinearizeDepth(gl_FragCoord.z); 

    depth = 1000 * depth;

    int depth_b = int(depth)%int(256);
    int depth_r = int(depth)/int(256);
    float b = float(depth_b / 255.0);
    float r = float(depth_r / 255.0);

    FragColor = vec4(vec3(r, r, b), 1.0);
}
