#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 ourColor;

uniform int direction;

uniform vec3 up_color;
uniform vec3 down_color;
uniform vec3 right_color;
uniform vec3 left_color;
uniform vec3 front_color;
uniform vec3 back_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	if (direction == 0) {
		ourColor = back_color;
	} else if (direction == 1) {
		ourColor = vec3(0.0f, 255.0f, 0.0f);
	} else {
		ourColor = vec3(0.0f, 0.0f, 255.0f);
	}
	
}