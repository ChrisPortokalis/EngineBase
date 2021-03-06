
//-------------------------------------------------------------------------//
// EngineUtil.h
// Some utility stuff - file reading and such
//
// David Cline
// 8/23/2014
//-------------------------------------------------------------------------//

// Prevent visual studio warnings
#define _CRT_SECURE_NO_WARNINGS


// some standard includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
using namespace std;

// lodePNG stuff (image reading)
#include "lodepng.h"

// OpenGL related includes
#define GLM_FORCE_RADIANS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Sound (irrKlang)
#include <irrKlang.h>
using namespace irrklang;
#pragma comment(lib, "irrKlang.lib")

//forward declarations
class Camera;
class Node;
class MoveScript;
class ControlScript;
class SpawnScript;

//-------------------------------------------------------------------------//
// MISCELLANEOUS
//-------------------------------------------------------------------------//

void ERROR(const string &msg, bool doExit = true);
double TIME(void);
void SLEEP(int millis);

//-------------------------------------------------------------------------//
// TEMPLATES
//-------------------------------------------------------------------------//

template<class T> class NameVal
{
public:
	string name;
	T val;
	NameVal() { name = ""; }
	NameVal(string &n, T &v) { name = n; val = v; }
};

template<class T> class NameIdVal
{
public:
	string name;
	int id;
	T val;
	NameIdVal() { name = ""; id = -1; }
	NameIdVal(string &n, int i, T &v) { name = n; id = i; val = v; }
};

//-------------------------------------------------------------------------//
// OPENGL STUFF
//-------------------------------------------------------------------------//

#define MAJOR_VERSION 4
#define MINOR_VERSION 1
GLFWwindow* createOpenGLWindow(int width, int height, const char *title, int samplesPerPixel=0);

#define NULL_HANDLE 0
GLuint loadShader(const string &fileName, GLuint shaderType);
GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader);

//-------------------------------------------------------------------------//
// GLM UTILITY STUFF
//-------------------------------------------------------------------------//

inline void printVec(const glm::vec2 &v) { printf("[%1.3f %1.3f]\n", v[0], v[1]); }
inline void printVec(const glm::vec3 &v) { printf("[%1.3f %1.3f %1.3f]\n", v[0], v[1], v[2]); }
inline void printVec(const glm::vec4 &v) { printf("[%1.3f %1.3f %1.3f %1.3f]\n", v[0], v[1], v[2], v[3]); }
inline void printQuat(const glm::quat &q) { printf("[%1.3f %1.3f %1.3f %1.3f]\n", q[0], q[1], q[2], q[3]); }
void printMat(const glm::mat4x4 &m);

//-------------------------------------------------------------------------//
// FILE READING
//-------------------------------------------------------------------------//

void addToPath(const string &p);
void removeFromPath(const string &p);
void printPath();

bool getFullFileName(const string &fileName, string &fullName);
FILE *openFileForReading(const string &fileName);

bool getToken(FILE *f, string &token, const string &oneCharTokens);
int getFloats(FILE *f, float *a, int num);
int getInts(FILE *f, int *a, int num);

bool loadFileAsString(const string &fileName, string &buffer);

void replaceIncludes(string &src, string &dest, const string &directive, 
	string &alreadyIncluded, bool onlyOnce);

//-------------------------------------------------------------------------//
// SOUND
//-------------------------------------------------------------------------//

void initSoundEngine(void);
ISound *loadSound();

//-------------------------------------------------------------------------//
// IMAGE
//-------------------------------------------------------------------------//

class RGBAImage
{
public:
	string name;
	vector<unsigned char> pixels;
	unsigned int width, height;
	GLuint textureId;
	GLuint samplerId;

	RGBAImage(void) { width = 0; height = 0; textureId = NULL_HANDLE; samplerId = NULL_HANDLE; }
	~RGBAImage();
	bool loadPNG(const string &fileName, bool doFlipY = true);
	bool writeToPNG(const string &fileName);
	void flipY(void);
	void sendToOpenGL(GLuint magFilter, GLuint minFilter, bool createMipMap);

	unsigned int &operator()(int x, int y) {
		return pixel(x, y);
	}
	unsigned int &pixel(int x, int y) {
		unsigned int *A = (unsigned int*)&pixels[0];
		return A[y*width + x];
	}
	void sendToOpenGL(void) {
		sendToOpenGL(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
	}
};

//-------------------------------------------------------------------------//
// TRANSFORM
//-------------------------------------------------------------------------//
class Camera
{
public:
	// look from, look at, view up
	glm::vec3 eye, center, vup;
    
    
	float fovy; // vertical field of view
	float znear, zfar; // near and far clip planes
    
	glm::mat4x4 worldViewProject;
    
	void refreshTransform(float screenWidth, float screenHeight) {
		glm::mat4x4 worldView = glm::lookAt(eye, center, vup);
		glm::mat4x4 project = glm::perspective((float)fovy,
                                               (float)(screenWidth / screenHeight), (float)znear, (float)zfar);
		worldViewProject = project * worldView;
	}
    
	void translateGlobal(glm::vec3 t) { eye += t; center += t; }
	void translateLocal(glm::vec3 t) {
		glm::vec3 zz = glm::normalize(eye - center);
		glm::vec3 xx = glm::normalize(glm::cross(vup, zz));
		glm::vec3 yy = glm::cross(zz, xx);
		glm::vec3 tt = t.x*xx + t.y*yy + t.z*zz;
		eye += tt; center += tt;
	}
    
	void rotateGlobal(glm::vec3 axis, float angle) {
		glm::mat4x4 R = glm::axisAngleMatrix(axis, angle);
		glm::vec4 zz = glm::vec4(eye - center, 0);
		glm::vec4 Rzz = R*zz;
		center = eye - glm::vec3(Rzz);
		//
		glm::vec4 up = glm::vec4(vup, 0);
		glm::vec4 Rup = R*up;
		vup = glm::vec3(Rup);
	}
	void rotateLocal(glm::vec3 axis, float angle) {
		glm::vec3 zz = glm::normalize(eye - center);
		glm::vec3 xx = glm::normalize(glm::cross(vup, zz));
		glm::vec3 yy = glm::cross(zz, xx);
		glm::vec3 aa = xx*axis.x + yy*axis.y + zz*axis.z;
		rotateGlobal(aa, angle);
	}
};



class Transform
{
public:
	// make transform class
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
    
	glm::mat4x4 transform;
	glm::mat4x4 invTransform;
    
	void refreshTransform(void)
	{
		glm::mat4x4 Mtrans = glm::translate(translation);
		glm::mat4x4 Mscale = glm::scale(scale);
		glm::mat4x4 Mrot = glm::toMat4(rotation);
		transform = Mtrans * Mrot * Mscale;  // transforms happen right to left
		invTransform = glm::inverse(transform);
	}
    
    void translateGlobal(glm::vec3 moveVec)
    {
        translation += moveVec;
    }
    
    void translateLocal(glm::vec3 moveVec, Camera camera)
    {
        glm::mat4x4 rot = glm::toMat4(rotation);
        glm::vec4 xAxis = glm::vec4(1,0,0,0);
        glm::vec4 yAxis = glm::vec4(0,1,0,0);
        glm::vec4 zAxis = glm::vec4(0,0,1,0);
        
        glm::vec4 locX = rot * xAxis;
        glm::vec4 locY = rot * yAxis;
        glm::vec4 locZ = rot * zAxis;
        
        locX *= moveVec.x;
        locY *= moveVec.y;
        locZ *= moveVec.z;
        
        translation += glm::vec3(locX.x, locX.y, locX.z);
        translation += glm::vec3(locY.x, locY.y, locY.z);
        translation += glm::vec3(locZ.x, locZ.y, locZ.z);
        
        cout << endl;
    }
    
    
    void translateLocal(glm::vec3 moveVec)
    {
        glm::mat4x4 rot = glm::toMat4(rotation);
        glm::vec4 xAxis = glm::vec4(1,0,0,0);
        glm::vec4 yAxis = glm::vec4(0,1,0,0);
        glm::vec4 zAxis = glm::vec4(0,0,1,0);
        
        glm::vec4 locX = rot * xAxis;
        glm::vec4 locY = rot * yAxis;
        glm::vec4 locZ = rot * zAxis;
        
        locX *= moveVec.x;
        locY *= moveVec.y;
        locZ *= moveVec.z;
        
        translation += glm::vec3(locX.x, locX.y, locX.z);
        translation += glm::vec3(locY.x, locY.y, locY.z);
        translation += glm::vec3(locZ.x, locZ.y, locZ.z);
        
        cout << endl;
    }
    
    void rotateGlobal(glm::vec3 axis, float angle)
    {
        glm::mat4x4 R = glm::axisAngleMatrix(axis,angle);
        float f = sin(angle/2);
        glm::quat tempQuat;
        tempQuat.x = axis.x * f;
        tempQuat.y = axis.y * f;
        tempQuat.z = axis.z * f;
        tempQuat.w = cos(angle/2);
        
        rotation = tempQuat * rotation;
        
        refreshTransform();
    }
    
    void rotateLocal(glm::vec3 axis, float angle)
    {
        glm::mat4x4 rot = glm::toMat4(rotation);
        glm::vec4 xAxis = glm::vec4(1,0,0,0);
        glm::vec4 yAxis = glm::vec4(0,1,0,0);
        glm::vec4 zAxis = glm::vec4(0,0,1,0);
        
        glm::vec4 locX = rot * xAxis;
        glm::vec4 locY = rot * yAxis;
        glm::vec4 locZ = rot * zAxis;
        
        locX *= axis.x;
        locY *= axis.y;
        locZ *= axis.z;
        
        glm::vec3 locAxis = glm::vec3(locX.x, locX.y, locX.z);
        locAxis += glm::vec3(locY.x, locY.y, locY.z);
        locAxis += glm::vec3(locZ.x, locZ.y, locZ.z);
        
        
        rotateGlobal(locAxis, angle);
        
    }
};


//-------------------------------------------------------------------------//
//  3D Sound
//-------------------------------------------------------------------------//

class Sound
{
public:
	Transform T;
	ISound *music;
};


//-------------------------------------------------------------------------//
// Camera
//-------------------------------------------------------------------------//



#define KEYBOARD_CONTROL 0
#define MOUSE_CONTROL 1

//-------------------------------------------------------------------------//
// MATERIAL
//-------------------------------------------------------------------------//

class Material
{
public:
	GLuint shaderProgram;
	vector< NameIdVal<glm::vec4> > colors;
	vector< NameIdVal<RGBAImage*> > textures;
	void bindMaterial(Transform &T, Camera &camera);
    void bindNodeMaterial(Node* node, Camera &camera);
};

//-------------------------------------------------------------------------//
// TRIANGLE MESH
//-------------------------------------------------------------------------//

class TriMesh
{
public:
	string name;
	vector<string> attributes;
	vector<float> vertexData;
	vector<int> indices;
	int numIndices;
    
	GLuint vao; // vertex array handle
	GLuint ibo; // index buffer handle
	
	bool readFromPly(const string &fileName, bool flipZ = false);
	bool sendToOpenGL(void);
	void draw(void);
};

class TriMeshInstance
{
public:
	string name;
	TriMesh *triMesh;
	Transform T;
	Material mat;
    ISound* nodeSound;
	
public:
	TriMeshInstance(void);
    
	void setMesh(TriMesh *mesh) { triMesh = mesh; }
	void setScale(const glm::vec3 &s) { T.scale = s; }
	void setRotation(const glm::quat &r) { T.rotation = r; }
	void setTranslation(const glm::vec3 &t) { T.translation = t; }
    
    void setSound(string fileName, ISoundEngine* sEngine) {
        
        vec3df meshPOS = vec3df(T.translation.x, T.translation.y, T.translation.z);
        
        cout << fileName << endl;
        
        this->nodeSound = sEngine->play3D(fileName.c_str(), meshPOS, true);
        
        if(nodeSound )
        {
            this->nodeSound->setMinDistance(-10.0);
            this->nodeSound->setMaxDistance(20.0);
            sEngine->setSoundVolume(0.25f);
        }
    }
    
    

    
	void draw(Camera &camera);
};

//-------------------------------------------------------------------------//
// Billboard
//-------------------------------------------------------------------------//

class Billboard 
{
public:
	string name;
	TriMesh *triMesh;
	Transform T;
	Material mat;
	int type;

public:
	Billboard(void);
	
	void setMesh(TriMesh *mesh) { triMesh = mesh; }
	void setScale(const glm::vec3 &s) { T.scale = s; }
	void fixScale();
	void setRotation(const glm::quat &r) { T.rotation = r; }
	void setTranslation(const glm::vec3 &t) { T.translation = t; }

	void refreshTransform(Camera &camera);

	void draw(Camera &camera);
};

//-------------------------------------------------------------------------//
// Lights
//-------------------------------------------------------------------------//

class Light 
{
public:
	// type is in attenuation.w
	glm::vec4 position, direction, color, attenuation, coneAngles;

	Light(const Light &L)
	{
		position = L.position; 
		direction = L.direction;
		color = L.color;
		attenuation = L.attenuation;
		coneAngles = L.coneAngles;
	}

	Light()
	{
		position = glm::vec4(0, 0, 0, 1);
		direction = glm::vec4(0, 1, 0, 0);
		color = glm::vec4(0, 0, 0, 0);
		attenuation = glm::vec4(0, 0, 0, 0);
		coneAngles = glm::vec4(0.5, 0.55, 0, 0);
	}
};

#define NO_LIGHT 0
#define AMBIENT_LIGHT 1
#define DIRECTIONAL_LIGHT 2
#define POINT_LIGHT 3
#define SPOT_LIGHT 4
#define HEAD_LIGHT 5
#define RIM_LIGHT 6

#define MAX_LIGHTS 10
#define LIGHT_BUFFER_ID 1
extern GLuint gLightBufferObject;
extern int gNumLights;
extern Light gLights[MAX_LIGHTS];

void initLightBuffer(void);

//-------------------------------------------------------------------------//
//  Particle System
//-------------------------------------------------------------------------//
#define MAX_PARTICLES 10000
#define PS_FOUNTAIN 0
#define PS_EXPLOSION 1

class Particle{
public:
	Transform T;
	glm::vec3 Vel, Accel;
	Billboard instance;
	float life;

	Particle(glm::vec3 VelMag, glm::vec3 _Pos, glm::vec3 _Accel, float _life, Billboard inst, int type);

	bool isDead(){
		if (life < 0.0)
			return true;
		else
			return false;
	}
	void update();
	void render(Camera &camera);
};

class partSys{
public:
	vector<Particle> particles;

	Billboard bb;
	glm::vec3 Origin, Vel, Accel, VelMag;
	float duration, life;
	int type;

	partSys(glm::vec3 Pos, glm::vec3 Vel, glm::vec3 Accel, glm::vec3 VelMag, Billboard _bb, float _duration, float _life, int _type);

	void initPS();
	void addParticle();
	void update();
	void render(Camera &camera);

	bool isDead(){
		if (particles[0].life < 0.0)
			return true;
		else
			return false;
	}
};


//-------------------------------------------------------------------------//
//  Scene Graph Node
//-------------------------------------------------------------------------//

class Node
{
public:
	vector<Node*> children;
    Node* parent;
	string name;
	int nodeType;
   

	TriMeshInstance *meshInst;

    Node(){ nodeType = NULL; parent = NULL;}

    Node(TriMeshInstance *_meshInst){ meshInst = new TriMeshInstance(*_meshInst); nodeType = 0; }

	void addChildren(Node *child){ children.push_back(child); }
    void draw(Camera &camera);
	void rotateLocal(glm::vec3 axis, float angle, bool inverse); //rotates just parent 
	void rotateGlobal(glm::vec3 axis, float angle, bool inverse){ //rotates parent and children 
		rotateLocal(axis, angle, !inverse);
		if (children.size() > 0){
			for (int i = 0; i < children.size(); i++){
				children[i]->rotateLocal(axis, angle, inverse);
			}
		}
	}

	void translateLocal(glm::vec3 &t, bool inverse); //transforms just parent
	void translateGlobal(glm::vec3 &t, bool inverse){ //transforms parent and children
		translateLocal(t, inverse);
		if (children.size() > 0){
			for (int i = 0; i < children.size(); i++){
				children[i]->translateLocal(t, inverse);
			}
		}
	}

};


//-------------------------------------------------------------------------//
// Scene
//-------------------------------------------------------------------------//

class Scene 
{
public:
	// Global properties
	glm::vec3 backgroundColor;
	string backgroundMusic;

	// Object pools
	map<string, TriMesh*> meshes;
	map<string, RGBAImage*> textures;
	map<string, Node*> nodes;
	map<string, TriMeshInstance*> meshInstances;
    map<string,MoveScript*> moveScripts;
    map<string, Node*> baseNodes;
    vector<ControlScript*> controlScripts;
    vector<SpawnScript*> spawnScripts;
    Node* player;
    TriMeshInstance* firstPerson;
    TriMeshInstance* thirdPerson;
    bool isFPCam;

	// Scene graph
	Camera camera;
	int currCam = 0;
	vector<Billboard> bboards;
	vector<partSys> ps;
	vector<Camera> cameras;
    
    //member functions
    void runScripts();
    /*void updateFirstPerson(int width, int height)
    {
        
        
        if(nodes["player"] != NULL)
        {
           if(!isFPCam)
           {
                camera.eye = nodes["player"]->meshInst->T.translation + (nodes["player"]->meshInst->T.rotation * glm::vec3(0,0,1)) * 20.0f + 5.0f * glm::vec3(0,1,0);
                camera.center = nodes["player"]->meshInst->T.translation + 3.0f * glm::vec3(0,1,0);
                camera.refreshTransform(width, height);
            }
            else
           {
                
                camera.eye = player->meshInst->T.translation + ( player->meshInst->T.rotation * glm::vec3(0,0,1)) * 1.0f + -0.1f * glm::vec3(0,1,0);
                camera.center = player->meshInst->T.translation + 0.1f * glm::vec3(0,1,0);
                camera.refreshTransform(width, height);
                
            }
            
        }
        else
        {
            
        }
    }*/
    
    
    void updateListenerPos(ISoundEngine* sEngine)
    {
        if(nodes["player"] != NULL)
        {
            glm::mat4x4 rot = glm::toMat4(nodes["player"]->meshInst->T.rotation);
            glm::vec4 xAxis = glm::vec4(1,0,0,0);
            glm::vec4 yAxis = glm::vec4(0,1,0,0);
            glm::vec4 zAxis = glm::vec4(0,0,1,0);
            
            glm::vec4 locX = rot * xAxis;
            glm::vec4 locY = rot * yAxis;
            glm::vec4 locZ = rot * zAxis;
            
            vec3df listenerView = vec3df(locX.x, locY.y, locZ.z);
            sEngine->setListenerPosition(vec3df(nodes["player"]->meshInst->T.translation.x, nodes["player"]->meshInst->T.translation.y, nodes["player"]->meshInst->T.translation.z), listenerView);
        }
        else
        {
            glm::vec3 zz = glm::normalize(camera.eye - camera.center);
            glm::vec3 xx = glm::normalize(glm::cross(camera.vup, zz));
            glm::vec3 yy = glm::cross(zz, xx);
            
            
            vec3df listenerView = vec3df(xx.x, yy.y, zz.z);
            sEngine->setListenerPosition(vec3df(camera.eye.x, camera.eye.y, camera.eye.z), listenerView);
        }
        
    }
    

	// getters and setters
	void addMesh(TriMesh* mesh) { meshes[mesh->name] = mesh; }
	TriMesh *getMesh(string &n) {
		if (meshes.find(n) != meshes.end()) return meshes[n];
		else return NULL;
	}
	void addTexture(RGBAImage *tex) {textures[tex->name] = tex;}
	RGBAImage *getTexture(string &n) {
		if (textures.find(n) != textures.end()) return textures[n];
		else return NULL;
	}
	void addMeshInstance(TriMeshInstance *instance) { meshInstances[instance->name] = instance; }
	TriMeshInstance *getInstance(string &n){
		if (meshInstances.find(n) != meshInstances.end()) return meshInstances[n];
		else return NULL;
	}
	void addCamera(Camera c){cameras.push_back(c);}
	void addNode(Node* node){ nodes[node->name] = node;
    
        //nodes.insert(nodes.begin(), nodes.find(node->name));
    
    }
	Node *getNode(string &n){
		if (nodes.find(n) != nodes.end()) return nodes[n];
		else{
			printf("cant find parent\n"); return NULL;}
	}
	void addBillboard(Billboard board){ bboards.push_back(board); }

	void addParticleSystem(partSys partsys){ ps.push_back(partsys); }
	void removeParticleSystem(int index){ ps.erase(ps.begin() + index); }

	// multi cam functions
	void switchCamera(int camNum){
		camera = cameras[camNum];
		currCam = camNum;
	}
	void updateCamera(void){
		cameras[currCam] = camera;
	}
	
	// multi light functions
	void updateLights(void)
	{
		// Update global lights
		glBindBuffer(GL_UNIFORM_BUFFER, gLightBufferObject);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * MAX_LIGHTS, gLights);
		//glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * MAX_LIGHTS, gLights, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0); // unbind buffer
	}

	void render(void) {
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		updateLights();

        renderNodes();
        renderBBoards();
		renderPartSys();
	}
    
    void renderNodes(void)
    {
        string nameSub;
        for (auto& x : nodes){
            
            if(x.second->parent == NULL)
            {
                //cout << "Node Name" << x.second->name << endl;
                x.second->draw(camera);
                renderNodes(x.second->children);
            }
        }
    }
    
    void renderNodes(vector<Node*> nodes)
    {
        for (int i = 0; i < nodes.size(); i++){
            
            nodes[i]->draw(camera);
            
            if(nodes[i] != NULL)
            {
                renderNodes(nodes[i]->children);
            }
        }
    }
    
    void renderBBoards()
    {
        for(int i = 0; i < bboards.size(); i++)
        {
            bboards[i].draw(camera);
        }
    }

	void renderPartSys()
	{
		for (int i = 0; i < ps.size(); i++){
			ps.at(i).update();
			ps.at(i).render(camera);
			if (ps.at(i).isDead()){
				removeParticleSystem(i);
			}
		}
	}
    
};


//control script class

class ControlScript //: public Script
{
public:
    //data members
    Scene* scene;
    GLFWwindow* gWindow;
    float width;
    float height;
    
    ControlScript()
    {
        keyboard = false;
        thirdPerson = false;
        firstPerson = false;
    }
    
    //boolean member variables to determine which scripts to run
    bool keyboard;
    bool thirdPerson;
    bool firstPerson;
    
    //setters for bools
    void useKeyboard(bool option);
    void useThirdPerson(bool option);
    void useFirstPerson(bool option);
    
    void setValue(string property, void* value);
    void* getValue(string property);       //returns a void ptr
    
    //scripts
    void keyboardControls();
    void firstPersonControls();
    void thirdPersonControls();
    
    //function to actually run scripts
    void runScripts();
    
};


class MoveScript
{
public:
    

    MoveScript()
    {
   
        useFaceTarget = false;
        useFollowPlayer = false;
        useGlobalRotate = false;
        useGlobalTrans = false;
        useLocalRotate = false;
        useLocalTrans = false;
        useSetScale = false;
        useBulletTrans = false;
        distCounter = 0;
    }
    
    Node* node;
    Node* targetNode;
    Scene* scene;
    string name;
    glm::vec3 transVec;
    glm::vec3 scaleVec;
    glm::vec3 axis;
    glm::vec3 targetTrans;

    
    //max translate
    float maxTransX;
    float maxTransY;
    float maxTransZ;
    
    //min translate
    float minTransX;
    float minTransY;
    float minTransZ;
    float angle;
    
    //follow floats for followPlayer script
    float followSpeed;
    float followDist;
    float maxDist;
    int distCounter;
    
    
    //utils
    Camera* camera;
    
    void setValue(string property, void* value);
    void setValue(string property, void* value1, void* value2);
    void setValue(string property1, string property2, void* value1);
    void* getValue(string property);
    
    //scripts
    void globalRotate();
    void localRotate();
    void localTrans();
    void globalTrans();
    void setScale();
    void localTransLimited();  //moves an object to a maximum position then moves back to original position
    void followPlayer();
    void faceTarget();
    void bulletTranslation();
    
    //bools for what script to run
    bool useGlobalRotate;
    bool useLocalRotate;
    bool useLocalTrans;
    bool useGlobalTrans;
    bool useSetScale;
    bool useLimitedTrans;
    bool useFollowPlayer;
    bool useFaceTarget;
    bool useBulletTrans;
    bool minSet;
    
    
    //function to runScripts
    void runScripts();
};

class SpawnScript
{
    
public:
    
    bool useSpawn;
    bool useMove;
    bool didSpawnBullet;
    
    Node* node;
    Node* copyNode;
    Node* targetNode;
    
    MoveScript* moveScript;
    
    Scene* scene;
    
    static int spawnNum;
    int numOfSpawn;
    
    string name;
    string nameOfMoveScript;
    
    Camera* camera;
    
    glm::vec3 spawnloc;
    
    
    SpawnScript()
    {
        useSpawn = false;
        useMove = false;
        didSpawnBullet = false;
        name = "spawn";
        numOfSpawn = spawnNum++;
    }
    
    
    void spawnNode();
    void attachMoveScript();
    void spawnBullet();
    void runScripts();
    
};


//




