#include <iostream>
#include <string>
#include <fstream>
#include <vector>

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

GLuint program;
GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO, VBO;

GLuint gProj;
GLuint gView;
GLuint gModel;

GLuint gTexture1;

unsigned int tex;

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
	window = SDL_CreateWindow("Lab1 - Create Box 100,000 Vertex", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	gl = SDL_GL_CreateContext(window);
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	LoadShader("shader/Shader4_2.vs", "shader/Shader4.fs");
}

void Start()
{
	gProj = glGetUniformLocation(program, "gProjection");
	gView = glGetUniformLocation(program, "gView");
	gModel = glGetUniformLocation(program, "gModel");
	
	gTexture1 = glGetUniformLocation(program, "outTexture1");

	tex = LoadImage("source/box.png");

	vector<GLfloat> vec(18000000);
	
	int a1 = 0;
	int a2 = 0;
	int a3 = 0;

	for(int i = 0; i < 18000000; i+=180)
	{
		GLfloat DivX = (GLfloat)a1 * 1.5f;
		GLfloat DivZ = (GLfloat)a2 * 1.5f;
		GLfloat DivY = (GLfloat)a3 * 1.5f;
		a1++;
		if(a1 >= 25)
		{
			a1 = 0;
			a2++;
			if(a2 >= 25)
			{
				a2 = 0;
				a3++;
			}
		}

		vec[i] = 0.0f + DivX;   vec[i+1] = 0.0f + DivY; vec[i+2] = 0.0f + DivZ; vec[i+3] = 0.0f; vec[i+4] = 0.0f;
	    vec[i+5] = 1.0f + DivX; vec[i+6] = 0.0f + DivY; vec[i+7] = 0.0f + DivZ; vec[i+8] = 1.0f; vec[i+9] = 0.0f;
	    vec[i+10] = 1.0f + DivX;vec[i+11] = 1.0f + DivY;vec[i+12] = 0.0f + DivZ;vec[i+13] = 1.0f;vec[i+14] = 1.0f;
	    vec[i+15] = 1.0f + DivX;vec[i+16] = 1.0f + DivY;vec[i+17] = 0.0f + DivZ;vec[i+18] = 1.0f;vec[i+19] = 1.0f;
	    vec[i+20] = 0.0f + DivX;vec[i+21] = 1.0f + DivY;vec[i+22] = 0.0f + DivZ;vec[i+23] = 0.0f;vec[i+24] = 1.0f;
	   	vec[i+25] = 0.0f + DivX;vec[i+26] = 0.0f + DivY;vec[i+27] = 0.0f + DivZ;vec[i+28] = 0.0f;vec[i+29] = 0.0f;

	   	vec[i+30] = 0.0f + DivX;vec[i+31] = 0.0f + DivY;vec[i+32] = 1.0f + DivZ;vec[i+33] = 0.0f;vec[i+34] = 0.0f;
	    vec[i+35] = 1.0f + DivX;vec[i+36] = 0.0f + DivY;vec[i+37] = 1.0f + DivZ;vec[i+38] = 1.0f;vec[i+39] = 0.0f;
	    vec[i+40] = 1.0f + DivX;vec[i+41] = 1.0f + DivY;vec[i+42] = 1.0f + DivZ;vec[i+43] = 1.0f;vec[i+44] = 1.0f;
	    vec[i+45] = 1.0f + DivX;vec[i+46] = 1.0f + DivY;vec[i+47] = 1.0f + DivZ;vec[i+48] = 1.0f;vec[i+49] = 1.0f;
	    vec[i+50] = 0.0f + DivX;vec[i+51] = 1.0f + DivY;vec[i+52] = 1.0f + DivZ;vec[i+53] = 0.0f;vec[i+54] = 1.0f;
	   	vec[i+55] = 0.0f + DivX;vec[i+56] = 0.0f + DivY;vec[i+57] = 1.0f + DivZ;vec[i+58] = 0.0f;vec[i+59] = 0.0f;

	   	vec[i+60] = 0.0f + DivX;vec[i+61] = 1.0f + DivY;vec[i+62] = 1.0f + DivZ;vec[i+63] = 1.0f;vec[i+64] = 0.0f;
	    vec[i+65] = 0.0f + DivX;vec[i+66] = 1.0f + DivY;vec[i+67] = 0.0f + DivZ;vec[i+68] = 1.0f;vec[i+69] = 1.0f;
	    vec[i+70] = 0.0f + DivX;vec[i+71] = 0.0f + DivY;vec[i+72] = 0.0f + DivZ;vec[i+73] = 0.0f;vec[i+74] = 1.0f;
	    vec[i+75] = 0.0f + DivX;vec[i+76] = 0.0f + DivY;vec[i+77] = 0.0f + DivZ;vec[i+78] = 0.0f;vec[i+79] = 1.0f;
	    vec[i+80] = 0.0f + DivX;vec[i+81] = 0.0f + DivY;vec[i+82] = 1.0f + DivZ;vec[i+83] = 0.0f;vec[i+84] = 0.0f;
	   	vec[i+85] = 0.0f + DivX;vec[i+86] = 1.0f + DivY;vec[i+87] = 1.0f + DivZ;vec[i+88] = 1.0f;vec[i+89] = 0.0f;

	   	vec[i+90] = 1.0f + DivX; vec[i+91] = 1.0f + DivY; vec[i+92] = 1.0f + DivZ; vec[i+93] = 1.0f; vec[i+94] = 0.0f;
	    vec[i+95] = 1.0f + DivX; vec[i+96] = 1.0f + DivY; vec[i+97] = 0.0f + DivZ; vec[i+98] = 1.0f; vec[i+99] = 1.0f;
	    vec[i+100] = 1.0f + DivX;vec[i+101] = 0.0f + DivY;vec[i+102] = 0.0f + DivZ;vec[i+103] = 0.0f;vec[i+104] = 1.0f;
	    vec[i+105] = 1.0f + DivX;vec[i+106] = 0.0f + DivY;vec[i+107] = 0.0f + DivZ;vec[i+108] = 0.0f;vec[i+109] = 1.0f;
	    vec[i+110] = 1.0f + DivX;vec[i+111] = 0.0f + DivY;vec[i+112] = 1.0f + DivZ;vec[i+113] = 0.0f;vec[i+114] = 0.0f;
	   	vec[i+115] = 1.0f + DivX;vec[i+116] = 1.0f + DivY;vec[i+117] = 1.0f + DivZ;vec[i+118] = 1.0f;vec[i+119] = 0.0f;

	   	vec[i+120] = 0.0f + DivX;vec[i+121] = 0.0f + DivY;vec[i+122] = 0.0f + DivZ;vec[i+123] = 0.0f;vec[i+124] = 1.0f;
	    vec[i+125] = 1.0f + DivX;vec[i+126] = 0.0f + DivY;vec[i+127] = 0.0f + DivZ;vec[i+128] = 1.0f;vec[i+129] = 1.0f;
	    vec[i+130] = 1.0f + DivX;vec[i+131] = 0.0f + DivY;vec[i+132] = 1.0f + DivZ;vec[i+133] = 1.0f;vec[i+134] = 0.0f;
	    vec[i+135] = 1.0f + DivX;vec[i+136] = 0.0f + DivY;vec[i+137] = 1.0f + DivZ;vec[i+138] = 1.0f;vec[i+139] = 0.0f;
	    vec[i+140] = 0.0f + DivX;vec[i+141] = 0.0f + DivY;vec[i+142] = 1.0f + DivZ;vec[i+143] = 0.0f;vec[i+144] = 0.0f;
	   	vec[i+145] = 0.0f + DivX;vec[i+146] = 0.0f + DivY;vec[i+147] = 0.0f + DivZ;vec[i+148] = 0.0f;vec[i+149] = 1.0f;

	   	vec[i+150] = 0.0f + DivX;vec[i+151] = 1.0f + DivY;vec[i+152] = 0.0f + DivZ;vec[i+153] = 0.0f;vec[i+154] = 1.0f;
	    vec[i+155] = 1.0f + DivX;vec[i+156] = 1.0f + DivY;vec[i+157] = 0.0f + DivZ;vec[i+158] = 1.0f;vec[i+159] = 1.0f;
	    vec[i+160] = 1.0f + DivX;vec[i+161] = 1.0f + DivY;vec[i+162] = 1.0f + DivZ;vec[i+163] = 1.0f;vec[i+164] = 0.0f;
	    vec[i+165] = 1.0f + DivX;vec[i+166] = 1.0f + DivY;vec[i+167] = 1.0f + DivZ;vec[i+168] = 1.0f;vec[i+169] = 0.0f;
	    vec[i+170] = 0.0f + DivX;vec[i+171] = 1.0f + DivY;vec[i+172] = 1.0f + DivZ;vec[i+173] = 0.0f;vec[i+174] = 0.0f;
	   	vec[i+175] = 0.0f + DivX;vec[i+176] = 1.0f + DivY;vec[i+177] = 0.0f + DivZ;vec[i+178] = 0.0f;vec[i+179] = 1.0f;
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vec.size(), &vec[0], GL_STATIC_DRAW);
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
	glm::mat4 model;

	projection = glm::perspective(45.0f, 800.0f/600.0f, 1.0f, 500.0f);

	glm::vec3 camDir(cosf(camPitch) * sinf(camYaw), sinf(camPitch), cosf(camPitch) * cosf(camYaw));
	glm::vec3 camRight(sinf(camYaw - MATH_PI/2.0f), 0.0f, cosf(camYaw - MATH_PI/2.0f));
	glm::vec3 camUp = cross(camRight, camDir);

	view = glm::lookAt(camPos, camPos + camDir, camUp);
	KeyboardController(camDir, camRight, 0.5f);

	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(gTexture1, 0);

	glUniformMatrix4fv(gProj, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(gView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(gModel, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3600000);
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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Update();
		SDL_GL_SwapWindow(window);
	}

	Close();

	return 0;
}