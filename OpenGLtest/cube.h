#pragma once
/**
	Structure and basic manipulations of Rubik's Cube.
 **/
#ifndef __CUBE_H__
#define __CUBE_H__

#include <utility>
#include <cstdint>


namespace rubiks_cube {
	struct face_t {
		enum face_type {
			top = 0,
			bottom = 1,
			front = 2,
			back = 3,
			left = 4,
			right = 5,
		};

		int8_t C[9];
	};

	struct block_t {
		int8_t top, bottom, front, back, left, right;
	};


	typedef std::pair<const int8_t*, const int8_t*> block_info_t;

	class cube_t {
	public:
		cube_t();
	public:
		block_t getBlock(int level, int x, int y) const;

		block_info_t getCornerBlock() const;
		block_info_t getEdgeBlock() const;
		// 旋转一个面90° * count
		void rotate(face_t::face_type, int count = 1);

	private:
		int8_t cp[8], co[8];	// 角块位置(position)与朝向(orientation)
		int8_t ep[12], eo[12];	// 边块位置与朝向
	};

}
#endif