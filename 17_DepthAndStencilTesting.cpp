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

struct Shader
{
	GLuint program;
	GLuint vertexShader;
	GLuint fragmentShader;
};

SDL_Window* window;
SDL_GLContext gl;

Shader shader;
Shader shader2;

GLuint VAO, VBO;

GLuint gTransformation;
GLuint gTransformation2;

GLuint gTexture1;

unsigned int tex;
unsigned int tex2;

glm::vec3 posBox[5];

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

Shader LoadShader(string _vertexShader, string _fragmentShader)
{
	Shader _shader;
	_shader.program = glCreateProgram();
	_shader.vertexShader = CompileShader(LoadFile(_vertexShader), GL_VERTEX_SHADER);
	_shader.fragmentShader = CompileShader(LoadFile(_fragmentShader), GL_FRAGMENT_SHADER);

	if(_shader.vertexShader == NULL && _shader.fragmentShader == NULL)
	{
		cout << "Compile Shader Fail." << endl;
	}else{
		glAttachShader(_shader.program, _shader.vertexShader);
		glAttachShader(_shader.program, _shader.fragmentShader);
		glLinkProgram(_shader.program);

		GLint success;
		glGetProgramiv(_shader.program, GL_LINK_STATUS, &success);
		if(!success)
		{
			GLchar error[1024];
			glGetProgramInfoLog(_shader.program, 1024, NULL, error);
			cout << error << endl;	
		}
	}

	return _shader;
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
	window = SDL_CreateWindow("Depth Testing and Stencil Testing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	gl = SDL_GL_CreateContext(window);
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	shader = LoadShader("shader/Shader4.vs", "shader/Shader4.fs");
	shader2 = LoadShader("shader/Shader2_2.vs", "shader/Shader2_2.fs");
}

void Start()
{
	gTransformation = glGetUniformLocation(shader.program, "gWorld");
	gTransformation2 = glGetUniformLocation(shader2.program, "gWorld");
	
	gTexture1 = glGetUniformLocation(shader.program, "outTexture1");

	tex = LoadImage("source/box.png");
	tex2 = LoadImage("source/image.jpg");

	posBox[0] = glm::vec3( 0.0f,  0.0f,  0.0f);
  	posBox[1] = glm::vec3( 2.0f,  1.0f, 3.0f); 
  	posBox[2] = glm::vec3(-1.5f, -1.2f, 2.5f); 
	posBox[3] = glm::vec3(-3.8f, -1.0f, 8.3f);  
	posBox[4] = glm::vec3( 2.4f, -0.4f, 3.5f);  

	GLfloat vec[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec), vec, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	camPos = glm::vec3(0.0f, 0.0f, -3.0f);

	camYaw = 0.0f;
	camPitch = 0.0f;
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
	KeyboardController(camDir, camRight, 0.005f);

	glStencilMask(0x00);

	glUseProgram(shader.program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex2);
	glUniform1i(gTexture1, 0);

	glBindVertexArray(VAO);
			glm::mat4 modelFloor;
			modelFloor = glm::translate(modelFloor, glm::vec3( 0.0f,  -4.0f,  0.0f));
			modelFloor = glm::scale(modelFloor, glm::vec3(50.0f, 0.1f, 50.0f));
			glUniformMatrix4fv(gTransformation, 1, GL_FALSE, glm::value_ptr(projection * view * modelFloor));
			glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);	

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(gTexture1, 0);

	glBindVertexArray(VAO);
		for(int i = 0; i < 5; i++)
		{
			glm::mat4 model;
			model = glm::translate(model, posBox[i]);
			model = glm::rotate(model, 20.0f * i, glm::vec3(1.0f, 0.3f, 0.5f));

			glUniformMatrix4fv(gTransformation, 1, GL_FALSE, glm::value_ptr(projection * view * model));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	glBindVertexArray(0);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(shader2.program);

	glBindVertexArray(VAO);
		for(int i = 0; i < 5; i++)
		{
			glm::mat4 model;
			model = glm::translate(model, posBox[i]);
			model = glm::rotate(model, 20.0f * i, glm::vec3(1.0f, 0.3f, 0.5f));
			model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));

			glUniformMatrix4fv(gTransformation2, 1, GL_FALSE, glm::value_ptr(projection * view * model));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	glBindVertexArray(0);

	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
}

void Close()
{
	glDetachShader(shader.program, shader.vertexShader);
	glDeleteShader(shader.vertexShader);
	glDetachShader(shader.program, shader.fragmentShader);
	glDeleteShader(shader.fragmentShader);
	glDeleteProgram(shader.program);

	glDetachShader(shader2.program, shader2.vertexShader);
	glDeleteShader(shader2.vertexShader);
	glDetachShader(shader2.program, shader2.fragmentShader);
	glDeleteShader(shader2.fragmentShader);
	glDeleteProgram(shader2.program);

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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		Update();
		SDL_GL_SwapWindow(window);
	}

	Close();

	return 0;
}