#ifndef KNU_APP
#define KNU_APP

#include <chrono>
#include <knu/window2.hpp>
#include <knu/gl_utility.hpp>
#include <knu/mathlibrary5.hpp>
#include "obj.hpp"

#ifdef WIN32
#define MAJOR_VERSION 4
#define MINOR_VERSION 5
#endif
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#define MAJOR_VERSION 3         // In sdl, for the time being, the only way to request a 4.1 context is
#define MINOR_VERSION 2         // to request a 3.2 context on mac
#endif

void APIENTRY debug_output1(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam);
class App
{
	Window window;
	std::chrono::steady_clock::time_point currentTime, lastTime, startTime;
	float clearColorVal[4];
	float clearDepthVal;
    knu::math::m4f defaultProjectionMatrix;
	knu::math::m4f defaultOrthographicMatrix;
	knu::Obj cubeModel;

	float timeSinceStart;
	knu::graphics::program objProgram;

	GLuint vao, vb;

private:
	void general_setup();
	void load_model();
	void draw_scene();
	void update(float seconds);
	void load_shaders();
	void initialize_graphics();
	void process_messages(SDL_Event *event);
	void resize(int w, int h);
    void get_window_size(int &w, int &h);
	int window_width();
	int window_height();

public:
	App();
	int run();
};

#endif