#version 330

in vec4 vPosition;
in vec3 vNormal;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vPosition;
	normal = mat3(model) * vNormal;
}
