#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include "OBJReader.h"
#include "MTLReader.h"
#include "stb_image.h"

enum Axis { X, Y, Z };
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float speed);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void readOptionsFile(string filepath);
void initWindow();
void objectMapping();
float hexStringToFloat(string hexStr);
void updateClearColor(string colorString);
void moveObj(int index, glm::vec3 movement);
void scaleObj(int index, float scaleFactor);
void rotateObj(int index, float rotation, Axis axis);

// Dimensoes da tela (Substituidas pelas dimensoes indicadas no arquivo de opcoes)
int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;
float bgColors[3] = { 0.0f, 0.0f, 0.0f };
short movementSelection = 0;
enum MovementType { TRANSLATION, ROTATION };
MovementType selectedMovement = TRANSLATION;


// Criacao da janela e constantes de versoes.
GLFWwindow* window = NULL;
const GLubyte* renderer;
const GLubyte* version;

// Posicoes padrao da camera
glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp;
float speedConst;

// Opcoes do mouse
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// Vetor de armazenamento de objetos
vector <Obj3D*> objects;

// Variaveis de tempo 
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float ambientStrength = 1.5f;
float specularStrength = 2.0f;

int main() {
	// Atualiza a posicao da camera, resolucao da tela e adiciona os objetos
	readOptionsFile("./options.txt");

	// Vertex Shader
	const char* vertex_shader =
		"#version 410\n"
		"layout(location=0) in vec3 vp;\n"
		"layout (location=1) in vec2 vt;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"out vec2 texture_coordinates;\n"
		"void main () {\n"
		"	texture_coordinates = vt;\n"
		"	gl_Position = projection * view * model * vec4(vp, 1.0f);\n"
		"}";

	// Fragment Shader
	const char* fragment_shader =
		"#version 410\n"
		"in vec2 texture_coordinates;\n"
		"in vec3 frag_position;\n"
		"in vec3 normal;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D basic_texture;\n"
		"uniform vec3 light_position;\n"
		"uniform vec3 view_position;\n"
		"uniform vec3 object_color;\n"
		"uniform float ambient_strength;\n"
		"uniform float specular_strength;\n"
		"void main () {\n"
		"   vec3 ambient = ambient_strength * object_color;\n"
		"   vec3 light_color = vec3(1.0);\n"
		"   vec3 light_direction = normalize(light_position - frag_position);\n"
		"   vec3 normal_normalized = normalize(normal);\n"
		"   float diff = max(dot(normal_normalized, light_direction), 0.0);\n"
		"   vec3 diffuse = diff * light_color;\n"
		"   vec3 view_direction = normalize(view_position - frag_position);\n"
		"   vec3 reflect_direction = reflect(-light_direction, normal_normalized);\n"
		"   float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 32);\n"
		"   vec3 specular = specular_strength * spec * light_color;\n"
		"   vec4 texel = texture(basic_texture, texture_coordinates);\n"
		"   vec3 result = (ambient + diffuse + specular) * vec3(texel);\n"
		"   frag_color = vec4(result, 1.0);\n"
		"}\n";

	GLuint vertexShader, fragShader;
	GLuint shaderProgram;

	// Inicializa a janela
	initWindow();

	// Cria objetos
	objectMapping();

	// Criacao e compilacao dos shaders
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex_shader, NULL);
	glCompileShader(vertexShader);

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragment_shader, NULL);
	glCompileShader(fragShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, fragShader);
	glAttachShader(shaderProgram, vertexShader);
	glLinkProgram(shaderProgram);

	float timeNow = 0;
	glUseProgram(shaderProgram);

	glm::vec3 objectColor(0.8f, 0.8f, 0.8f);

	glm::vec3 lightPos(1.0f, 2.0f, 2.0f);
	glm::vec3 viewPos = cameraPos;

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = static_cast<float>(glfwGetTime());

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window, speedConst);

		glClearColor(bgColors[0], bgColors[1], bgColors[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Atualização dos valores dos uniformes no shader
		glUseProgram(shaderProgram);
		glUniform3fv(glGetUniformLocation(shaderProgram, "object_color"), 1, &objectColor[0]);
		glUniform1f(glGetUniformLocation(shaderProgram, "ambient_strength"), ambientStrength);
		glUniform1f(glGetUniformLocation(shaderProgram, "specular_strength"), specularStrength);

		// Atualiza matriz de projecao
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Atualiza matriz view
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Renderiza objetos
		for (Obj3D* obj : objects) {
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(obj->transform));
			for (Group* g : obj->mesh->groups) {
				glBindVertexArray(g->vao); // Faz bind com VAO
				glBindTexture(GL_TEXTURE_2D, g->mat->TID_mapKd); // Faz bind com textura do material
				glDrawArrays(GL_TRIANGLES, 0, g->faces.size() * 3); // Desenha triangulos
				glBindTexture(GL_TEXTURE_2D, 0); // Remove bind da textura para nao interferir com outros objetos
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow* window, float speed) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// Define selecao de movimento
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		movementSelection = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		movementSelection = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		movementSelection = 2;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		movementSelection = 3;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		movementSelection = 4;
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		movementSelection = 5;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		movementSelection = 6;
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		movementSelection = 7;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
		movementSelection = 8;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		movementSelection = 9;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		selectedMovement = TRANSLATION;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		selectedMovement = ROTATION;
	}

	float cameraSpeed = static_cast<float>(speed * deltaTime);
	float scaleFactor = 0.005f;
	float rotationIncrement = 1;


	if (movementSelection > objects.size()) return;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

		if (movementSelection == 0) {
			cameraPos += cameraSpeed * cameraFront;
		}
		else if (selectedMovement == TRANSLATION) {
			moveObj(movementSelection - 1, glm::vec3(0, 0, cameraSpeed));
		}
		else {
			rotateObj(movementSelection - 1, rotationIncrement, X);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		if (movementSelection == 0) {
			cameraPos -= cameraSpeed * cameraFront;
		}
		else if (selectedMovement == TRANSLATION) {
			moveObj(movementSelection - 1, glm::vec3(0, 0, -cameraSpeed));
		}
		else {
			rotateObj(movementSelection - 1, -rotationIncrement, X);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		if (movementSelection == 0) {
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
		else if (selectedMovement == TRANSLATION) {
			moveObj(movementSelection - 1, glm::vec3(cameraSpeed, 0, 0));
		}
		else {
			rotateObj(movementSelection - 1, rotationIncrement, Y);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		if (movementSelection == 0) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
		else if (selectedMovement == TRANSLATION) {
			moveObj(movementSelection - 1, glm::vec3(-cameraSpeed, 0, 0));
		}
		else {
			rotateObj(movementSelection - 1, -rotationIncrement, Y);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (movementSelection == 0) {
			cameraPos += cameraUp * cameraSpeed;
		}
		else if (selectedMovement == TRANSLATION) {
			moveObj(movementSelection - 1, glm::vec3(0, cameraSpeed, 0));
		}
		else {
			rotateObj(movementSelection - 1, rotationIncrement, Z);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		if (movementSelection == 0) {
			cameraPos -= cameraUp * cameraSpeed;
		}
		else if (selectedMovement == TRANSLATION) {
			moveObj(movementSelection - 1, glm::vec3(0, -cameraSpeed, 0));
		}
		else {
			rotateObj(movementSelection - 1, -rotationIncrement, Z);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && movementSelection != 0) {
		scaleObj(movementSelection - 1, 1 + scaleFactor);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && movementSelection != 0) {
		scaleObj(movementSelection - 1, 1 - scaleFactor);
	}

}

void moveObj(int index, glm::vec3 movement) {
	Obj3D* obj = objects.at(index);
	glm::mat4 trans = obj->transform;
	obj->transform = glm::translate(trans, movement);
	glm::vec4 newMinCollision = trans * obj->mesh->min;
	glm::vec4 newMaxCollision = trans * obj->mesh->max;
	obj->mesh->max = newMaxCollision;
	obj->mesh->min = newMinCollision;
}

void scaleObj(int index, float scaleFactor) {
	Obj3D* obj = objects.at(index);
	glm::mat4 trans = obj->transform;
	obj->transform = glm::scale(trans, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
	glm::vec4 newMinCollision = trans * obj->mesh->min;
	glm::vec4 newMaxCollision = trans * obj->mesh->max;
	obj->mesh->max = newMaxCollision;
	obj->mesh->min = newMinCollision;
}

void rotateObj(int index, float rotation, Axis axis) {
	Obj3D* obj = objects.at(index);
	glm::mat4 trans = obj->transform;
	obj->transform = glm::rotate(trans, glm::radians(rotation), glm::vec3(axis == X ? 1 : 0, axis == Y ? 1 : 0, axis == Z ? 1 : 0));
	glm::vec4 newMinCollision = trans * obj->mesh->min;
	glm::vec4 newMaxCollision = trans * obj->mesh->max;
	obj->mesh->max = newMaxCollision;
	obj->mesh->min = newMinCollision;
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Valor inverso pois as coordenadas y sao da base para o topo
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Define valores maximos e minimos de pitch
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

// Funcao chamada quando usuario utiliza o scroll do mouse
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}

// Funcao que le e atualiza variaveis de acordo com preferencias do usuario
void readOptionsFile(string filepath) {
	OBJReader objreader;
	ifstream file(filepath);

	while (!file.eof()) {
		string line;
		getline(file, line);

		stringstream sline;
		sline << line;

		string temp;
		sline >> temp;

		if (temp == "camera") {
			float eyeX, eyeY, eyeZ, dirX, dirY, dirZ, upX, upY, upZ, speed;

			sline >> eyeX >> eyeY >> eyeZ >> dirX >> dirY >> dirZ >> upX >> upY >> upZ >> speed;
			cameraPos = glm::vec3(eyeX, eyeY, eyeZ);;
			cameraFront = glm::vec3(dirX, dirY, dirZ);
			cameraUp = glm::vec3(upX, upY, upZ);
			speedConst = speed;
		}
		else if (temp == "viewport") {
			int width, height;
			string bgColor;
			sline >> width >> height >> bgColor;
			SCR_WIDTH = width;
			SCR_HEIGHT = height;
			updateClearColor(bgColor);
		}
		else if (temp == "obj") {
			Obj3D* obj = new Obj3D;
			string objectPath;

			float translateX, translateY, translateZ, rotate, scale;
			sline >> objectPath >> translateX >> translateY >> translateZ >> rotate >> scale;
			printf("Loading %s\n", objectPath.c_str());

			obj = objreader.readFile(objectPath);
			glm::mat4 trans = glm::mat4(1.0);
			trans = glm::translate(trans, glm::vec3(translateX, translateY, translateZ));
			trans = glm::rotate(trans, glm::radians(rotate), glm::vec3(0.0, 1.0, 0.0));
			trans = glm::scale(trans, glm::vec3(scale, scale, scale));
			glm::vec4 newMinCollision = trans * obj->mesh->min;
			glm::vec4 newMaxCollision = trans * obj->mesh->max;
			obj->mesh->max = newMaxCollision;
			obj->mesh->min = newMinCollision;
			obj->transform = trans;

			objects.push_back(obj);
		}
		else if (temp == "light") {
			sline >> ambientStrength >> specularStrength;
		}
	}
}

// Configuracoes da janela do glfw e glew, e dos callbacks
void initWindow() {
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: unable to start GLFW3\n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(
		SCR_WIDTH, SCR_HEIGHT, "OBJ Viewer", NULL, NULL
	);

	if (!window) {
		fprintf(stderr, "ERROR: unable to open window with GLFW3\n");
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glewExperimental = GL_TRUE;
	glewInit();

	renderer = glGetString(GL_RENDERER);
	version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

// Cria VBO e VAO de cada objeto e le textura
void objectMapping() {
	for (Obj3D* obj : objects) {
		string centralPath = obj->filePath;
		for (Group* g : obj->mesh->groups) {
			if (g->mat->map_kd != "") {
				string textPath = centralPath + g->mat->map_kd;
				const char* c = textPath.c_str();

				GLuint genText;
				int x, y, n;
				int force_channels = 4;
				glEnable(GL_TEXTURE_2D);
				stbi_set_flip_vertically_on_load(true);
				unsigned char* image_data = stbi_load(c, &x, &y, &n, force_channels);

				if (!image_data) {
					fprintf(stderr, "ERROR: could not load %s\n", c);
				}

				if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
					fprintf(
						stderr, "WARNING: texture %s is not power-of-2 dimensions\n", c
					);
				}

				glGenTextures(1, &genText);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, genText);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				GLfloat max_aniso = 0.0f;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

				g->mat->TID_mapKd = genText;
			}

			vector<float> vs;
			vector<float> vts;
			vector<float> vns;

			for (Face* f : g->faces) {
				for (int i = 0; i < f->verts.size(); i++) {
					glm::vec3* v = obj->mesh->vertex[f->verts[i]];
					vs.push_back(v->x);
					vs.push_back(v->y);
					vs.push_back(v->z);
				}

				for (int i = 0; i < f->texts.size(); i++) {
					if (f->texts[i] == -1) {
						vts.push_back(0);
						vts.push_back(0);
					}
					else {
						glm::vec2* vt = obj->mesh->mappings[f->texts[i]];
						vts.push_back(vt->x);
						vts.push_back(vt->y);
					}
				}
			}

			GLuint vao;
			GLuint vboV, vboVT;

			glGenBuffers(1, &vboV);
			glBindBuffer(GL_ARRAY_BUFFER, vboV);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vs.size(), vs.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &vboVT);
			glBindBuffer(GL_ARRAY_BUFFER, vboVT);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vts.size(), vts.data(), GL_STATIC_DRAW);

			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vboV);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, vboVT);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			g->vao = vao;
		}
	}
}

float hexStringToFloat(string hexStr) {
	int hexValue;
	std::stringstream ss;
	ss << std::hex << hexStr;
	ss >> hexValue;
	return static_cast<float>(hexValue) / 255.0f;
}

void updateClearColor(string colorString) {
	if (colorString.size() != 7 || colorString[0] != '#') {
		fprintf(stderr, "ERROR: Invalid color string format.\n");
		return;
	}

	float red = hexStringToFloat(colorString.substr(1, 2));
	float green = hexStringToFloat(colorString.substr(3, 2));
	float blue = hexStringToFloat(colorString.substr(5, 2));

	bgColors[0] = red;
	bgColors[1] = green;
	bgColors[2] = blue;
}