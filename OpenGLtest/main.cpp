#include "viewer.h"
//#include "algo.h"
#include <random>
#include <cstdio>
#include <fstream>
#include <map>
#include <string>
#include <iostream>

using namespace rubiks_cube;

std::random_device rd;
std::mt19937 mt(rd());

const char* face_str = "UDFBLR";

void helper() 
{
	std::puts("-r generate a cube in random state");
	std::puts("-f <filename> generate a cube with state written in file");

	std::exit(0);
}

void put_rotate(int c1, int c2)
{
	std::putchar(face_str[c1]);
	if (c2 > 1) std::putchar(c2 == 2 ? '2' : '\'');
}

/* Todo */
void read_file(const char* filename)
{
	return;
}

int main(int argc, char** argv) {

	auto viewer = create_opengl_viewer();

	cube_t c;

	std::uniform_int_distribution<int> gen(0, 5);
	std::uniform_int_distribution<int> gen2(1, 3);

	std::puts("Generating cube...");
	c = cube_t();

	int random_times = 15;
	for (int i = 0; i < random_times; ++i)
	{
		int c1 = gen(mt), c2 = gen2(mt);
		c.rotate(face_t::face_type(c1), c2);
		put_rotate(c1, c2);
	}

	std::putchar('\n');

	viewer->init(argc, argv);
	viewer->set_rotate_duration(1.0);
	viewer->set_cube(c);

	viewer->run();

	return 0;
}