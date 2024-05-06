#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>
#include <string>
using std::string;
#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include "helper/texture.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/scenerunner.h"


using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;


// Textures
GLuint mountain;
GLuint texture2;
float deltaT;


// Load Mountain model
SceneBasic_Uniform::SceneBasic_Uniform(){
	mesh = ObjMesh::load("media/mountain/mountain3015.obj", true);
}

// Initialise the scene
void SceneBasic_Uniform::initScene()
{
	// compile and enable depth testing 
    compile();
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	// initial transform matrices
	model = mat4(1.0f);
    view = glm::lookAt(vec3(1.0f, 1.25f, 3.25f ), vec3(0.0f, 0.2f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(45.0, 4.0 / 3.0, 1.0, 50.0);
	angle = 0.0f;
	
	
	// lighting and fog uniforms
    prog.setUniform("Light.Position", view * glm::vec4(5.0f, 5.0f, 2.0f, 1.0f));
    prog.setUniform("Light.L", vec3(1.0f));
    prog.setUniform("Light.La", vec3(0.05f));
	prog.setUniform("ModelMatrix", model);
	prog.setUniform("ViewMatrix", view);
	prog.setUniform("ProjectionMatrix", projection);

	prog.setUniform("Fog.MaxDist", 40.0f);
	prog.setUniform("Fog.MinDist", 1.0f);
	prog.setUniform("Fog.Color", vec3(0.9f, 0.9f, 0.9f));

	// loading textures to be used
	mountain = Texture::loadTexture("media/texture/snow_02_diff_2k.jpg");
	texture2 = Texture::loadTexture("media/texture/gray_rocks_diff_2k.jpg");

	setupFBO();

	GLfloat verts[] = {
		-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	};

	GLfloat tc[] = {
		1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	};

	unsigned int handle[2];
	glGenBuffers(2, handle);
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

	glGenVertexArrays(1, &fsQuad);
	glBindVertexArray(fsQuad);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	prog.setUniform("EdgeThreshold", 0.13f);
	prog.setUniform("Light.L", vec3(0.9f));
	prog.setUniform("Light.La", vec3(0.5f));
	
}
// Compile and link Shaders
void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::setupFBO()
{
	glGenFramebuffers(1, &fboHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

	glGenTextures(1, &renderTex);
	glBindTexture(GL_TEXTURE_2D, renderTex);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

	GLuint depthBuf;
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (result == GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Frame buffer is complete!" << std::endl;
	}
	else {
		std::cout << "Frame buffer error! : " << result << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::pass1()
{

	prog.setUniform("Pass", 1);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, width, height);

	projection = glm::perspective(glm::radians(100.0f), (float)width / height, 0.3f, 100.0f);
	// update light position based on current angle
	vec4 lightPos = vec4(10.0f * cos(angle), 10.0f, 10.0f * sin(angle), 1.0f);

	prog.setUniform("Light.Position", vec4(view * lightPos));

	// set material properties 
	prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
	prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
	prog.setUniform("Material.Ka", vec3(0.2f * 0.3f, 0.55f * 0.3f, 0.9f * 0.3f));
	prog.setUniform("Material.Shininess", 100.0f);

	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	setMatrices();

	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
	setMatrices();

	// bind textures and render the mesh
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mountain);
	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.0f, 0.0f, 0.0f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 180.0f);
	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	setMatrices();
	mesh->render();


}

void SceneBasic_Uniform::pass2()
{
	prog.setUniform("Pass", 2);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	model = mat4(1.0f);
	view = mat4(1.0f);
	projection = mat4(1.0f);

	setMatrices();

	glBindVertexArray(fsQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// Update scene based on time and camera values
void SceneBasic_Uniform::update( float t, glm::vec3 Orientation, glm::vec3 Position, glm::vec3 Up )
{
	
	deltaT = t - tPrev;
	if (tPrev == 0.0f)
	{
		deltaT = 0.0f;
	}
	tPrev = t;
	angle += 0.1f * deltaT;
	if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();
	prog.setUniform("ViewMatrix", view);
	view = glm::lookAt(Position, Position + Orientation, Up);
	
	

}

// Render scene
void SceneBasic_Uniform::render()
{
	pass1();
	glFlush();
	pass2();	
}

// Adjust viewport size and update projection based on window size
void SceneBasic_Uniform::resize(int w, int h)
{
	glViewport(0, 0, w, h);
    width = w;
    height = h;
    
    
	projection = glm::perspective(glm::radians(70.0f), (float) w / h, 0.3f, 100.0f);
}

// Update shader values based on model, view and projection matrices
void SceneBasic_Uniform::setMatrices() 
{
	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix",mv);
	prog.setUniform("NormalMatrix",glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);

}


