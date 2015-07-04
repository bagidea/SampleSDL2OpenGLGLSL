#include <iostream>
#include <string>
#include <fstream>

#include <GL/glew.h>
#include <SDL.h>

using namespace std;

SDL_Window* window;
SDL_GLContext gl;

GLuint program;
GLuint vertexShader;
GLuint fragmentShader;

GLuint gScale;
GLfloat scale;
bool chk;

GLuint VAO, VBO;

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

void LoadShader(string _vertexShader, string _fragmentShader)
{
	program = glCreateProgram();
	vertexShader = CompileShader(LoadFile(_vertexShader), GL_VERTEX_SHADER);
	fragmentShader = CompileShader(LoadFile(_fragmentShader), GL_FRAGMENT_SHADER);

	if(vertexShader == NULL && fragmentShader == NULL)
	{
		cout << "Compile Shader Fail." << endl;
	}else{
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if(!success)
		{
			GLchar error[1024];
			glGetProgramInfoLog(program, 1024, NULL, error);
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
	window = SDL_CreateWindow("Shader Basic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	gl = SDL_GL_CreateContext(window);
	glewInit();
	SDL_GL_SetSwapInterval(0);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	LoadShader("shader/Shader2.vs", "shader/Shader2.fs");
}

void Start()
{
	gScale = glGetUniformLocation(program, "scale");
	scale = 0.5f;
	chk = false;

	GLfloat vec[] = {
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     	1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    	0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	
	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec), vec, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void Update()
{
	if(!chk)
	{
		scale += 0.0001f;
		if(scale >= 1.0f)
		{
			scale = 1.0f;
			chk = true;
		}
	}else{
		scale -= 0.0001f;
		if(scale <= 0.5f)
		{
			scale = 0.5f;
			chk = false;
		}
	}

	glUseProgram(program);
	glUniform1f(gScale, scale);
	glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

void Close()
{
	glDetachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);

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

		glClear(GL_COLOR_BUFFER_BIT);
		Update();
		SDL_GL_SwapWindow(window);
	}

	Close();

	return 0;
}