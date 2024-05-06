#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/cube.h"



class SceneBasic_Uniform : public Scene
{
private:
    std::unique_ptr<ObjMesh> mesh;
    float tPrev;
    float angle;
    GLSLProgram prog;
    void setMatrices();
    void compile();
    void setupFBO();
    void pass1();
    void pass2();
    GLuint fsQuad, renderTex, fboHandle;

public:
    SceneBasic_Uniform();
    
    void initScene();
    void update( float t, glm::vec3 Orientation, glm::vec3 Position, glm::vec3 Up);
    void render();
    void resize(int, int);

};

#endif // SCENEBASIC_UNIFORM_H
