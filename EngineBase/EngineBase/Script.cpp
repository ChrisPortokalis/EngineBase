//
//  Script.cpp
//  EngineBase
//
//  Created by PORTOKALIS CHRISTOPHER G on 11/20/14.
//  Copyright (c) 2014 dave. All rights reserved.
//

#include "Script.h"
#include "EngineUtil.h"


void MoveScript::setValue(string property, void* value)
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
    
    
    
    
}