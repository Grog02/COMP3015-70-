#pragma once

#include <glm/glm.hpp>

class Scene
{
protected:
	glm::mat4 model, view, projection;

public:
    int width;
    int height;

	Scene() : m_animate(true), width(800), height(600) { }
	virtual ~Scene() {}

	void setDimensions( int w, int h ) {
	    width = w;
	    height = h;
	}
	
    /**
      Load textures, initialize shaders, etc.
      */
    virtual void initScene() = 0;
    /*virtual void pressW() = 0;
    virtual void pressA() = 0;
    virtual void pressS() = 0;
    virtual void pressD() = 0;
    virtual void pressUp() = 0;
    virtual void pressDown() = 0;
    virtual void mouseClick() = 0;
    virtual void mouseRelease() = 0;*/
    
    /**
      This is called prior to every frame.  Use this
      to update your animation.
      */
    virtual void update(float t, glm::vec3 Orientation, glm::vec3 Position, glm::vec3 Up) = 0;

    /**
      Draw your scene.
      */
    virtual void render() = 0;

    /**
      Called when screen is resized
      */
    virtual void resize(int, int) = 0;
    
    
    void animate( bool value ) { m_animate = value; }
    bool animating() { return m_animate; }
    
protected:
	bool m_animate;
};
