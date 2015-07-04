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

GLuint program[1];
GLuint vertexShader[1];
GLuint fragmentShader[1];

GLuint VAO, VBO;

GLuint gProj;
GLuint gView;
GLuint gModel;

GLuint gAmbientStrength;
GLuint gSpecularStrength;
GLuint gTexture1;
GLuint gTexture2;

GLuint gObjectColor;
GLuint gLightColor;
GLuint gLightDirection;
GLuint gShininess;

GLfloat num;
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
	window = SDL_CreateWindow("Directional Light", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	gl = SDL_GL_CreateContext(window);
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

	LoadShader("source/shader/Shader5.vs", "source/shader/Shader6.fs", 0);
}

void Start()
{
	gProj = glGetUniformLocation(program[0], "gProjection");
	gView = glGetUniformLocation(program[0], "gView");
	gModel = glGetUniformLocation(program[0], "gModel");

	gAmbientStrength = glGetUniformLocation(program[0], "ambientStrength");
	gSpecularStrength = glGetUniformLocation(program[0], "specularStrength");
	gTexture1 = glGetUniformLocation(program[0], "outTexture1");
	gTexture2 = glGetUniformLocation(program[0], "outSpecular");

	gObjectColor = glGetUniformLocation(program[0], "objectColor");
	gLightColor = glGetUniformLocation(program[0], "lightColor");
	gLightDirection = glGetUniformLocation(program[0], "lightDirection");
	gShininess = glGetUniformLocation(program[0], "shininess");

	num = 0.0f;
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

	num += 0.001f;

	glm::mat4 projection;
	glm::mat4 view;

	projection = glm::perspective(45.0f, 800.0f/600.0f, 0.1f, 100.0f);
	
	glm::vec3 camDir(cosf(camPitch) * sinf(camYaw), sinf(camPitch), cosf(camPitch) * cosf(camYaw));
	glm::vec3 camRight(sinf(camYaw - MATH_PI/2.0f), 0.0f, cosf(camYaw - MATH_PI/2.0f));
	glm::vec3 camUp = cross(camRight, camDir);

	view = glm::lookAt(camPos, camPos + camDir, camUp);
	KeyboardController(camDir, camRight, 0.01f);

	glUseProgram(program[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(gTexture1, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex2);
	glUniform1i(gTexture2, 1);

	glUniformMatrix4fv(gProj, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(gView, 1, GL_FALSE, glm::value_ptr(view));
	
	glUniform1f(gAmbientStrength, 0.2f);
	glUniform1f(gSpecularStrength, 1.0f);
	glUniform3f(gObjectColor, 0.8f, 0.8f, 0.8f);
	glUniform3f(gLightColor, 1.0f, 1.0f, 1.0f);
	glUniform3f(gLightDirection, -10.0f, -5.0f, 10.0f);
	glUniform1f(gShininess, 64.0f);

	glBindVertexArray(VAO);
		int lineX = 0;
		int lineZ = 0;
		for(int i = 0; i < 16; i++)
		{
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(lineX * 2.0f, 0.0f, lineZ * 2.0f));

			glUniformMatrix4fv(gModel, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, 36);

			lineX++;
			if(lineX >= 4)
			{
				lineX = 0;
				lineZ++;
			}
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