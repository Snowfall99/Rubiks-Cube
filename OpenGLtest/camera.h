#pragma once
#include <cmath>

class camera
{
public:
	camera();

	float camera_x, camera_y, camera_z;
	float lookat_x, lookat_y, lookat_z;

	void walk_straight(float);
	void walk_tansverse(float);

	float angle;
	float speed;
	float sight;
	float rad_yz, rad_xz;
	float rotate_yz, rotate_xz;

	float PI;
};

camera::camera()
{
	PI = 3.1415;

	angle = 3;
	speed = 0.3;
	sight = 100;

	rotate_yz = 0.0f;
	rotate_xz = -90.0f;
	rad_yz = rotate_yz * PI / 180.0f;
	rad_xz = rotate_xz * PI / 180.0f;

	camera_x = 0.0f;
	camera_y = 0.0f;
	camera_z = 8.0f;

	lookat_x = camera_x + sight * cos(rad_yz) * cos(rad_xz);
	lookat_y = camera_y + sight * sin(rad_yz);
	lookat_z = camera_z + sight * cos(rad_yz) * sin(rad_xz);
}

void camera::walk_straight(float fSpeed)
{
	camera_x += fSpeed * cos(rad_yz) * cos(rad_xz);
	camera_y += fSpeed * sin(rad_yz);
	camera_z += fSpeed * cos(rad_yz) * sin(rad_xz);

	lookat_x = camera_x + sight * cos(rad_yz) * cos(rad_xz);
	lookat_y = camera_y + sight * sin(rad_yz);
	lookat_z = camera_z + sight * cos(rad_yz) * sin(rad_xz);
}

void camera::walk_tansverse(float fSpeed)
{
	camera_x += fSpeed * cos(rad_yz) * sin(rad_xz);
	camera_z -= fSpeed * cos(rad_yz) * cos(rad_xz);

	lookat_x = camera_x + sight * cos(rad_yz) * cos(rad_xz);
	lookat_y = camera_y + sight * sin(rad_yz);
	lookat_z = camera_z + sight * cos(rad_yz) * sin(rad_xz);
}