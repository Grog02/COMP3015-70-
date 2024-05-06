#include <glad/glad.h>
#include "scene.h"
#include <GLFW/glfw3.h>
#include "glutils.h"


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#include <chrono>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>


class SceneRunner {
public:
	GLFWwindow* window;
private:

	int fbw, fbh;
	bool debug;           // Set true to enable debug messages


public:
	float deltaT = 0.0f;
	float camSpeed = 0.01f;		
	glm::vec3 orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 position = glm::vec3(0.0f, 5.0f, 30.0f);
	glm::mat4	 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	bool firstClick = true;
	float camSensitivity = 100.0f;
	SceneRunner(const std::string& windowTitle, int width = WIN_WIDTH, int height = WIN_HEIGHT, int samples = 0) : debug(true) {
		// Initialize GLFW
		if (!glfwInit()) exit(EXIT_FAILURE);

#ifdef __APPLE__
		// Select OpenGL 4.1
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
		// Select OpenGL 4.6
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		if (debug)
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		}
		if (samples > 0) {
			glfwWindowHint(GLFW_SAMPLES, samples);
		}

		// Open the window
		window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, windowTitle.c_str(), NULL, NULL);
		if (!window) {
			std::cerr << "Unable to create OpenGL context." << std::endl;
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
		glfwMakeContextCurrent(window);

		// Get framebuffer size
		glfwGetFramebufferSize(window, &fbw, &fbh);

		// Load the OpenGL functions.
		if (!gladLoadGL()) { exit(-1); }

		GLUtils::dumpGLInfo();

		// Initialization
		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
#ifndef __APPLE__
		if (debug) {
			glDebugMessageCallback(GLUtils::debugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Start debugging");
		}
#endif
	}

	int run(Scene& scene) {
		scene.setDimensions(fbw, fbh);
		scene.initScene();
		scene.resize(fbw, fbh);

		// Enter the main loop
		mainLoop(window, scene);

#ifndef __APPLE__
		if (debug)
		{
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "End debug");
		}
#endif

		// Close window and terminate GLFW
		glfwTerminate();

		// Exit program
		return EXIT_SUCCESS;
	}

	static std::string parseCLArgs(int argc, char** argv, std::map<std::string, std::string>& sceneData) {
		if (argc < 2) {
			printHelpInfo(argv[0], sceneData);
			exit(EXIT_FAILURE);
		}

		std::string recipeName = argv[1];
		auto it = sceneData.find(recipeName);
		if (it == sceneData.end()) {
			printf("Unknown recipe: %s\n\n", recipeName.c_str());
			printHelpInfo(argv[0], sceneData);
			exit(EXIT_FAILURE);
		}

		return recipeName;
	}

private:
	static void printHelpInfo(const char* exeFile, std::map<std::string, std::string>& sceneData) {
		printf("Usage: %s recipe-name\n\n", exeFile);
		printf("Recipe names: \n");
		for (auto it : sceneData) {
			printf("  %11s : %s\n", it.first.c_str(), it.second.c_str());
		}
	}
	
	void mainLoop(GLFWwindow* window, Scene& scene) {
		auto startTime = std::chrono::high_resolution_clock::now();
		while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			auto currentTime = std::chrono::high_resolution_clock::now();

			// calculate time 
			deltaT = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			GLUtils::checkForOpenGLError(__FILE__, __LINE__);
			// update scene with orientation, position and up of the camera
			scene.update(float(glfwGetTime()), orientation, position, up);
			// render scene
			scene.render();

			// swap front and back buffers and process events
			glfwSwapBuffers(window);
			glfwPollEvents();

			int state = glfwGetKey(window, GLFW_KEY_SPACE);
			if (state == GLFW_PRESS)
			{
				scene.animate(!scene.animating());
			}

			/* camera movement using keyboard input
			 W - Forward
			 A - Left
			 S - Backwards
			 D - Right
			 Up Arrow - Up
			 Down Arrow - Down
			 Left Shift - Increase Speed
			 */
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				position += camSpeed *  deltaT * orientation;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				position += camSpeed * deltaT * -glm::normalize(glm::cross(orientation, up));
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				position += camSpeed * deltaT * -orientation;
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				position += camSpeed * deltaT * glm::normalize(glm::cross(orientation, up));
			}
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				position += camSpeed * deltaT * up;
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				position += camSpeed * deltaT * -up;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				camSpeed = 0.02;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
			{
				camSpeed = 0.01;
			}
			// Mouse control 
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				// hides mouse when holding down left click
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				// Reset mouse position when clicking
				if (firstClick)
				{
					glfwSetCursorPos(window, (WIN_WIDTH / 2), (WIN_HEIGHT / 2));
					firstClick = false;
				}

				// Mouse coordinates
				double mouseX;
				double mouseY;

				// Get mouse position
				glfwGetCursorPos(window, &mouseX, &mouseY);

				// Calculate rotation based on Mouse sensitivity
				float rotX = camSensitivity * (float(mouseY - (WIN_HEIGHT / 2)) / WIN_HEIGHT);
				float rotY = camSensitivity * (float(mouseX - (WIN_WIDTH / 2)) / WIN_WIDTH);


				// Update the camera's orientation
				orientation = glm::rotate(orientation, glm::radians(-rotY), up);
				
				glm::vec3 newOrientation = glm::rotate(orientation, glm::radians(-rotX), glm::normalize(glm::cross(orientation, up)));

				if (!((glm::angle(newOrientation, up) <= glm::radians(5.0f)) or glm::angle(newOrientation, -up) <= glm::radians(5.0f)))
				{
					orientation = newOrientation;
				}

				// Update the camera's position based on the new orientation
				updateMatrix(45.0f, 0.1f, 100.0f);

				// reset mouse position 
				glfwSetCursorPos(window, (WIN_WIDTH / 2), (WIN_HEIGHT / 2));
			}

			else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
			{
				// Show mouse after releasing left click
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				firstClick = true;
			}

		}
	}
	// Update matrices of camera 
	void updateMatrix(float FOVdeg, float nearPlane, float farPlane)
	{
		// update view matrix based on current position and orientation
		view = glm::lookAt(position, position + glm::vec3(12.0f, 4.0f, 1.0f), up);

		// update projection matrix based on current field of view, aspect ratio of the window and near/far plane
		projection = glm::perspective(glm::radians(FOVdeg), (float)(WIN_WIDTH / WIN_HEIGHT), nearPlane, farPlane);

		// combine projection and view matrices for the camera matrix
		cameraMatrix = projection * view;
	}
};
