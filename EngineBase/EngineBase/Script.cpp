//
//  Script.cpp
//  EngineBase
//
//  Created by PORTOKALIS CHRISTOPHER G on 11/20/14.
//  Copyright (c) 2014 dave. All rights reserved.
//

#include "Script.h"
#include "EngineUtil.h"


//***************************************************************
//Move Script Functions
//***************************************************************

/*void MoveScript::setValue(string property, void* value)
{
    
    
    if(property == "move")
    {
        glm::vec3* vecptr = (glm::vec3*) value;
        this->moveVector = *vecptr;
        
    }
    else if(property == "scale")
    {
        glm::vec3* vecptr = (glm::vec3*) value;
        this->moveVector = *vecptr;
        
        
    }
    else if(property == "rotate")
    {
        glm::quat* quatptr = (glm::quat*) value;
        this->rotQuat = *quatptr;
        
    }
    
    
    
    
}*/

//***************************************************************
//Control Script Functions
//***************************************************************

/*void ControlScript::setValue(string property, void* value)
{

    
    if(property == "scene")
    {
        this->scene = (Scene*) value;
        
        
        cout << "Scene script success!!! Player Name: " << scene->nodes["player"]->name << endl;
    }
    else if(property == "width")
    {
       float* floatPtr = (float*) value;
       this->width = *floatPtr;
        
       cout << "Script Width: " << width << endl;
    }
    else if(property == "height")
    {
        float* floatPtr = (float*) value;
        this->height = *floatPtr;
    }
    else if(property == "window")
    {
        
    }
    
    
}


void* ControlScript::getValue(string property)
{
    void* value;
    
    if(property == "scene")
    {
        value = this->scene;
    }
    else if(property == "height")
    {
        value = &this->height;
    }
    else if(property == "width")
    {
        value = &this->width;
    }
    
    return value;
}



void ControlScript::keyboardControls()
{

    bool playerPresent = false;
    
    if(scene->nodes["player"] != NULL)
    {
        playerPresent = true;
    }
    
    
    const float r = 0.01f;
    
    if (glfwGetKey(gWindow, 'W'))
    {
        
        if(playerPresent)
        {
            scene->nodes["player"]->meshInst->T.translateLocal(glm::vec3(0, 0, -1), scene->camera);
        }
        else
        {
            scene->camera.translateLocal(glm::vec3(0,0,-1));
        }
        
    }
    
    if (glfwGetKey(gWindow, 'S'))
    {
        if(playerPresent)
        {
            scene->nodes["player"]->meshInst->T.translateLocal(glm::vec3(0, 0, 1), scene->camera);
        }
        else
        {
            scene->camera.translateLocal(glm::vec3(0,0,1));
        }
        
    }
    
    if (glfwGetKey(gWindow, 'D'))
    {
        if(playerPresent)
        {
           scene->nodes["player"]->meshInst->T.translateLocal(glm::vec3(.2, 0, 0), scene->camera);
        }
        else
        {
            scene->camera.translateLocal(glm::vec3(.2,0,0));
        }
        
    }
    
    if (glfwGetKey(gWindow, 'A'))
    {
        if(playerPresent)
        {
            scene->nodes["player"]->meshInst->T.translateLocal(glm::vec3(-.2, 0, 0), scene->camera);
        }
        else
        {
            scene->camera.translateLocal(glm::vec3(-.2,0,0));
        }
        
    }
    if (glfwGetKey(gWindow, GLFW_KEY_DOWN))
    {
        if(playerPresent)
        {
            scene->nodes["player"]->meshInst->T.rotateLocal(glm::vec3(1,0,0), -.01);
        }
        else
        {
            scene->camera.rotateLocal(glm::vec3(1, 0, 0) , -.01);
        }
    }
    if (glfwGetKey(gWindow, GLFW_KEY_UP))
    {
        if(playerPresent)
        {
            scene->nodes["player"]->meshInst->T.rotateLocal(glm::vec3(1,0,0), .01);
        }
        else
        {
            scene->camera.rotateLocal(glm::vec3(1, 0, 0), .01);
        }
    }
    
    if (glfwGetKey(gWindow, GLFW_KEY_LEFT))
    {
        
        if(playerPresent)
        {
            scene->nodes["player"]->meshInst->T.rotateGlobal(glm::vec3(0 ,1, 0), r);
        }
        else
        {
            scene->camera.rotateGlobal(glm::vec3(0,1,0), r);
        }
    }
    if (glfwGetKey(gWindow, GLFW_KEY_RIGHT))
    {
        
        
        if(playerPresent)
        {
            scene->nodes["player"]->meshInst->T.rotateGlobal(glm::vec3(0,1,0), -r);
        }
        else
        {
            scene->camera.rotateGlobal(glm::vec3(0,1,0), -r);
        }
    }
    if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
    {
        //gScene.nextCamera(gWidth, gHeight);
    }
    
    scene->camera.refreshTransform((float)width, (float)height);
}

void ControlScript::runScripts(){
    
    keyboardControls();
}*/









