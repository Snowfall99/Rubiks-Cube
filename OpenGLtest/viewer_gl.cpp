#include <Windows.h>
#include "viewer.h"
#include "virtual_rotate.h"
#include "camera.h"
#include <memory>
#include <chrono>
#include <tuple>
#include <queue>
#include <algorithm>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace rubiks_cube {
	int WINDOW_WIDTH = 600;
	int WINDOW_HEIGHT = 800;

	class rotate_manager_t {
		typedef std::chrono::system_clock clock_type;
		std::chrono::time_point<clock_type> t_s;
		double t;
		bool active;

	public:
		rotate_manager_t() : active(false) {}

		decltype(t_s) now()
		{
			return clock_type::now();
		}

		bool is_active()
		{
			return active;
		}

		double get()
		{
			std::chrono::duration<double> rate = now() - t_s;
			double r = rate.count() / t;
			if (r >= 1.0) active = false;
			return r;
		}

		void set(double duration)
		{
			t_s = now();
			t = duration;
			active = true;
		}
	};

	class viewer_gl : public viewer_t
	{
	public:
		viewer_gl();
		~viewer_gl() = default;
	public:
		void run();
		bool init(int&, char**&);
		void set_cube(const cube_t&);
		void set_rotate_duration(double);
		void add_rotate(face_t::face_type, int);
		void add_rotate(face_t::face_type, int, int);
	public:
		static void on_resize(GLFWwindow*, int, int);
		static void on_mouse_button(GLFWwindow*, int, int, int);
		static void on_mouse_move(GLFWwindow*, double, double);
		void processInput(GLFWwindow*);
		static void on_key_callback(GLFWwindow*, int, int, int, int);
	private:
		template<typename CubeType>
		void draw_cube(const CubeType&);
		void draw_block(GLfloat x, GLfloat y, GLfloat z, GLfloat size, block_t, GLenum);
		void update_rotate();
		void set_color(int);
	private:
		// face_type, depth, cnt
		typedef std::tuple<face_t::face_type, int, int> rotate_que_t;
		std::queue<rotate_que_t> rotate_que;

		int rotate_mask[3];
		GLfloat rotate_deg, rotate_vec;
		rotate_manager_t rotate_manager;
		double rotate_duration;

		virtual_ball_t vball;

		int cube_size;
		cube_t cube;

		GLFWwindow* window;
	};

	viewer_gl::viewer_gl()
	{
		window = nullptr;
		cube_size = 3;
		rotate_duration = 1;
		rotate_deg = rotate_vec = 0;
		std::fill(rotate_mask, rotate_mask + 3, -1);
	}

	bool viewer_gl::init(int&, char**&)
	{
		if (!glfwInit())
			return false;

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		window = glfwCreateWindow(600, 600, "Rubik's Cube", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			return false;
		}

		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, on_resize);
		glfwSetCursorPosCallback(window, on_mouse_move);
		glfwSetMouseButtonCallback(window, on_mouse_button);
		glfwSetKeyCallback(window, on_key_callback);

		glfwMakeContextCurrent(window);
		//glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		
		vball.set_rotate(45, { -1, 1, 0 });

		return true;
	}

	void viewer_gl::run()
	{
		while (!glfwWindowShouldClose(window))
		{
			processInput(window);

			glfwPollEvents();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			update_rotate();

			draw_cube(cube);

			glfwSwapBuffers(window);
		}

		glfwTerminate();
	}

	void viewer_gl::set_cube(const cube_t& cube)
	{
		cube_size = 3;
		this->cube = cube;
	}

	void viewer_gl::add_rotate(face_t::face_type type, int cnt)
	{
		add_rotate(type, 1, cnt);
	}


	void viewer_gl::add_rotate(face_t::face_type type, int depth, int cnt)
	{
		rotate_que.push({ type, depth, cnt % 4 });
	}

	void viewer_gl::update_rotate()
	{
		if (rotate_que.empty())
			return;

		face_t::face_type ftype;
		int depth, cnt;
		std::tie(ftype, depth, cnt) = rotate_que.front();

		if (!rotate_manager.is_active())
		{
			rotate_manager.set(rotate_duration);
			std::fill(rotate_mask, rotate_mask + 3, -1);

			rotate_vec = cnt < 0 ? -1 : 1;

			switch (ftype)
			{
			case face_t::top:
				rotate_mask[0] = cube_size - depth;
				break;
			case face_t::bottom:
				rotate_mask[0] = depth - 1;
				rotate_vec = -rotate_vec;
				break;
			case face_t::left:
				rotate_mask[2] = depth - 1;
				rotate_vec = -rotate_vec;
				break;
			case face_t::right:
				rotate_mask[2] = cube_size - depth;
				break;
			case face_t::front:
				rotate_mask[1] = cube_size - depth;
				rotate_vec = -rotate_vec;
				break;
			case face_t::back:
				rotate_mask[1] = depth - 1;
				break;
			}
		}

		double rate = rotate_manager.get();
		rotate_deg = std::abs(cnt) * 90.0 * rate;

		if (!rotate_manager.is_active())
		{
			std::fill(rotate_mask, rotate_mask + 3, -1);
			if (cube_size == 3) cube.rotate(ftype, cnt);
			rotate_que.pop();
		}
	}

	void viewer_gl::set_color(int type)
	{
		static const GLfloat colors[7][3] =
		{ { 0.0f, 1.0f, 0.0f }, // top    (green)
		  { 0.3f, 0.3f, 1.0f }, // bottom (blue)
		  { 1.0f, 0.3f, 0.3f }, // front  (red)
		  { 1.0f, 0.5f, 0.0f }, // back   (orange)
		  { 1.0f, 1.0f, 0.0f }, // left   (yellow)
		  { 1.0f, 1.0f, 1.0f }, // right  (white)
		  { 0.0f, 0.0f, 0.0f }  // frame
		};

		const GLfloat* C = colors[type];
		glColor3f(C[0], C[1], C[2]);
	}

	void viewer_gl::set_rotate_duration(double sec)
	{
		rotate_duration = sec;
	}

	void viewer_gl::draw_block(GLfloat x, GLfloat y, GLfloat z, GLfloat s, block_t color, GLenum type)
	{
		set_color(color.back);
		glBegin(type);
		glVertex3f(x, y, z);
		glVertex3f(x, y + s, z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y, z);
		glEnd();

		set_color(color.front);
		glBegin(type);
		glVertex3f(x, y, z - s);
		glVertex3f(x, y + s, z - s);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x + s, y, z - s);
		glEnd();

		set_color(color.top);
		glBegin(type);
		glVertex3f(x, y + s, z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x, y + s, z - s);
		glEnd();

		set_color(color.bottom);
		glBegin(type);
		glVertex3f(x, y, z);
		glVertex3f(x + s, y, z);
		glVertex3f(x + s, y, z - s);
		glVertex3f(x, y, z - s);
		glEnd();

		set_color(color.left);
		glBegin(type);
		glVertex3f(x, y, z);
		glVertex3f(x, y + s, z);
		glVertex3f(x, y + s, z - s);
		glVertex3f(x, y, z - s);
		glEnd();

		set_color(color.right);
		glBegin(type);
		glVertex3f(x + s, y, z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x + s, y, z - s);
		glEnd();
	}

	template<typename CubeType>
	void viewer_gl::draw_cube(const CubeType& cube)
	{
		class rotate_guard
		{
			bool is_rotated;
		public:
			rotate_guard(int mask, int real, GLfloat deg, GLfloat X, GLfloat Y, GLfloat Z)
			{
				if (mask == real)
				{
					is_rotated = true;
					glPushMatrix();
					glRotatef(deg, X, Y, Z);
				}
				else is_rotated = false;
			}

			~rotate_guard()
			{
				if (is_rotated)
					glPopMatrix();
			}
		};

		GLfloat size = 0.8f / cube_size;

		glPushMatrix();

		vball.rotate();
		glLineWidth(1.5f);

		GLfloat base = -size * cube_size * 0.5f, x, y, z;
		x = y = base, z = -base;

		for (int i = 0; i != cube_size; ++i, y += size, x = base, z = -base)
		{
			rotate_guard _guard(rotate_mask[0], i, rotate_deg, 0, rotate_vec, 0);
			for (int j = 0; j != cube_size; ++j, z -= size, x = base)
			{
				rotate_guard _guard(rotate_mask[1], j, rotate_deg, 0, 0, rotate_vec);
				for (int k = 0; k != cube_size; ++k, x += size)
				{
					rotate_guard _guard(rotate_mask[2], k, rotate_deg, rotate_vec, 0, 0);
					draw_block(x, y, z, size, cube.getBlock(i, j, k), GL_QUADS);
					draw_block(x, y, z, size, { 6, 6, 6, 6, 6, 6 }, GL_LINE_LOOP);
				}
			}
		}

		glPopMatrix();
	}

	void viewer_gl::on_resize(GLFWwindow* window, int w, int h)
	{
		glfwMakeContextCurrent(window);

		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	void viewer_gl::on_mouse_button(GLFWwindow* window, int button, int action, int)
	{
		if (button != GLFW_MOUSE_BUTTON_LEFT)
			return;

		viewer_gl* viewer = reinterpret_cast<viewer_gl*>(glfwGetWindowUserPointer(window));

		int w, h;
		double x, y;
		glfwGetWindowSize(window, &w, &h);
		glfwGetCursorPos(window, &x, &y);
		if (action == GLFW_PRESS)
		{
			viewer->vball.set_start(x / w - 0.5, y / h - 0.5);
		}
		else if (action == GLFW_RELEASE) {
			viewer->vball.set_end(x / w - 0.5, y / h - 0.5);
		}
	}

	void viewer_gl::on_mouse_move(GLFWwindow* window, double x, double y)
	{
		viewer_gl* viewer = reinterpret_cast<viewer_gl*>(glfwGetWindowUserPointer(window));

		if (!viewer->vball)
			return;

		int w, h;
		glfwGetWindowSize(window, &w, &h);
		viewer->vball.set_middle(x / w - 0.5, y / h - 0.5);
	}

	void viewer_gl::on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		// ��ȡ��ǰ����ָ��
		viewer_gl* viewer = reinterpret_cast<viewer_gl*>(glfwGetWindowUserPointer(window));

		const char* face_str = "UDFBLR";

		if (key == GLFW_KEY_U && action == GLFW_PRESS) {
			viewer->add_rotate(face_t::top, 1);
			std::putchar(face_str[0]);
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			viewer->add_rotate(face_t::bottom, 1);
			std::putchar(face_str[1]);
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS) {
			viewer->add_rotate(face_t::front, 1);
			std::putchar(face_str[2]);
		}
		if (key == GLFW_KEY_B && action == GLFW_PRESS) {
			viewer->add_rotate(face_t::back, 1);
			std::putchar(face_str[3]);
		}
		if (key == GLFW_KEY_L && action == GLFW_PRESS) {
			viewer->add_rotate(face_t::left, 1);
			std::putchar(face_str[4]);
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			viewer->add_rotate(face_t::right, 1);
			std::putchar(face_str[5]);
		}
		return;
	}

	void viewer_gl::processInput(GLFWwindow* window) 
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}

	std::shared_ptr<viewer_t> create_opengl_viewer()
	{
		return std::make_shared<viewer_gl>();
	}
}
