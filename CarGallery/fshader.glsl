#version 330

in vec3 normal;
out vec4 fColor;

uniform vec3 objectColor;

// TEMPORARY until Phong lighting (next step): a fixed direction so adjacent walls get different brightness
const vec3 tempLightDir = vec3(0.3, 0.8, 0.5);

void main()
{
	float shade = 0.35 + 0.65 * abs(dot(normalize(normal), normalize(tempLightDir)));
	fColor = vec4(objectColor * shade, 1.0);
}
