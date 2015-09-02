#include "app.hpp"
#include <string>
#include <iostream>

using namespace knu::math;
using namespace knu::math::utility;

void App::general_setup()
{
	load_shaders();
	load_model();
	glEnable(GL_DEPTH_TEST);

	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vb);
	auto data = cubeModel.first_mesh().interleaved_array();
	glNamedBufferData(vb, sizeof(v3f) * 2 * cubeModel.meshes[0].vertex_count, data.data(), GL_STATIC_DRAW);
	
	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	
	glEnableVertexArrayAttrib(vao, 0);

	glVertexArrayAttribBinding(vao, 2, 0);
	glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE, sizeof(v3f));
	glEnableVertexArrayAttrib(vao, 2);

	glVertexArrayVertexBuffer(vao, 0, vb, 0, sizeof(v3f) * 2);
	glBindVertexArray(vao);
}

void App::load_model()
{
	cubeModel.load_obj("models/normalcube.mtl", "models/normalcube.obj");
}

void App::draw_scene()
{
	glClearBufferfv(GL_COLOR, 0, clearColorVal);
	glClearBufferfv(GL_DEPTH, 0, &clearDepthVal);
    
	auto rx = make_rotation_x<float>(degrees_to_radians<float>(timeSinceStart* 98));
	auto ry = make_rotation_y<float>(degrees_to_radians<float>(-timeSinceStart * 125));
	auto t = make_translate<float>(0.0f, 0.0f, -3.0f);
	auto transform = rx * ry * t;
	auto perspective = defaultProjectionMatrix;

	glProgramUniformMatrix4fv(objProgram.obj(), objProgram.uniform("model"), 1, GL_FALSE, transform.data());
	glProgramUniformMatrix4fv(objProgram.obj(), objProgram.uniform("projection"), 1, GL_FALSE, perspective.data());

	objProgram.bind();
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, cubeModel.meshes[0].vertex_count);
	glBindVertexArray(0);
}

void App::update(float seconds)
{

}

void App::load_shaders()
{
	objProgram.add_vertex_file("shaders/obj_vn.vs");
	objProgram.add_fragment_file("shaders/obj_vn.fs");
	objProgram.build();
}

void App::process_messages(SDL_Event *event)
{
	switch (event->type)
	{
	case SDL_WINDOWEVENT:
		{
			switch (event->window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				{
					resize(event->window.data1, event->window.data2);
				}break;

			case SDL_WINDOWEVENT_CLOSE:
				{
					window.set_quit(true);
				}break;
			}
		}break;

	case SDL_KEYDOWN:
		{
			if (event->key.keysym.sym == SDLK_ESCAPE)
				window.set_quit(true);
		}break;
            
        case SDL_QUIT:
        {
            window.set_quit(true);
        }break;
	
	}
}

void App::resize(int w, int h)
{
	glViewport(0, 0, w, h);
    defaultProjectionMatrix = make_perspective<float>(degrees_to_radians(70.0f), static_cast<float>(w) / h, 0.1f, 100.0f);
	defaultOrthographicMatrix = make_ortho<float>(0.0f, (float)w, 0.0f, (float)h, 0.01f, 1000.0f);
}

void App::get_window_size(int &w, int &h)
{
    window.get_window_size(w, h);
}

int App::window_width()
{
	int w, h;
	get_window_size(w, h);
	return w;
}

int App::window_height()
{
	int w, h;
	get_window_size(w, h);
	return h;
}
App::App():
	window(1024, 768, MAJOR_VERSION, MINOR_VERSION, false, 24, 0)
{
	window.set_event_callback(std::bind(&App::process_messages, this, std::placeholders::_1));
	clearColorVal[0] = 0.5f; clearColorVal[1] = 0.5f; clearColorVal[2] = 0.5f; clearColorVal[3] = 1.0f;
	clearDepthVal = 1.0f;
}

int App::run()
{
	startTime = lastTime = currentTime = std::chrono::steady_clock::now();
	initialize_graphics();
    general_setup();

	while (window.is_active())
	{
		currentTime = std::chrono::steady_clock::now();
		window.poll_events();
		timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
		update(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count() / 1000.0f);
		draw_scene();

        lastTime = currentTime;
		window.swap_buffers();
	}

	return 0;
}

void App::initialize_graphics()
{
#ifdef WIN32
	glDebugMessageCallback(&debug_output1, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
	int w, h; window.get_window_size(w, h);	resize(w, h);
}

void APIENTRY debug_output1(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam)
{
	std::string msg = std::string("source: ") + std::to_string(source) + "\n type: " + std::to_string(type)
		+ "\n id: " + std::to_string(id) + "\n severity: " + std::to_string(severity)
		+ "\n " + std::string(message) + "\n";

#ifdef WIN32
	OutputDebugStringA(msg.c_str());
#endif
	std::cerr << msg << std::endl;
}