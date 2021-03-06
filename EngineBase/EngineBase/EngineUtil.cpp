
//-------------------------------------------------------------------------//
// MyUtil.h
// Some utility stuff - file reading and such
//
// David Cline
// 6/14/2014
//-------------------------------------------------------------------------//

// Local includes
#include "EngineUtil.h"

//-------------------------------------------------------------------------//
// MISCELLANEOUS
//-------------------------------------------------------------------------//

void ERROR(const string &msg, bool doExit)
{
	cerr << "\nERROR! " << msg << endl;
	if (doExit) exit(0);
}

double TIME(void)
{
	return (double)clock() / (double)CLOCKS_PER_SEC;
}

void SLEEP(int millis)
{
	this_thread::sleep_for(chrono::milliseconds(millis));
}

//-------------------------------------------------------------------------//
// OPENGL STUFF
//-------------------------------------------------------------------------//

GLFWwindow* createOpenGLWindow(int width, int height, const char *title, int samplesPerPixel)
{
	// Initialise GLFW
	if (!glfwInit()) ERROR("Failed to initialize GLFW.", true);
	glfwWindowHint(GLFW_SAMPLES, samplesPerPixel);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
	// Open a window and create its OpenGL context
	GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		ERROR("Failed to open GLFW window.", true);
	}
	glfwMakeContextCurrent(window);
    
	// Ensure we can capture the escape key being pressed
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		glfwTerminate();
		ERROR("Failed to initialize GLEW.", true);
	}
    
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
    
	// Print the OpenGL version we are working with
	char *GL_version = (char *)glGetString(GL_VERSION);
	printf("OpenGL Version: %s\n", GL_version);
	return window;
}

//-------------------------------------------------------------------------//

GLuint loadShader(const string &fileName, GLuint shaderType)
{
	// load the shader as a file
	string mainCode;
	if (!loadFileAsString(fileName, mainCode)) {
		ERROR("Could not load file '" + fileName + "'", false);
		return NULL_HANDLE;
	}

	string shaderCode;
	string alreadyIncluded = fileName;
	replaceIncludes(mainCode, shaderCode, "#include", alreadyIncluded, true);

    
	// print the shader code
	#ifdef _DEBUG
	//cout << "\n----------------------------------------------- SHADER CODE:\n";
	//cout << shaderCode << endl;
	//cout << "--------------------------------------------------------------\n";
	#endif
    
	// transfer shader code to card and compile
	GLuint shaderHandle = glCreateShader(shaderType); // create handle for the shader
	const char* source = shaderCode.c_str();          // get C style string for shader code
	glShaderSource(shaderHandle, 1, &source, NULL);   // pass the shader code to the card
	glCompileShader(shaderHandle);                    // attempt to compile the shader
	
	// check to see if compilation worked
	// If the compilation did not work, print an error message and return NULL handle
	int status;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		ERROR("compiling shader '" + fileName + "'", false);				
		GLint msgLength = 0;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &msgLength);
		std::vector<char> msg(msgLength);
		glGetShaderInfoLog(shaderHandle, msgLength, &msgLength, &msg[0]);
		printf("%s\n", &msg[0]);
		glDeleteShader(shaderHandle); 
		return NULL_HANDLE;
	}
    
	return shaderHandle;
}

//-------------------------------------------------------------------------//

GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// Create and link the shader program
	GLuint shaderProgram = glCreateProgram(); // create handle
	if (!shaderProgram) {
		ERROR("could not create the shader program", false);
		return NULL_HANDLE;
	}
	glAttachShader(shaderProgram, vertexShader);    // attach vertex shader
	glAttachShader(shaderProgram, fragmentShader);  // attach fragment shader
	glLinkProgram(shaderProgram);
    
	// check to see if the linking was successful
	int linked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked); // get link status
	if (!linked) {
		ERROR("could not link the shader program", false);
		GLint msgLength = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &msgLength);
		std::vector<char> msg(msgLength);
		glGetShaderInfoLog(shaderProgram, msgLength, &msgLength, &msg[0]);
		printf("%s\n", &msg[0]);
		glDeleteProgram(shaderProgram);
		return NULL_HANDLE;
	}

	// HOOK UP UNIFORM BUFFER AND UNIFORM BLOCK TO SAME BINDING POINT
	GLint lightBlockIndex = glGetUniformBlockIndex(shaderProgram, "Lights");
	glUniformBlockBinding(shaderProgram, lightBlockIndex, LIGHT_BUFFER_ID);
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_BUFFER_ID, gLightBufferObject);

	//printf("LIGHT STUFF %d %d %d\n", shaderProgram, lightBlockIndex, lightBufferObject);
    
	return shaderProgram;
}

//-------------------------------------------------------------------------//
// GLM UTILITY STUFF
//-------------------------------------------------------------------------//

void printMat(const glm::mat4x4 &m)
{
	printf("\n");
	for (int r = 0; r < 4; r++) {
		printf("[ ");
		for (int c = 0; c < 4; c++) {
			printf("%7.3f ", m[c][r]); // glm uses column major order
		}
		printf("]\n");
	}
}

//-------------------------------------------------------------------------//
// FILE READING
//-------------------------------------------------------------------------//

vector<string> PATH;

void addToPath(const string &p)
{
	PATH.push_back(p);
}

void removeFromPath(const string &p)
{
	for (int i = (int)p.length() - 1; i >= 0; i--) {
		if (PATH[i] == p) PATH.erase(PATH.begin() + i);
	}
}

void printPath(void)
{
	for (int i = 0; i < (int)PATH.size(); i++) {
		cout << PATH[i] << endl;
	}
}

bool getFullFileName(const string &fileName, string &fullName)
{
	for (int i = -1; i < (int)PATH.size(); i++) {
		if (i < 0) fullName = fileName;
		else fullName = PATH[i] + fileName;

		FILE *f = fopen(fullName.c_str(), "rb");
		if (f != NULL) {
			fclose(f);
			return true;
		}
	}
	fullName = "";
	return false;
}

FILE *openFileForReading(const string &fileName)
{
	string fullName;
	bool fileExists = getFullFileName(fileName, fullName);
	if (fileExists) {
		cout << "Opening file '" << fileName << "'" << endl;
		return fopen(fullName.c_str(), "rb");
	}

	string msg = "Could not open file " + fileName;
	ERROR(msg.c_str(), false);
	return NULL;
}

//-------------------------------------------------------------------------//

bool getToken(FILE *f, string &token, const string &oneCharTokens)
{
	token = "";
	while (!feof(f)) {
		int c = getc(f);
		int tokenLength = (int)token.length();
		int cIsSpace = isspace(c);
		bool cIsOneCharToken = ((int)oneCharTokens.find((char)c) >= 0);

		if (cIsSpace && tokenLength == 0) { // spaces before token, ignore
			continue;
		}
		else if (cIsSpace) { // space after token, done
			break;
		}
		else if (c == EOF) { // end of file, done
			break;
		}
		else if ((tokenLength == 0) && cIsOneCharToken) { // oneCharToken, done
			token += (char)c;
			break;
		}
		else if (cIsOneCharToken) { // oneCharToken after another token, push back
			ungetc(c, f);
			break;
		}
		else if ((tokenLength == 0) && (c == '\"' || c == '\'')) { // quoted string, append til end quote found
			char endQuote = c;
			while (!feof(f)) {
				int d = getc(f);
				if (d == endQuote) return true;
				token += (char)d;
			}
			break;
		}
		else if (c == '\"' || c == '\'') { // quote after token started, push back
			ungetc(c, f);
			break;
		}
		else {
			token += (char)c;
		}
	}
	//cout << token << endl;
	return (token.length() > 0);
}

//-------------------------------------------------------------------------//

int getFloats(FILE *f, float *a, int num)
{
	string token;
	int count = 0;
	while (getToken(f, token, "[],")) {
		if (token == "]") {
			break;
		}
		else if (isdigit(token[0]) || token[0] == '-') {
			sscanf(token.c_str(), "%f", &a[count]);
			count++;
			if (count == num) break;
		}
	}
	return count;
}

int getInts(FILE *f, int *a, int num)
{
	string token;
	int count = 0;
	while (getToken(f, token, "[],")) {
		if (token == "]") {
			break;
		}
		else if (isdigit(token[0]) || token[0] == '-') {
			sscanf(token.c_str(), "%d", &a[count]);
			count++;
			if (count == num) break;
		}
	}
	return count;
}
//-------------------------------------------------------------------------//

bool loadFileAsString(const string &fileName, string &fileContents)
{
	printf("loading file '%s'\n", fileName.c_str());

	string fullName;
	bool fileExists = getFullFileName(fileName, fullName);
	if (!fileExists) return false;

	ifstream fileStream(fullName.c_str());
	if (fileStream.good()) {
		stringstream stringStream;
		stringStream << fileStream.rdbuf();
		fileContents = stringStream.str();
		fileStream.close();
		return true;
	}

	fileStream.close();
	return false;
}

//-------------------------------------------------------------------------//

void replaceIncludes(string &src, string &dest, const string &directive,
	string &alreadyIncluded, bool onlyOnce)
{
	int start = 0;

	while (true) {
		int includeIndex = (int)src.find("#include", start);
		if (includeIndex < 0) {
			dest += src.substr(start);
			break;
		}
		if (includeIndex > 0 && !isspace(src[includeIndex - 1])) continue;
		dest += src.substr(start, includeIndex - 1);
		//
		int quoteStart = (int)src.find("\"", start + 8);
		int quoteEnd = (int)src.find("\"", quoteStart + 1);
		start = quoteEnd + 1;
		if (quoteStart >= quoteEnd) {
			ERROR("could not replace includes");
			break;
		}
		string includeFileName = src.substr(quoteStart + 1, (quoteEnd - quoteStart - 1));
		if ((int)alreadyIncluded.find(includeFileName) < 0) {
			if (onlyOnce) {
				alreadyIncluded.append("|");
				alreadyIncluded.append(includeFileName);
			}
			string subSource;
			loadFileAsString(includeFileName, subSource);
			replaceIncludes(subSource, dest, directive, alreadyIncluded, onlyOnce);
		}
	}
}

//-------------------------------------------------------------------------//
// RGBAImage
//-------------------------------------------------------------------------//

RGBAImage::~RGBAImage()
{
	if (textureId != NULL_HANDLE) glDeleteTextures(1, &textureId);
	if (samplerId != NULL_HANDLE) glDeleteSamplers(1, &samplerId);
}

bool RGBAImage::loadPNG(const string &fileName, bool doFlipY)
{
	string fullName;
	getFullFileName(fileName, fullName);
	unsigned error = lodepng::decode(pixels, width, height, fullName.c_str());
	if (error) {
		ERROR(lodepng_error_text(error), false);
		return false;
	}

	if (doFlipY) flipY(); // PNGs go top-to-bottom, OpenGL is bottom-to-top
	name = fileName;
	return true;
}

bool RGBAImage::writeToPNG(const string &fileName)
{
	unsigned error = lodepng::encode(fileName.c_str(), pixels, width, height);
	if (error) {
		ERROR(lodepng_error_text(error), false);
		return false;
	}
	return true;
}

void RGBAImage::flipY(void)
{
	unsigned int *a, *b;
	unsigned int temp;

	for (int y = 0; y < (int)height / 2; y++)
	{
		a = &pixel(0, y);
		b = &pixel(0, height - 1 - y);
		for (int x = 0; x < (int)width; x++) {
			temp = a[x];
			a[x] = b[x];
			b[x] = temp;
		}
	}
}

void RGBAImage::sendToOpenGL(GLuint magFilter, GLuint minFilter, bool createMipMap)
{
	if (width <= 0 || height <= 0) return;

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
	if (createMipMap) glGenerateMipmap(GL_TEXTURE_2D);

	glGenSamplers(1, &samplerId);
	glBindSampler(textureId, samplerId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
}

//-------------------------------------------------------------------------//
// TRIANGLE MESH
//-------------------------------------------------------------------------//

bool TriMesh::readFromPly(const string &fileName, bool flipZ)
{
	name = fileName;
	FILE *f = openFileForReading(fileName);
	if (f == NULL) return false;
	string token, t;
	int numVertices = 0;
	int numFaces = 0;
	int numTriangles = 0;
	vector<int> faceIndices;
    
	// get num vertices
	while (getToken(f, token, "")) {
		//cout << token << endl;
		if (token == "vertex") break;
	}
	getToken(f, token, "");
	numVertices = atoi(token.c_str());
    
	// get vertex attributes
	while (getToken(f, token, "")) {
		if (token == "property") {
			getToken(f, t, ""); // token should be "float"
			getToken(f, t, ""); // attribute name
			attributes.push_back(t);
		}
		else if (token == "face") {
			getToken(f, t, "");
			numFaces = atoi(t.c_str());
			break;
		}
	}
    
	// read to end of header
	while (getToken(f, token, "")) {
		if (token == "end_header") break;
	}
    
	// get vertices
	float val;
	for (int i = 0; i < (int)(numVertices*attributes.size()); i++) {
		//getToken(f, token, "");
		//vertexData.push_back((float)atof(token.c_str()));
		fscanf(f, "%f", &val);
		vertexData.push_back(val);
	}
    
	// divide color values by 255, and flip normal directions if needed
	// This deals with issues related to exporting from Blender to ply
	// usin the y-axis as UP and the z-axis as FRONT.
	for (int i = 0; i < (int)attributes.size(); i++) {
		if (attributes[i] == "red" || attributes[i] == "green" || attributes[i] == "blue") {
			for (int j = 0; j < numVertices; j++) {
				vertexData[i + j*attributes.size()] /= 255.0f;
			}
		}
		else if (flipZ && (attributes[i] == "z" || attributes[i] == "nz")) {
			for (int j = 0; j < numVertices; j++) {
				vertexData[i + j*attributes.size()] *= -1.0f;
			}
		}
	}
    
	// get faces
	int idx;
	for (int i = 0; i < numFaces; i++) {
		faceIndices.clear();
		getToken(f, token, "");
		int numVerts = atoi(token.c_str());
		for (int j = 0; j < numVerts; j++) { // get all vertices in face
			//getToken(f, t, "");
			//faceIndices.push_back(atoi(t.c_str()));
			fscanf(f, "%d", &idx);
			faceIndices.push_back(idx);
		}
		for (int j = 2; j < numVerts; j++) { // make triangle fan
			indices.push_back(faceIndices[0]);
			indices.push_back(faceIndices[j-1]);
			indices.push_back(faceIndices[j]);
			numTriangles++;
		}
	}
	numIndices = (int)indices.size();
    
	//printf("vertices:%d, triangles:%d, attributes:%d\n",
	//	vertexData.size()/attributes.size(),
	//	indices.size()/3,
	//	attributes.size());
    
	fclose(f);
	return true;
}

//-------------------------------------------------------------------------//

void Material::bindMaterial(Transform &T, Camera &camera)
{
	glUseProgram(shaderProgram);

	// MATRICES FROM TRANSFORM
	GLint loc = glGetUniformLocation(shaderProgram, "uObjectWorldM");
	if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(T.transform));
	//
	loc = glGetUniformLocation(shaderProgram, "uObjectWorldInverseM");
	if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(T.invTransform));
	//
	glm::mat4x4 objectWorldViewPerspect = camera.worldViewProject * T.transform;
	loc = glGetUniformLocation(shaderProgram, "uObjectPerpsectM");
	if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(objectWorldViewPerspect));
    
    loc = glGetUniformLocation(shaderProgram, "uView");
	if (loc != -1) glUniform4fv(loc, 1, &camera.eye[0]);

	// MATERIAL COLORS
	for (int i = 0; i < (int) colors.size(); i++) {
		if (colors[i].id == -1) {
			loc = glGetUniformLocation(shaderProgram, colors[i].name.c_str());
			colors[i].id = loc;
		}
		if (colors[i].id >= 0) {
			glUniform4fv(colors[i].id, 1, &colors[i].val[0]);
		}
	}

	// MATERIAL TEXTURES
	for (int i = 0; i < (int) textures.size(); i++) {
		if (textures[i].id == -1) {
			loc = glGetUniformLocation(shaderProgram, textures[i].name.c_str());
			textures[i].id = loc;
		}
		if (textures[i].id >= 0) {
			//printf("\n%d %d\n", textures[i].id, textures[i].val->samplerId);
			glActiveTexture(GL_TEXTURE0 + i);
			glUniform1i(textures[i].id, i);
			glBindTexture(GL_TEXTURE_2D, textures[i].val->textureId);
			glBindSampler(textures[i].id, textures[i].val->samplerId);
		}
	}
}
//-------------------------------------------------------------------------//

#define V_POSITION 0
#define V_NORMAL 1
#define V_ST 2
#define V_COLOR 3
int NUM_COMPONENTS[] = { 3, 3, 2, 3 };

bool TriMesh::sendToOpenGL(void)
{
	// Create vertex array object.  The vertex array object
	// holds the structure of how the vertices are stored. VAOs
	// also are bound for rendering.
	//
	glGenVertexArrays(1, &vao); // generate 1 array
	glBindVertexArray(vao);
    
	// Make and bind the vertex buffer object.  The vbo
	// holds the raw data that will be indexed by the vao.
	//
	GLuint vbo; // vertex buffer object
	glGenBuffers(1, &vbo); // generate 1 buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size()*sizeof(float), &vertexData[0], GL_STATIC_DRAW);
    
	// At this point, we have to tell the vertex array what kind
	// of data it holds, and where it is located in the vertex buffer.
	// The code here uses the first field of four possible data types
	// (position, normal, textureCoordinate, color)
	//
	int stride = (int)attributes.size() * sizeof(float); // size of a vertex in bytes
    
	for (int i = 0; i < (int)attributes.size(); i++) {
		int bindIndex = -1;
		//int numComponents = 0;
        
		if (attributes[i] == "x") bindIndex = V_POSITION;
		else if (attributes[i] == "nx") bindIndex = V_NORMAL;
		else if (attributes[i] == "s") bindIndex = V_ST;
		else if (attributes[i] == "red") bindIndex = V_COLOR;
        
		if (bindIndex >= 0) {
			//printf("bindIndex = %d\n", bindIndex);
			glEnableVertexAttribArray(bindIndex);
			glVertexAttribPointer(bindIndex, NUM_COMPONENTS[bindIndex],
                                  GL_FLOAT, GL_FALSE, stride, (void*)(i * sizeof(float)));
		}
	}
    
	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    
	// Generate the index buffer
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int),
                 &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind
    
	return true;
}

//-------------------------------------------------------------------------//

void TriMesh::draw(void)
{
	glBindVertexArray(vao); // bind the vertices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // bind the indices
    
	// draw the triangles.  modes: GL_TRIANGLES, GL_LINES, GL_POINTS
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)0);
}

//-------------------------------------------------------------------------//

TriMeshInstance::TriMeshInstance(void)
{
	triMesh = NULL;
	
	T.scale = glm::vec3(1, 1, 1);
	T.translation = glm::vec3(0, 0, 0);
}

//-------------------------------------------------------------------------//

void TriMeshInstance::draw(Camera &camera)
{
	T.refreshTransform();
	mat.bindMaterial(T, camera);
	if (triMesh != NULL) triMesh->draw();
	else printf("Error! Null Mesh.");
}
//-------------------------------------------------------------------------//

//-------------------------------------------------------------------------//
Billboard::Billboard(void)
{
	triMesh = NULL;
	
	T.scale = glm::vec3(1, 1, 1);
	T.translation = glm::vec3(0, 0, 0);
	type = 0;
}

void Billboard::refreshTransform(Camera &camera)
{
	glm::vec3 objToCamProj;
	float angleY, angleX;
	if (type == 0){
		objToCamProj = glm::vec3(camera.eye - T.translation);
		angleY = -atan2(objToCamProj.x, objToCamProj.z);
		T.rotation = glm::quat(sin(angleY / 2), glm::vec3(0, cos(angleY / 2), 0));
	}
	else if (type == 1){
		objToCamProj = glm::vec3(camera.eye - T.translation);
		objToCamProj = glm::normalize(objToCamProj);
		angleY = -atan2(objToCamProj.x, objToCamProj.z);
		angleX = asin(objToCamProj.y);
		T.rotation = glm::quat(sin(angleY / 2), glm::vec3(0, cos(angleY / 2), 0));
		T.rotation *= glm::quat(cos(angleX / 2), glm::vec3(sin(angleX / 2), 0, 0));
	}
}

void Billboard::draw(Camera &camera){
	T.refreshTransform();
	refreshTransform(camera);
	mat.bindMaterial(T, camera);
	if (triMesh != NULL) triMesh->draw();
	else printf("Error! Can't find Billboard Mesh.");
}

void Billboard::fixScale(){

}
//-------------------------------------------------------------------------//

GLuint gLightBufferObject = NULL_HANDLE;
int gNumLights = 0;
Light gLights[MAX_LIGHTS];

void initLightBuffer() 
{
	if (gLightBufferObject != NULL_HANDLE) return;
	glGenBuffers(LIGHT_BUFFER_ID, &gLightBufferObject); // create a new buffer id

	glBindBuffer(GL_UNIFORM_BUFFER, gLightBufferObject); // bind the new buffer
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * MAX_LIGHTS, gLights, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); // unbind buffer
}
//-------------------------------------------------------------------------//
// Node Stuff
//-------------------------------------------------------------------------//

void Node::rotateLocal(glm::vec3 axis, float angle, bool inverse){ //rotates just parent 

	//rotate in opposite direction as parent
	if (inverse)
		axis *= -1;

	//create quaternion equivalent
	float quatAng = glm::radians(angle);
	quatAng /= 2;
	glm::quat r = glm::quat(cos(quatAng), axis.x*sin(quatAng), axis.y*sin(quatAng), axis.z*sin(quatAng));

	//different rotations for each object

		//Mesh Instance
	if (nodeType == 0) {
		cout << nodeType;
		meshInst->T.rotation *= r;
	}
	else{
		cout << "That's no good\n";
	}
}

void Node::translateLocal(glm::vec3 &t, bool inverse){ //transforms just parent
	if (inverse)
		t *= -1;

		//Mesh Instance
	if (nodeType == 0) {
		meshInst->T.translation += t;
	}
	//Default
	else{
		cout << "That's no good\n";
	}
}


void Node::draw(Camera &camera)
{
    this->meshInst->T.refreshTransform();
    this->meshInst->mat.bindNodeMaterial(this, camera);
    if (this->meshInst->triMesh != NULL) this->meshInst->triMesh->draw();
    else printf("Error! Null Mesh.");
}

void Material::bindNodeMaterial(Node* node, Camera &camera)
{
    glUseProgram(shaderProgram);
    
    GLint loc;
    
    
    if(node->parent == NULL)
    {
        loc = glGetUniformLocation(shaderProgram, "uObjectWorldM");
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(node->meshInst->T.transform));
        //
        loc = glGetUniformLocation(shaderProgram, "uObjectWorldInverseM");
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(node->meshInst->T.invTransform));
        //
        glm::mat4x4 objectWorldViewPerspect = camera.worldViewProject * node->meshInst->T.transform;
        loc = glGetUniformLocation(shaderProgram, "uObjectPerpsectM");
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(objectWorldViewPerspect));
        
        
    }
    else
    {
        loc = glGetUniformLocation(shaderProgram, "uObjectWorldM");
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(node->parent->meshInst->T.transform * node->meshInst->T.transform ));
        //
        loc = glGetUniformLocation(shaderProgram, "uObjectWorldInverseM");
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(node->meshInst->T.invTransform * node->parent->meshInst->T.invTransform));
        //
        glm::mat4x4 objectWorldViewPerspect = camera.worldViewProject * node->parent->meshInst->T.transform * node->meshInst->T.transform;
        loc = glGetUniformLocation(shaderProgram, "uObjectPerpsectM");
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(objectWorldViewPerspect));
        
        node->meshInst->T.transform = node->parent->meshInst->T.transform * node->meshInst->T.transform;
        
        if(node->name == "childofChild")
        {
            
        }
    }
    
    // MATERIAL COLORS
    for (int i = 0; i < (int) colors.size(); i++) {
        if (colors[i].id == -1) {
            loc = glGetUniformLocation(shaderProgram, colors[i].name.c_str());
            colors[i].id = loc;
        }
        if (colors[i].id >= 0) {
            glUniform4fv(colors[i].id, 1, &colors[i].val[0]);
        }
    }
    
    loc = glGetUniformLocation(shaderProgram, "uView");
    
    glm::vec4 cameraNormal = glm::vec4(glm::normalize(camera.eye - camera.center), 0);
    
    if(loc != 0) glUniform4fv(loc, 1, glm::value_ptr(cameraNormal));
    
    
    //printVec(colors[0]);
    
    // MATERIAL TEXTURES
    for (int i = 0; i < (int) textures.size(); i++) {
        if (textures[i].id == -1) {
            loc = glGetUniformLocation(shaderProgram, textures[i].name.c_str());
            textures[i].id = loc;
        }
        if (textures[i].id >= 0) {
            //printf("\n%d %d\n", textures[i].id, textures[i].val->samplerId);
            glActiveTexture(GL_TEXTURE0 + i);
            glUniform1i(textures[i].id, i);
            glBindTexture(GL_TEXTURE_2D, textures[i].val->textureId);
            glBindSampler(textures[i].id, textures[i].val->samplerId);
        }
    }
}






//***************************************************************
//Move Script Functions
//***************************************************************

/*void MoveScript::setValue(string property, void* value)
 {
    if(property == "move")
    {
        glm::vec3* vecptr = (glm::vec3*) value;
        this->transVec = *vecptr;
 
    }
    else if(property == "scale")
    {
        glm::vec3* vecptr = (glm::vec3*) value;
        this->scaleVec = *vecptr;
 
 
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

void ControlScript::setValue(string property, void* value)
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
         gWindow = (GLFWwindow*) value;
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
 
     if(scene->nodes["player"] != NULL && (firstPerson || thirdPerson))
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
         scene->camera.refreshTransform((float)width, (float)height);
 
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
         scene->camera.refreshTransform((float)width, (float)height);
 
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
         scene->camera.refreshTransform((float)width, (float)height);
 
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
         scene->camera.refreshTransform((float)width, (float)height);
 
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
         scene->camera.refreshTransform((float)width, (float)height);
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
         scene->camera.refreshTransform((float)width, (float)height);
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
         scene->camera.refreshTransform((float)width, (float)height);
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
         
         scene->camera.refreshTransform((float)width, (float)height);
     }
     if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
     {
         //gScene.nextCamera(gWidth, gHeight);
     }
     
     if(glfwGetKey(gWindow, GLFW_KEY_SPACE) == GLFW_PRESS)// && glfwGetKey(gWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
     {
         //SLEEP(20);
         if(playerPresent && glfwGetKey(gWindow, GLFW_KEY_SPACE) == GLFW_RELEASE)
         {
             if(scene->baseNodes["baseNodeBullet"] != NULL)
             {
                 MoveScript* MS = new MoveScript();
                 MS->maxDist = 50;
                 MS->followSpeed = 1.0;
                 MS->scene = scene;
                 MS->useBulletTrans = true;
                 
                 SpawnScript* newSpawn = new SpawnScript();
                 newSpawn->camera = &scene->camera;
                 newSpawn->scene = scene;
                 newSpawn->moveScript = MS;
                 newSpawn->copyNode = scene->baseNodes["baseNodeBullet"];
                 newSpawn->spawnBullet();
             }
         }
     }
 
     scene->camera.refreshTransform((float)width, (float)height);
 }

void ControlScript::firstPersonControls()
{
    if(scene->nodes["player"] != NULL)
    {
        scene->camera.eye = scene->nodes["player"]->meshInst->T.translation + ( scene->nodes["player"]->meshInst->T.rotation * glm::vec3(0,0,1)) * 20.0f + 2.0f * glm::vec3(0,1,0);
        scene->camera.center = scene->nodes["player"]->meshInst->T.translation + 2.0f * glm::vec3(0,1,0);
        scene->camera.refreshTransform(width, height);
        
        //cout << scene->camera.center.x << endl << "eye = " << scene->camera.eye.z << endl;
    }

}

void ControlScript::thirdPersonControls()
{
    if(scene->nodes["player"] != NULL)
    {
        scene->camera.eye = scene->nodes["player"]->meshInst->T.translation + (scene->nodes["player"]->meshInst->T.rotation * glm::vec3(0,0,1)) * 5.0f + 5.0f * glm::vec3(0,1,0);
        scene->camera.center = scene->nodes["player"]->meshInst->T.translation + 1.0f * glm::vec3(0,1,0);
        scene->camera.refreshTransform(width, height);
        
        
    }
    
}

void ControlScript::runScripts(){
    
    if(keyboard)
    {
        keyboardControls();
    }
    
    if(thirdPerson)
    {
        thirdPersonControls();
        //cout << "third" << endl;
    }
    else
    {
        firstPersonControls();
        //cout << "first" << endl;
    }
}


void ControlScript::useKeyboard(bool option){
    
    this->keyboard = option;
}

void ControlScript::useFirstPerson(bool option)
{
    this->firstPerson = option;
    
    if(firstPerson && thirdPerson)
    {
        this->thirdPerson = false;
    }
}

void ControlScript::useThirdPerson(bool option)
{
    this->thirdPerson = option;
    
    if(firstPerson && thirdPerson)
    {
        this->firstPerson = false;
    }
    
}




//***************************************************************
//Move Script Functions
//***************************************************************
void MoveScript::setValue(string property, void* value)
{
    if(property == "node")
    {
        this->node = (Node*)value;
        
    }
    else if(property == "translate")
    {
        glm::vec3* vecPtr = (glm::vec3*) value;
        this->transVec = *vecPtr;
    }
    else if(property == "scale")
    {
        glm::vec3* vecPtr = (glm::vec3*) value;
        this->scaleVec = *vecPtr;
    }
    else if(property == "target")
    {
        this->targetNode = (Node*)value;
    }
    else if(property == "minTrans")
    {
        float* min = (float*) value;
        this->minTransX = *min;
        minTransX = minTransY = minTransZ;
    }
    else if(property == "followDist")
    {
        float* follow = (float*) value;
        this->followDist = *follow;
    }
    else if(property == "followSpeed")
    {
        float* follow = (float*) value;
        this->followSpeed = *follow;
    }
}

void MoveScript::setValue(string property, void* value1, void* value2)
{
    if(property == "rotate")
    {
        glm::vec3* vecPtr = (glm::vec3*) value1;
        this->axis = *vecPtr;
        
        float* flPtr = (float*) value2;
        this->angle = *flPtr;
    }
}



void MoveScript::setValue(string property1, string property2, void* value)
{
    if(property1 == "maxTrans")
    {
        float* max = (float*) value;
        
        
        
        if(property2 == "X" || property2 == "x")
        {
            this->maxTransX = *max;
        }
        else if(property2 == "Y" || property2 == "y")
        {
            this->maxTransY = *max;
        }
        else if(property2 =="Z" || property2 == "z")
        {
            this->maxTransZ = *max;
        }
    }

}



void MoveScript::globalRotate()
{
    node->meshInst->T.rotateGlobal(axis, angle);
}

void MoveScript::localRotate()
{
    node->meshInst->T.scale = scaleVec;
}

void MoveScript::localTrans()
{
    node->meshInst->T.translateLocal(transVec);
}

void MoveScript::globalTrans()
{
    node->meshInst->T.translateGlobal(transVec);
}
void MoveScript::setScale()
{
    node->meshInst->T.scale = scaleVec;
}

void MoveScript::localTransLimited()
{
    if(!minSet)
    {
        this->minTransX = node->meshInst->T.translation.x;
        this->minTransY = node->meshInst->T.translation.y;
        this->minTransZ = node->meshInst->T.translation.z;
        minSet = true;
        //cout << "reached it" << endl;
    }
    
    globalTrans();
    
    if(this->minTransZ > node->meshInst->T.translation.z)
    {
        transVec.z = -transVec.z;
    }
    else if(this->maxTransZ < node->meshInst->T.translation.z)
    {
        transVec.z = -transVec.z;
    }
    
    if(this->minTransX > node->meshInst->T.translation.x)
    {
        transVec.x = -transVec.x;
    }
    else if(this->maxTransX < node->meshInst->T.translation.x)
    {
        transVec.x = -transVec.x;
    }
    
    if(this->minTransY > node->meshInst->T.translation.y)
    {
        transVec.y = -transVec.y;
    }
    else if(this->maxTransY < node->meshInst->T.translation.y)
    {
        transVec.y = -transVec.y;
    }
    
    node->meshInst->T.refreshTransform();
    //camera->refreshTransform(600, 600);
}

void MoveScript::followPlayer()
{
    //cout << "Player y = " << playerNode->meshInst->T.translation.z << endl;
    //cout << "Node y = " << node->meshInst->T.translation.z << endl;

    
    float zDist = node->meshInst->T.translation.z - targetNode->meshInst->T.translation.z;
    float yDist = node->meshInst->T.translation.y - targetNode->meshInst->T.translation.y;
    float xDist = node->meshInst->T.translation.x - targetNode->meshInst->T.translation.x;
   // cout << "yDist = " << yDist << " X Dist = " << xDist << endl;

    if(abs(zDist) <= followDist)
    {
        //return;
    }
    else
    {
        if(zDist < 0)
        {
            node->meshInst->T.translation.z += followSpeed;
        }
        else
        {
            node->meshInst->T.translation.z -= followSpeed;
        }
    }
    
    if(abs(yDist) <= followDist)
    {
       // cout << "return y" << endl;
        //eturn;
    }
    else
    {
        if(yDist < 0)
        {
            node->meshInst->T.translation.y += followSpeed;
        }
        else
        {
            node->meshInst->T.translation.y -= followSpeed;
        }
        
       // cout << "player changed dir node z = " << node->meshInst->T.translation.z << endl;
    }
 
    if(abs(xDist) <= followDist)
    {
        //cout << "return x" << endl;
        //return;
        
    }
    else
    {
        if(xDist < 0)
        {
            node->meshInst->T.translation.x += followSpeed;
        }
        else
        {
            node->meshInst->T.translation.x -= followSpeed;
        }
        
    }
    
}

void MoveScript::faceTarget()
{
    glm::vec3 objToCamProj;
    float angleY, angleX;
    
    objToCamProj = glm::vec3(targetNode->meshInst->T.translation);
    objToCamProj = glm::normalize(objToCamProj);
    angleY = -atan2(objToCamProj.x, objToCamProj.z);
    angleX = asin(objToCamProj.y);
    node->meshInst->T.rotation = glm::quat(sin(angleY / 2), glm::vec3(0, cos(angleY / 2), 0));
    node->meshInst->T.rotation *= glm::quat(cos(angleX / 2), glm::vec3(sin(angleX / 2), 0, 0));
}


void MoveScript::bulletTranslation()
{
    if(scene->nodes["player"] != NULL)
    {
        node->meshInst->T.translateLocal(glm::vec3(0,0,-followSpeed), scene->camera);
        distCounter += followSpeed;
        if(distCounter >= maxDist)
        {
            scene->nodes.erase(node->name);
            useBulletTrans = false;
            //delete this;
        }
    }
    
    
}

void MoveScript::runScripts()
{
    if(useFaceTarget)
    {
        faceTarget();
    }
    if(useGlobalRotate)
    {
        globalRotate();
    }
    if(useLocalRotate)
    {
        localRotate();
    }
    if(useLocalTrans)
    {
        localTrans();
    }
    if(useGlobalTrans)
    {
        globalTrans();
    }
    if(useSetScale)
    {
        setScale();
    }
    if(useLimitedTrans)
    {
        localTransLimited();
    }
    if(useFollowPlayer)
    {
        followPlayer();
    }
    if(useBulletTrans)
    {
        bulletTranslation();
    }

}

//*****************
//Scene Functions
//****************


void Scene::runScripts()
{
    
    
    for(int i = 0; i < spawnScripts.size(); i++)
    {
        spawnScripts[i]->runScripts();
    }
    for(int i = 0; i < controlScripts.size(); i++)
    {
        controlScripts[i]->runScripts();
    }
    
    for(auto& x : moveScripts)
    {
        
        x.second->runScripts();
        int i = 0;
    }

    
    
   
}


//*****************
//SpawnScript Functions
//****************


void SpawnScript::spawnNode()
{  
    std::ostringstream oss;
    oss << numOfSpawn;
    //cout << "spawn Num = " << spawnNum << endl;
    name = name + oss.str();
    Node* newNode = copyNode;
    node = new Node();
    *node = *newNode;
    node->name = name;
    TriMeshInstance* newInst = copyNode->meshInst;
    node->meshInst = new TriMeshInstance;
    *node->meshInst = *newInst;
    node->meshInst->T.translation = spawnloc;
    
    if(didSpawnBullet)
    {
        node->meshInst->T.rotation = scene->nodes["player"]->meshInst->T.rotation;
    }
    
    if(moveScript != NULL)
    {
        attachMoveScript();
    }
    
    scene->nodes[name] = this->node;
    int i = 0;
}

void SpawnScript::attachMoveScript()
{
    MoveScript* ms = moveScript;
    MoveScript* newMS = new MoveScript();
    
    *newMS = *ms;
    newMS->node = node;
    moveScript = newMS;
    
    scene->moveScripts[name] = moveScript;
    
    cout << name << endl;
}

void SpawnScript::spawnBullet()
{
    if(scene->nodes["player"] != NULL)
    {
        spawnloc = scene->nodes["player"]->meshInst->T.translation;
        didSpawnBullet = true;
        spawnNode();
    }
}

void SpawnScript::runScripts()
{

    if(this->useSpawn)
    {
        spawnNode();
    }

}



int SpawnScript::spawnNum = 0;

//*****************
//Particle System Functions
//****************

Particle::Particle(glm::vec3 VelMag, glm::vec3 _Pos, glm::vec3 _Accel, float _life, Billboard inst, int type){
	if (type == PS_EXPLOSION){
		Vel = glm::vec3(glm::linearRand(-VelMag.x, VelMag.x), glm::linearRand(-VelMag.y, VelMag.y), glm::linearRand(-VelMag.z, VelMag.z));
	}
	if (type == PS_FOUNTAIN){
		Vel = glm::vec3(glm::linearRand(-VelMag.x, VelMag.x), glm::linearRand(0.0f, VelMag.y), glm::linearRand(-VelMag.z, VelMag.z));
	}
	T.translation = _Pos;
	Accel = _Accel;
	instance = inst;
	life = _life;
	instance.setTranslation(T.translation); //This is broken!
}

void Particle::update(){
	T.translation = T.translation + Vel;
	Vel = Vel + Accel;
	life -= 1.0;
	instance.setTranslation(T.translation);
	//instance.setScale(instance.T.scale * (life / 1000));
}
void Particle::render(Camera &camera){
	instance.draw(camera);
}


void partSys::initPS(){
	Particle emitter(glm::vec3(0, 0, 0), Origin, glm::vec3(0, 0, 0), duration, bb, type);
	particles.push_back(emitter);
}

void partSys::addParticle(){
	Particle p(VelMag, Origin, Accel, life, bb, type);
	particles.push_back(p);
}

void partSys::update(){
	for (int i = particles.size() - 1; i > 0; i--){
		Particle p = particles.at(i);
		particles.at(i).update();
		if (p.isDead()){
			particles.erase(particles.begin() + i);
		}
	}
	particles[0].update();
}

void partSys::render(Camera &camera){
	for (int i = particles.size() - 1; i >= 0; i--){
		particles[i].render(camera);
	}
	addParticle();
}

partSys::partSys(glm::vec3 Pos, glm::vec3 _Vel, glm::vec3 _Accel, glm::vec3 _VelMag, Billboard _bb, float _duration, float _life, int _type){
	Origin = Pos;
	Vel = _Vel;
	Accel = _Accel;
	VelMag = _VelMag;
	bb = _bb;
	duration = _duration;
	life = _life;
	type = _type;
	initPS();
}



