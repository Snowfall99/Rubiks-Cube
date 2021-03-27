#version 330 core
out vec4 FragColor;

uniform vec4 ourColor;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = ourColor;
}