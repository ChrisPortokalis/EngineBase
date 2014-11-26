
//-------------------------------------------------------------------------//
// Transforms - Tests using transforms for objects
// David Cline
// June 27, 2014
//-------------------------------------------------------------------------//


#include "EngineUtil.h"

//-------------------------------------------------------------------------//
// Global State.  Eventually, this should include the global 
// state of the system, including multiple scenes, objects, shaders, 
// cameras, and all other resources needed by the system.
//-------------------------------------------------------------------------//

GLFWwindow* gWindow = NULL;
string gWindowTitle = "OpenGL App";
int gWidth = 600; // window width
int gHeight = 600; // window height
int gSPP = 16; // samples per pixel
int cameraControl = 0;

Scene gScene;

ISoundEngine* engine = NULL;
ISound* music = NULL;
ControlScript* control = new ControlScript();
MoveScript* moveScript = new MoveScript();
MoveScript* moveFollow = new MoveScript();

void setupScript();

//-------------------------------------------------------------------------//
// Callback for Keyboard Input
//-------------------------------------------------------------------------//

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (action == GLFW_PRESS &&
		((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))) {
		printf("\n%c\n", (char)key);
	}
}

//-------------------------------------------------------------------------//
// Callback for MouseWheel Input
//-------------------------------------------------------------------------//

/*void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	if (yoffset == 1.0){
		//gScene.camera.translateLocal(glm::vec3(0.0f, 0.0f, -0.4f));
	}
	else if (yoffset == -1.0){
		gScene.camera.translateLocal(glm::vec3(0.0f, 0.0f, 0.4f));
	}*/

//}
//-------------------------------------------------------------------------//
// Parse Scene File
//-------------------------------------------------------------------------//

string ONE_TOKENS = "{}[]()<>+-*/,;";


void loadWorldSettings(FILE *F)
{
	string token, t;
	while (getToken(F, token, ONE_TOKENS)) {
		//cout << "  " << token << endl;
		if (token == "}") break;
		if (token == "windowTitle") getToken(F, gWindowTitle, ONE_TOKENS);
		else if (token == "width") getInts(F, &gWidth, 1);
		else if (token == "height") getInts(F, &gHeight, 1);
		else if (token == "spp") getInts(F, &gSPP, 1);
		else if (token == "controller") getInts(F, &cameraControl, 1);
	}

	// Initialize the window with OpenGL context
	gWindow = createOpenGLWindow(gWidth, gHeight, gWindowTitle.c_str(), gSPP);
	glfwSetKeyCallback(gWindow, keyCallback);
	if (cameraControl == MOUSE_CONTROL){
		//glfwSetScrollCallback(gWindow, scroll_callback);
	}
}

void loadSceneSettings(FILE *F, Scene *scene)
{
	string token, t;

	while (getToken(F, token, ONE_TOKENS)){
		if (token == "}"){
			break;
		}
		else if (token == "backgroundColor"){
			getFloats(F, &scene->backgroundColor[0], 3);
		}
		else if (token == "backgroundMusic") {
			string fileName, fullFileName;
			getToken(F, fileName, ONE_TOKENS);
			getFullFileName(fileName, fullFileName);
			cout << "file with path is ";
			cout << fullFileName;
			cout << "\n";
			music = engine->play3D(fullFileName.c_str(), vec3df(0,0,0), true, false, true);
			if (music)
				music->setMinDistance(5.0f);
		}
	}
}

void loadMesh(FILE *F, Scene *scene)
{
	string token;
	while (getToken(F, token, ONE_TOKENS)) {
		if (token == "}") {
			break;
		}
		else if (token == "file") {
			string fileName = "";
			getToken(F, fileName, ONE_TOKENS);
			TriMesh *mesh = new TriMesh();
			mesh->readFromPly(fileName, false);
			mesh->sendToOpenGL();
			scene->addMesh(mesh);
		}
	}
}

void loadMeshInstance(FILE *F, Scene *scene)
{
	string token;
	GLuint vertexShader = NULL_HANDLE;
	GLuint fragmentShader = NULL_HANDLE;
	GLuint shaderProgram = NULL_HANDLE;
	TriMeshInstance *meshInstance = new TriMeshInstance();
	//scene->addMeshInstance(meshInstance);

	while (getToken(F, token, ONE_TOKENS)) {
		if (token == "}") {
			break;
		}
		else if (token == "vertexShader") {
			string vsFileName;
			getToken(F, vsFileName, ONE_TOKENS);
			vertexShader = loadShader(vsFileName.c_str(), GL_VERTEX_SHADER);
		}
		else if (token == "fragmentShader") {
			string fsFileName;
			getToken(F, fsFileName, ONE_TOKENS);
			fragmentShader = loadShader(fsFileName.c_str(), GL_FRAGMENT_SHADER);
		}
		else if (token == "texture") {
			string texAttributeName;
			getToken(F, texAttributeName, ONE_TOKENS);
			string texFileName;
			getToken(F, texFileName, ONE_TOKENS);
			RGBAImage *image = scene->getTexture(texFileName);
			if (image == NULL){
				image = new RGBAImage();
				image->loadPNG(texFileName);
				image->sendToOpenGL();
				scene->addTexture(image);
			}
			NameIdVal<RGBAImage*> texref(texAttributeName, -1, image);
			meshInstance->mat.textures.push_back(texref);
		}
		else if (token == "mesh") {
			string meshName;
			getToken(F, meshName, ONE_TOKENS);
			TriMesh *mesh = scene->getMesh(meshName);
			if (mesh == NULL){
				mesh = new TriMesh();
				mesh->readFromPly(meshName);
				mesh->sendToOpenGL();
				scene->addMesh(mesh);
			}
			meshInstance->setMesh(mesh);
		}
		else if (token == "translate"){
			glm::vec3 t;
			getFloats(F, &t[0], 3);
			meshInstance->T.translation += t;
		}
		else if (token == "scale"){
			glm::vec3 s;
			getFloats(F, &s[0], 3);
			meshInstance->T.scale *= s;
		}
		else if (token == "name"){
			string name;
			getToken(F, name, ONE_TOKENS);
			meshInstance->name = name;
		}
        else if (token == "rotate"){
            glm::vec3 rotVec;
            getFloats(F, &rotVec[0], 3);
            meshInstance->setRotation(glm::quat(rotVec));
        }
        else if(token == "sound")
        {
            string soundFile;
            getToken(F, soundFile, ONE_TOKENS);
            meshInstance->setSound(soundFile, engine);
        }
	}

        shaderProgram = createShaderProgram(vertexShader, fragmentShader);
        meshInstance->mat.shaderProgram = shaderProgram;
    
 
        scene->addMeshInstance(meshInstance);
            

}

void loadCamera(FILE *F, Scene *scene)
{
	string token;
	//Camera *camera = &scene->camera;
	Camera camera;

	while (getToken(F, token, ONE_TOKENS)) {
		if (token == "}") break;
		else if (token == "eye") getFloats(F, &(camera.eye[0]), 3);
		else if (token == "center") getFloats(F, &(camera.center[0]), 3);
		else if (token == "vup") getFloats(F, &(camera.vup[0]), 3);
		else if (token == "znear") getFloats(F, &(camera.znear), 1);
		else if (token == "zfar") getFloats(F, &(camera.zfar), 1);
		else if (token == "fovy") getFloats(F, &(camera.fovy), 1);
	}
	scene->addCamera(camera);

	camera.refreshTransform((float)gWidth, (float)gHeight);
}

void loadLight(FILE *F, Scene *scene)
{
	string token;
	Light light;

	//position direction color attenuation coneAngles type

	while (getToken(F, token, ONE_TOKENS)) {
		if (token == "}") break;
		else if (token == "position") getFloats(F, &(light.position[0]), 3);
		else if (token == "color") getFloats(F, &(light.color[0]), 3);
		else if (token == "attenuation") getFloats(F, &(light.attenuation[0]), 3);
		else if (token == "coneAngles") {
			getFloats(F, &(light.coneAngles[0]), 2);
			light.coneAngles[0] = cos(light.coneAngles[0]);
			light.coneAngles[1] = cos(light.coneAngles[1]);
		}
		else if (token == "direction") {
			getFloats(F, &(light.direction[0]), 3);
			light.direction = glm::normalize(light.direction);
		}
		else if (token == "type") {
			string t;
			getToken(F, t, ONE_TOKENS);
			if (t == "point") light.attenuation.w = POINT_LIGHT;
			else if (t == "directional") light.attenuation.w = DIRECTIONAL_LIGHT;
			else if (t == "spot") light.attenuation.w = SPOT_LIGHT;
			else if (t == "ambient") light.attenuation.w = AMBIENT_LIGHT;
		}
	}

	// Add light to global light list
	gLights[gNumLights] = light;
	gNumLights++;
	if (gNumLights >= MAX_LIGHTS)
	{
		ERROR("Too many lights!", false);
		gNumLights--;
	}
}

void loadNode(FILE *F, Scene *scene){
	string token;
	Node *node = new Node();


	while (getToken(F, token, ONE_TOKENS)) {
		if (token == "}") break;
		else if (token == "meshInstance"){

			string name;
			getToken(F, name, ONE_TOKENS);
			//node = new Node(scene->getInstance(name));
            node->meshInst = gScene.meshInstances[name];
		}
		else if (token == "name"){
			string name;
			getToken(F, name, ONE_TOKENS);
			node->name = name;
            cout << "Node Name: " << name << endl;
		}
		else if (token == "parent"){
			string parent;
			getToken(F, parent, ONE_TOKENS);
            
            cout << "Parent Name: " << parent << endl;
			if (gScene.nodes.find(parent)->second == NULL && gScene.player != NULL){
				printf("Error: Can't find parent\n");
			}
			else{
				gScene.nodes[parent]->addChildren(node);
                node->parent = gScene.nodes[parent];
			}
		}
	}
    

    scene->addNode(node);


}



void loadBillboard(FILE *F, Scene *scene){
	string token;
	GLuint vertexShader = NULL_HANDLE;
	GLuint fragmentShader = NULL_HANDLE;
	GLuint shaderProgram = NULL_HANDLE;
	Billboard* board = new Billboard();

	while (getToken(F, token, ONE_TOKENS)){
		if (token == "}") {
			break;
		}

		else if (token == "vertexShader") {
			string vsFileName;
			getToken(F, vsFileName, ONE_TOKENS);
			vertexShader = loadShader(vsFileName.c_str(), GL_VERTEX_SHADER);
		}
		else if (token == "fragmentShader") {
			string fsFileName;
			getToken(F, fsFileName, ONE_TOKENS);
			fragmentShader = loadShader(fsFileName.c_str(), GL_FRAGMENT_SHADER);
		}

		else if (token == "texture") {
			string texAttributeName;
			getToken(F, texAttributeName, ONE_TOKENS);
			string texFileName;
			getToken(F, texFileName, ONE_TOKENS);
			RGBAImage *image = scene->getTexture(texFileName);
			if (image == NULL){
				image = new RGBAImage();
				image->loadPNG(texFileName);
				image->sendToOpenGL();
				scene->addTexture(image);
			}
			NameIdVal<RGBAImage*> texref(texAttributeName, -1, image);
			board->mat.textures.push_back(texref);
		}

		else if (token == "mesh") {
			string meshName;
			getToken(F, meshName, ONE_TOKENS);
			TriMesh *mesh = scene->getMesh(meshName);
			if (mesh == NULL){
				mesh = new TriMesh();
				mesh->readFromPly(meshName);
				mesh->sendToOpenGL();
				scene->addMesh(mesh);
			}
			board->setMesh(mesh);
		}
		else if (token == "type"){
			int t;
			getInts(F, &t, 1);
			board->type = t;
		}

		else if (token == "translate"){
			glm::vec3 t;
			getFloats(F, &t[0], 3);
			board->T.translation += t;
		}

		else if (token == "name"){
			string name;
			getToken(F, name, ONE_TOKENS);
			board->name = name;
		}
	}
	shaderProgram = createShaderProgram(vertexShader, fragmentShader);
	board->mat.shaderProgram = shaderProgram;
	scene->addBillboard(board);
}

void loadScene(const char *sceneFile, Scene *scene)
{
	string sceneFileName = sceneFile;
	int separatorIndex = sceneFileName.find_last_of("/");
	if (separatorIndex < 0) separatorIndex = sceneFileName.find_last_of("\\");
	if (separatorIndex > 0) addToPath(sceneFileName.substr(0, separatorIndex + 1));
	//printf("PATH:\n");
	//printPath();

	FILE *F = openFileForReading(sceneFile);
	string token;

	while (getToken(F, token, ONE_TOKENS)) {
		//cout << token << endl;
		if (token == "worldSettings") {
			loadWorldSettings(F);
			initLightBuffer();
		}
		else if (token == "sceneSettings") {
			loadSceneSettings(F, scene);
		}
		else if (token == "mesh") {
			loadMesh(F, scene);
		}
		else if (token == "meshInstance") {
			loadMeshInstance(F, scene);
		}
		else if (token == "camera") {
			loadCamera(F, scene);
		}
		else if (token == "light") {
			loadLight(F, scene);
		}
		else if (token == "node"){
			loadNode(F, scene);
		}
		else if (token == "billboard"){
			loadBillboard(F, scene);
		}
	}
}

//-------------------------------------------------------------------------//
// Update
//-------------------------------------------------------------------------//
int hhhhhh = 0;
bool poop = true;

void update(void)
{	
	glm::vec3 cameraPos = gScene.camera.eye;
	glm::vec3 cameraRot = gScene.camera.center;
    gScene.updateFirstPerson( gWidth, gHeight);
    gScene.updateListenerPos(engine);
    

    control->runScripts();
    moveScript->runScripts();
    moveFollow->runScripts();
    gScene.camera.refreshTransform(gWidth, gHeight);
	//engine->setListenerPosition(vec3df(cameraPos.x, cameraPos.y, cameraPos.z), vec3df(cameraRot.x, cameraRot.y, cameraRot.z) );

	//gScene.nodes["parent"]->rotateLocal(glm::vec3(0, 1, 0), 0.03, false);
	//gScene.nodes["child"]->rotateGlobal(glm::vec3(0, 1, 0), 0.15, true);
   // gScene.updateListenerPos(engine);
}

//-------------------------------------------------------------------------//
// Draw a frame
//-------------------------------------------------------------------------//

void render(void)
{
	gScene.render();
}

//-------------------------------------------------------------------------//
// Control the camera
//-------------------------------------------------------------------------//

void cameraController(Camera &camera, int type){
   /*bool playerPresent = false;
	if (glfwGetKey(gWindow, GLFW_KEY_1)){
		gScene.updateCamera();
		gScene.switchCamera(0);
	}
	if (glfwGetKey(gWindow, GLFW_KEY_2)){
		gScene.updateCamera();
		gScene.switchCamera(1);
	}
	if (glfwGetKey(gWindow, GLFW_KEY_3)){
		gScene.updateCamera();
		gScene.switchCamera(2);
	}
    
    if(gScene.nodes["player"] != NULL)
    {
        playerPresent = true;
    }

	if (type == KEYBOARD_CONTROL){
       const float t = 0.025f;
       const float r = 0.01f;
        const float conT = 1.0f;
        
        if (glfwGetKey(gWindow, 'W'))
        {
            
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.translateLocal(glm::vec3(0, 0, -1), gScene.camera);
            }
            else
            {
                const glm::vec3 moveVec = glm::vec3(0,0,-1);
                camera.translateLocal(glm::vec3(0,0,-1));
            }
            
        }
        
        if (glfwGetKey(gWindow, 'S'))
        {
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.translateLocal(glm::vec3(0, 0, 1), gScene.camera);
            }
            else
            {
               camera.translateLocal(glm::vec3(0,0,1));
            }
            
        }
        
        if (glfwGetKey(gWindow, 'D'))
        {
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.translateLocal(glm::vec3(.2, 0, 0), gScene.camera);
            }
            else
            {
                camera.translateLocal(glm::vec3(.2,0,0));
            }
            
        }
        
        if (glfwGetKey(gWindow, 'A'))
        {
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.translateLocal(glm::vec3(-.2, 0, 0), camera);
            }
            else
            {
                camera.translateLocal(glm::vec3(-.2,0,0));
            }
            
        }
        if (glfwGetKey(gWindow, GLFW_KEY_DOWN))
        {
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.rotateLocal(glm::vec3(1,0,0), -.01);
            }
            else
            {
                camera.rotateLocal(glm::vec3(1, 0, 0) , -.01);
            }
        }
        if (glfwGetKey(gWindow, GLFW_KEY_UP))
        {
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.rotateLocal(glm::vec3(1,0,0), .01);
            }
            else
            {
                camera.rotateLocal(glm::vec3(1, 0, 0), .01);
            }
        }
        
        if (glfwGetKey(gWindow, GLFW_KEY_LEFT))
        {
            
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.rotateGlobal(glm::vec3(0 ,1, 0), r);
            }
            else
            {
                camera.rotateGlobal(glm::vec3(0,1,0), r);
            }
        }
        if (glfwGetKey(gWindow, GLFW_KEY_RIGHT))
        {
            
            
            if(playerPresent)
            {
                gScene.nodes["player"]->meshInst->T.rotateGlobal(glm::vec3(0,1,0), -r);
            }
            else
            {
                camera.rotateGlobal(glm::vec3(0,1,0), -r);
            }
        }
        if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
        {
            //gScene.nextCamera(gWidth, gHeight);
        }
        
        gScene.camera.refreshTransform((float)gWidth, (float)gHeight);
	}

	else if (type == MOUSE_CONTROL){

	}
	camera.refreshTransform(gWidth, gHeight);*/
}

//-------------------------------------------------------------------------//
// Main method
//-------------------------------------------------------------------------//

int main(int numArgs, char **args)
{
	// check usage
	if (numArgs < 2) {
		cout << "Usage: Transforms sceneFile.scene" << endl;
		exit(0);
	}

    engine = createIrrKlangDevice(); // start default sound engine
	if (!engine) 
		return 0; // start up error

	loadScene(args[1], &gScene);

	// start time (used to time framerate)
	double startTime = TIME();
    
	// init current camera
	gScene.switchCamera(0);
    
    setupScript();

	/*for (auto& x : gScene.nodes){
		cout << "nodes created: ";
		cout << x.second->name;
		cout << "\n";
	}*/

	// render loop
	while (true) {
		// update and render
		update();
		render();
		glfwGetWindowSize(gWindow, &gWidth, &gHeight);
        
		// handle input
		glfwPollEvents();
		if (glfwWindowShouldClose(gWindow) != 0) break;
		cameraController(gScene.camera, cameraControl);

		double xx, yy;
		glfwGetCursorPos(gWindow, &xx, &yy);
        
		// print framerate
		double endTime = TIME();
		startTime = endTime;
        
		// swap buffers
		//SLEEP(1); // sleep 1 millisecond to avoid busy waiting
		glfwSwapBuffers(gWindow);
	}

	// Shut down sound engine
	if (music) music->drop(); // release music stream.
	engine->drop(); // delete engine
    
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

//-------------------------------------------------------------------------//


void setupScript()
{
    
    control->setValue("width", &gWidth);
    control->setValue("height",&gHeight);
    control->setValue("window", gWindow);
    control->setValue("scene", &gScene);
    control->useThirdPerson(true);
    control->useFirstPerson(false);
    control->useKeyboard(true);
    
    if(gScene.nodes["rotObj"] != NULL)
    {
        float angle = 0.01;
        float max = 30;
        glm::vec3 axis = glm::vec3(0,1,0);
        glm::vec3 scale = glm::vec3(5,5,5);
        glm::vec3 translate = glm::vec3(0,0,-0.1);
        moveScript->setValue("node", gScene.nodes["rotObj"]);
        moveScript->setValue("rotate", &axis, &angle);
        //moveScript->setValue("scale", &scale);
        moveScript->setValue("maxTrans", "z", &max);
        moveScript->setValue("translate", &translate);
       // moveScript->setValue("camera", &gScene.camera);
        moveScript->useGlobalRotate = true;
        //moveScript->useSetScale = true;
        moveScript->useLimitedTrans = true;
        //moveScript->useLocalTrans=true;

    }
    
    if(gScene.nodes["follow"] != NULL)
    {

        float follow = 0.8;
        float follows = 0.1;
        
        moveFollow->setValue("node", gScene.nodes["follow"]);
        moveFollow->setValue("player", gScene.nodes["player"]);
        moveFollow->setValue("followDist", &follow);
        moveFollow->setValue("followSpeed", &follows);
        moveFollow->useFollowPlayer = true;
        moveFollow->useFacePlayer = true;
        
        
    }
}







