//
//  Script.h
//  EngineBase
//
//  Created by PORTOKALIS CHRISTOPHER G on 11/20/14.
//  Copyright (c) 2014 dave. All rights reserved.
//

#ifndef __EngineBase__Script__
#define __EngineBase__Script__

#include <stdio.h>
#include <string>



//include openGL stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace std;

//class Scene;

class Script
{
    
    virtual void setValue(string property, void* value) = 0;
    virtual void* getValue(string property) = 0;
    virtual void runScripts();
    
};



/*class MoveScript : public Script
{
    glm::vec3 moveVector;
    glm::vec3 scaleVector;
    glm::quat rotQuat;
    
    void setValue(string property, void* value);
    void* getValue(string property);
};*/

class ControlScript //: public Script
{
public:
    //Scene* scene;
    GLFWwindow* gWindow;
    float width;
    float height;
    
    void setValue(string property, void* value);
    void* getValue(string property);
    
    void runScripts();
    
    //scripts
    void keyboardControls();

};


#endif /* defined(__EngineBase__Script__) */
