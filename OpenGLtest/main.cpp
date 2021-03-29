#include "viewer.h"
#include "algo.h"
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
move_seq_t read_file(const char* filename)
{
	// Read command from file
	std::ifstream myfile(filename);
	if (!myfile.is_open()) {
		std::puts("Unable to open file\n");
		std::exit(-1);
	}
	std::string line;
	getline(myfile, line);
	myfile.close();

	// add command into move sequence
	move_seq_t move_seq;
	for (int i = 0; i < line.length(); i++) {
		if (line[i] == 'U')
			move_seq.push_back(move_step_t{ face_t::top, 1 });
		if (line[i] == 'D')
			move_seq.push_back(move_step_t{ face_t::bottom, 1 });
		if (line[i] == 'R')
			move_seq.push_back(move_step_t{ face_t::right, 1 });
		if (line[i] == 'L')
			move_seq.push_back(move_step_t{ face_t::left, 1 });
		if (line[i] == 'F')
			move_seq.push_back(move_step_t{ face_t::front, 1 });
		if (line[i] == 'B')
			move_seq.push_back(move_step_t{ face_t::back, 1 });
	}
	
	return move_seq;
}

int main(int argc, char** argv) {

	auto viewer = create_opengl_viewer();

	cube_t c;
	c = cube_t();
	
	// make a choice
	std::puts("Random state or read state from a file:\n");
	std::puts("1. Random state\n");
	std::puts("2. Read state from a file\n");

	int choice;
	std::cin >> choice;

	// Generating cube
	move_seq_t move_seq;
	std::puts("Generating cube...");
	if (choice == 2) {
		move_seq = read_file("command.txt");
	}
	else if (choice == 1) {
		std::uniform_int_distribution<int> gen(0, 5);
		std::uniform_int_distribution<int> gen2(1, 3);

		int random_times = 15;
		for (int i = 0; i < random_times; ++i)
		{
			int c1 = gen(mt), c2 = gen2(mt);
			move_seq.push_back(move_step_t{ face_t::face_type(c1), c2 });
		}
	}
	else {
		std::puts("Undefined choice number!\n");
		std::exit(-1);
	}

	for (move_step_t& step : move_seq) {
		c.rotate(step.first, step.second);
		put_rotate(step.first, step.second);
	}

	std::putchar('\n');

	viewer->init(argc, argv);
	viewer->set_rotate_duration(1.0);
	viewer->set_cube(c);

	viewer->run();

	return 0;
}