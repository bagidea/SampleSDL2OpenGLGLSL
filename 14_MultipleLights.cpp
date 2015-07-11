#include <iostream>
#include <string>
#include <fstream>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MATH_PI 3.14159265359f

using namespace std;

SDL_Window* window;
SDL_GLContext gl;

GLuint program[2];
GLuint vertexShader[2];
GLuint fragmentShader[2];

GLuint VAO, lightVAO, VBO;

GLuint gProj[2];
GLuint gView[2];
GLuint gModel[2];

GLuint gObjectColor[2];

GLuint gMaterial_diffuseTexture;
GLuint gMaterial_specularTexture;
GLuint gMaterial_color;
GLuint gMaterial_shineness;

GLuint gDirectionalLight_direction;
GLuint gDirectionalLight_ambient;
GLuint gDirectionalLight_diffuse;
GLuint gDirectionalLight_specular;

GLuint gPointLight_position[3];
GLuint gPointLight_constant[3];
GLuint gPointLight_linear[3];
GLuint gPointLight_quadratic[3];
GLuint gPointLight_ambient[3];
GLuint gPointLight_diffuse[3];
GLuint gPointLight_specular[3];

GLuint gSpotLight_position;
GLuint gSpotLight_direction;
GLuint gSpotLight_constant;
GLuint gSpotLight_linear;
GLuint gSpotLight_quadratic;
GLuint gSpotLight_cutOff;
GLuint gSpotLight_outerCutOff;
GLuint gSpotLight_ambient;
GLuint gSpotLight_diffuse;
GLuint gSpotLight_specular;

GLuint gViewPosition;

GLfloat lightPos[3];
bool chk;

unsigned int tex;
unsigned int tex2;

glm::vec3 camPos;
GLfloat camYaw, camPitch, lastX, lastY;

unsigned int LoadImage(string path)
{
	SDL_Surface* img = IMG_Load(path.c_str());
	unsigned int id;

	if(img == NULL)
	{
		cout << "Can't Load Image : " << path << endl;
	}else{
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		GLenum formatTexture;
		switch(img->format->BytesPerPixel)
		{
		case 4:
			if(img->format->Rmask == 0x000000ff)
			{
				formatTexture = GL_RGBA;
			}else{
				formatTexture = GL_BGRA;
			}
			break;
		case 3:
			if(img->format->Rmask == 0x000000ff)
			{
				formatTexture = GL_RGB;
			}else{
				formatTexture = GL_BGR;
			}
			break;
		default:
			cout << "The Image not Truecolor." << endl;
		}


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, formatTexture, img->w, img->h, 0, formatTexture, GL_UNSIGNED_BYTE, img->pixels);

		SDL_FreeSurface(img);
		img = NULL;
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return id;
}

string LoadFile(string path)
{
	string line, output;
	output = "";

	ifstream in(path.c_str());
	if(!in.is_open())
	{
		cout << "Can't Load File " << path << endl;
		return output;
	}else{
		while(in.good())
		{
			getline(in, line);
			output.append(line+"\n");
		}
	}

	return output;
}

GLuint CompileShader(string text, unsigned int shaderType)
{
	GLuint shader = glCreateShader(shaderType);

	const char* source[1];
	source[0] = text.c_str();

	GLint length[1];
	length[0] = text.length();

	glShaderSource(shader, 1, source, length);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		GLchar error[1024];
		glGetShaderInfoLog(shader, 1024, NULL, error);
		cout << error << endl;
	}

	return shader;
}

void LoadShader(string _vertexShader, string _fragmentShader, int id)
{
	program[id] = glCreateProgram();
	vertexShader[id] = CompileShader(LoadFile(_vertexShader), GL_VERTEX_SHADER);
	fragmentShader[id] = CompileShader(LoadFile(_fragmentShader), GL_FRAGMENT_SHADER);

	if(vertexShader[id] == NULL && fragmentShader[id] == NULL)
	{
		cout << "Compile Shader Fail." << endl;
	}else{
		glAttachShader(program[id], vertexShader[id]);
		glAttachShader(program[id], fragmentShader[id]);
		glLinkProgram(program[id]);

		GLint success;
		glGetProgramiv(program[id], GL_LINK_STATUS, &success);
		if(!success)
		{
			GLchar error[1024];
			glGetProgramInfoLog(program[id], 1024, NULL, error);
			cout << error << endl;	
		}
	}
}

void Init()
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Multiple Lights", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	gl = SDL_GL_CreateContext(window);
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	LoadShader("shader/Shader5.vs", "shader/Shader9.fs", 0);
	LoadShader("shader/Shader5_2.vs", "shader/Shader5_2.fs", 1);
}

void Start()
{
	gProj[0] = glGetUniformLocation(program[0], "gProjection");
	gView[0] = glGetUniformLocation(program[0], "gView");
	gModel[0] = glGetUniformLocation(program[0], "gModel");
	gProj[1] = glGetUniformLocation(program[1], "gProjection");
	gView[1] = glGetUniformLocation(program[1], "gView");
	gModel[1] = glGetUniformLocation(program[1], "gModel");
	gObjectColor[1] = glGetUniformLocation(program[1], "objectColor");

	gMaterial_diffuseTexture = glGetUniformLocation(program[0], "material.diffuse");
	gMaterial_specularTexture = glGetUniformLocation(program[0], "material.specular");
	gMaterial_color = glGetUniformLocation(program[0], "material.color");
	gMaterial_shineness = glGetUniformLocation(program[0], "material.shineness");

	gDirectionalLight_direction = glGetUniformLocation(program[0], "directionalLight.direction");
	gDirectionalLight_ambient = glGetUniformLocation(program[0], "directionalLight.ambient");
	gDirectionalLight_diffuse = glGetUniformLocation(program[0], "directionalLight.diffuse");
	gDirectionalLight_specular = glGetUniformLocation(program[0], "directionalLight.specular");

	gPointLight_position[0] = glGetUniformLocation(program[0], "pointLight[0].position");
	gPointLight_constant[0] = glGetUniformLocation(program[0], "pointLight[0].constant");
	gPointLight_linear[0] = glGetUniformLocation(program[0], "pointLight[0].linear");
	gPointLight_quadratic[0] = glGetUniformLocation(program[0], "pointLight[0].quadratic");
	gPointLight_ambient[0] = glGetUniformLocation(program[0], "pointLight[0].ambient");
	gPointLight_diffuse[0] = glGetUniformLocation(program[0], "pointLight[0].diffuse");
	gPointLight_specular[0] = glGetUniformLocation(program[0], "pointLight[0].specular");

	gPointLight_position[1] = glGetUniformLocation(program[0], "pointLight[1].position");
	gPointLight_constant[1] = glGetUniformLocation(program[0], "pointLight[1].constant");
	gPointLight_linear[1] = glGetUniformLocation(program[0], "pointLight[1].linear");
	gPointLight_quadratic[1] = glGetUniformLocation(program[0], "pointLight[1].quadratic");
	gPointLight_ambient[1] = glGetUniformLocation(program[0], "pointLight[1].ambient");
	gPointLight_diffuse[1] = glGetUniformLocation(program[0], "pointLight[1].diffuse");
	gPointLight_specular[1] = glGetUniformLocation(program[0], "pointLight[1].specular");

	gPointLight_position[2] = glGetUniformLocation(program[0], "pointLight[2].position");
	gPointLight_constant[2] = glGetUniformLocation(program[0], "pointLight[2].constant");
	gPointLight_linear[2] = glGetUniformLocation(program[0], "pointLight[2].linear");
	gPointLight_quadratic[2] = glGetUniformLocation(program[0], "pointLight[2].quadratic");
	gPointLight_ambient[2] = glGetUniformLocation(program[0], "pointLight[2].ambient");
	gPointLight_diffuse[2] = glGetUniformLocation(program[0], "pointLight[2].diffuse");
	gPointLight_specular[2] = glGetUniformLocation(program[0], "pointLight[2].specular");

	gSpotLight_position = glGetUniformLocation(program[0], "spotLight.position");
	gSpotLight_direction = glGetUniformLocation(program[0], "spotLight.direction");
	gSpotLight_constant = glGetUniformLocation(program[0], "spotLight.constant");
	gSpotLight_linear = glGetUniformLocation(program[0], "spotLight.linear");
	gSpotLight_quadratic = glGetUniformLocation(program[0], "spotLight.quadratic");
	gSpotLight_cutOff = glGetUniformLocation(program[0], "spotLight.cutOff");
	gSpotLight_outerCutOff = glGetUniformLocation(program[0], "spotLight.outerCutOff");
	gSpotLight_ambient = glGetUniformLocation(program[0], "spotLight.ambient");
	gSpotLight_diffuse = glGetUniformLocation(program[0], "spotLight.diffuse");
	gSpotLight_specular = glGetUniformLocation(program[0], "spotLight.specular");

	gViewPosition = glGetUniformLocation(program[0], "viewPosition");

	lightPos[0] = 0.0f;
	lightPos[1] = 3.0f;
	lightPos[2] = 6.0f;

	chk = false;

	tex = LoadImage("source/box.jpg");
	tex2 = LoadImage("source/box_spec.jpg");

	GLfloat vec[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

	    -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec), vec, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec), vec, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	camPos = glm::vec3(3.0f, 2.0f, -5.0f);

	camYaw = 0.0f;
	camPitch = -10.0f*(MATH_PI/180.0f);
	lastX = 400.0f;
	lastY = 300.0f;

	SDL_WarpMouseInWindow(window, lastX, lastY);
}

void MouseController()
{
	int mX, mY;
	SDL_GetMouseState(&mX, &mY);

	camYaw += 0.005f * (lastX-mX);
	camPitch += 0.005f * (lastY-mY);

	SDL_WarpMouseInWindow(window, lastX, lastY);

	if(camYaw/(MATH_PI/180.0f) > 360.0f)
	{
		camYaw = 0.0f;
	}
	else if(camYaw/(MATH_PI/180.0f) <= 0.0f)
	{
		camYaw = 360.0f*(MATH_PI/180.0f);
	}

	if(camPitch/(MATH_PI/180.0f) > 90.0f)
	{
		camPitch = 90.0f*(MATH_PI/180.0f);
	}
	else if(camPitch/(MATH_PI/180.0f) < -90.0f)
	{
		camPitch = -90.0f*(MATH_PI/180.0f);
	}
}

void KeyboardController(glm::vec3 dir, glm::vec3 right, GLfloat speed)
{
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if(state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W])
	{	
		camPos += speed * dir;
	}
	else if(state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S])
	{	
		camPos -= speed * dir;
	}
	if(state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A])
	{	
		camPos -= speed * right;
	}
	else if(state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D])
	{	
		camPos += speed * right;
	}
}

void Update()
{
	MouseController();

	glm::mat4 projection;
	glm::mat4 view;

	projection = glm::perspective(45.0f, 800.0f/600.0f, 0.1f, 100.0f);
	
	glm::vec3 camDir(cosf(camPitch) * sinf(camYaw), sinf(camPitch), cosf(camPitch) * cosf(camYaw));
	glm::vec3 camRight(sinf(camYaw - MATH_PI/2.0f), 0.0f, cosf(camYaw - MATH_PI/2.0f));
	glm::vec3 camUp = cross(camRight, camDir);

	view = glm::lookAt(camPos, camPos + camDir, camUp);
	KeyboardController(camDir, camRight, 0.01f);

	glUseProgram(program[0]);

	glUniformMatrix4fv(gProj[0], 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(gView[0], 1, GL_FALSE, glm::value_ptr(view));

	//Material
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(gMaterial_diffuseTexture, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex2);
	glUniform1i(gMaterial_specularTexture, 1);

	glUniform3f(gMaterial_color, 0.8f, 0.8f, 0.8f);
	glUniform1f(gMaterial_shineness, 512.0f);
	
	//Directional Light
	glUniform3f(gDirectionalLight_direction, -10.0f, -5.0f, 10.0f);
	glUniform3f(gDirectionalLight_ambient, 0.05f, 0.05f, 0.05f);
	glUniform3f(gDirectionalLight_diffuse, 0.3f, 0.05f, 0.05f);
	glUniform3f(gDirectionalLight_specular, 0.1f, 0.1f, 0.1f);

	//Point Lights
	glm::vec3 PointDiffuse[] = {
		glm::vec3(3.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 3.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 3.0f)
	};

	if(!chk)
	{
		lightPos[0] += 0.005f;
		lightPos[2] -= 0.005f;
		if(lightPos[0] >= 6.0f)
		{
			chk = true;
		}
	}else{
		lightPos[0] -= 0.005f;
		lightPos[2] += 0.005f;
		if(lightPos[0] <= 0.0f)
		{
			chk = false;
		}
	}

	glUniform3f(gPointLight_position[0], 0.0f, 1.0f, lightPos[0]);
	glUniform1f(gPointLight_constant[0], 1.0f);
	glUniform1f(gPointLight_linear[0], 0.09f);
	glUniform1f(gPointLight_quadratic[0], 0.032f);
	glUniform3f(gPointLight_ambient[0], 0.05f, 0.05f, 0.05f);
	glUniform3f(gPointLight_diffuse[0], PointDiffuse[0].x, PointDiffuse[0].y, PointDiffuse[0].z);
	glUniform3f(gPointLight_specular[0], 0.1f, 0.1f, 0.1f);

	glUniform3f(gPointLight_position[1], 3.0f, 1.0f, lightPos[1]);
	glUniform1f(gPointLight_constant[1], 1.0f);
	glUniform1f(gPointLight_linear[1], 0.09f);
	glUniform1f(gPointLight_quadratic[1], 0.032f);
	glUniform3f(gPointLight_ambient[1], 0.05f, 0.05f, 0.05f);
	glUniform3f(gPointLight_diffuse[1], PointDiffuse[1].x, PointDiffuse[1].y, PointDiffuse[1].z);
	glUniform3f(gPointLight_specular[1], 0.1f, 0.1f, 0.1f);

	glUniform3f(gPointLight_position[2], 3.0f, 1.0f, lightPos[2]);
	glUniform1f(gPointLight_constant[2], 1.0f);
	glUniform1f(gPointLight_linear[2], 0.09f);
	glUniform1f(gPointLight_quadratic[2], 0.032f);
	glUniform3f(gPointLight_ambient[2], 0.05f, 0.05f, 0.05f);
	glUniform3f(gPointLight_diffuse[2], PointDiffuse[2].x, PointDiffuse[2].y, PointDiffuse[2].z);
	glUniform3f(gPointLight_specular[2], 0.1f, 0.1f, 0.1f);

	//Spot Light
	glUniform3f(gSpotLight_position, camPos.x, camPos.y, camPos.z);
	glUniform3f(gSpotLight_direction, camDir.x, camDir.y, camDir.z);
	glUniform1f(gSpotLight_constant, 1.0f);
	glUniform1f(gSpotLight_linear, 0.09f);
	glUniform1f(gSpotLight_quadratic, 0.032f);
	glUniform1f(gSpotLight_cutOff, glm::cos(glm::radians(12.5f)));
	glUniform1f(gSpotLight_outerCutOff, glm::cos(glm::radians(17.5f)));
	glUniform3f(gSpotLight_ambient, 0.05f, 0.05f, 0.05f);
	glUniform3f(gSpotLight_diffuse, 2.0f, 2.0f, 2.0f);
	glUniform3f(gSpotLight_specular, 0.5f, 0.5f, 0.5f);

	glUniform3f(gViewPosition, camPos.x, camPos.y, camPos.z);

	glBindVertexArray(VAO);
		int lineX = 0;
		int lineZ = 0;
		for(int i = 0; i < 16; i++)
		{
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(lineX * 2.0f, 0.0f, lineZ * 2.0f));

			glUniformMatrix4fv(gModel[0], 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, 36);

			lineX++;
			if(lineX >= 4)
			{
				lineX = 0;
				lineZ++;
			}
		}
	glBindVertexArray(0);

	glUseProgram(program[1]);

	glUniformMatrix4fv(gProj[1], 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(gView[1], 1, GL_FALSE, glm::value_ptr(view));

	glBindVertexArray(lightVAO);
		for(int i = 0; i < 3; i++)
		{
			glUniform3f(gObjectColor[1],  PointDiffuse[i].x, PointDiffuse[i].y, PointDiffuse[i].z);

			glm::mat4 light;
			light = glm::translate(light, glm::vec3(i*3.0f, 1.0f, lightPos[i]));
			light = glm::scale(light, glm::vec3(0.3f, 0.3f, 0.3f));
			glUniformMatrix4fv(gModel[1], 1, GL_FALSE, glm::value_ptr(light));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	glBindVertexArray(0);
}

void Close()
{
	for(int i = 0; i < 2; i++)
	{
		glDetachShader(program[i], vertexShader[i]);
		glDeleteShader(vertexShader[i]);
		glDetachShader(program[i], fragmentShader[i]);
		glDeleteShader(fragmentShader[i]);
		glDeleteProgram(program[i]);
	}

	SDL_GL_DeleteContext(gl);
	SDL_DestroyWindow(window);
	window = NULL;
}

int main(int argc, char* argv[])
{
	Init();
	Start();

	bool quit = false;
	SDL_Event e;

	while(!quit)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
			{
				quit = true;
				break;
			}
			else if(e.type == SDL_KEYDOWN)
			{
				if(e.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
					break;
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Update();
		SDL_GL_SwapWindow(window);
	}

	Close();

	return 0;
}