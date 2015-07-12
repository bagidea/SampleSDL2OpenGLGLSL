#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MATH_PI 3.14159265359f

using namespace std;

struct Shader
{
	GLuint program;
	GLuint vertexShader;
	GLuint fragmentShader;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

struct Texture
{
	unsigned int id;
	string type;
	aiString path;
};

class Mesh
{
public:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vec, vector<GLuint> ind, vector<Texture> tex);

	void Draw(Shader _shader);
private:
	GLuint VAO, VBO, EBO;

	void SetupMesh();
};

Mesh::Mesh(vector<Vertex> vec, vector<GLuint> ind, vector<Texture> tex)
{
	vertices = vec;
	indices = ind;
	textures = tex;

	SetupMesh();
}

void Mesh::Draw(Shader _shader)
{
	GLuint diffuseNr = 1;
	GLuint specularNr = 1;

	for(GLuint i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0+i);

		stringstream ss;
		string number;
		string name = textures[i].type;

		if(name == "texture_diffuse")
		{
			ss << diffuseNr++;
		}
		else if(name == "texture_specular")
		{
			ss << specularNr++;
		}

		number = ss.str();

		glUniform1i(glGetUniformLocation(_shader.program, (name+number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	glUniform1f(glGetUniformLocation(_shader.program, "material.shininess"), 64.0f);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	for(GLuint i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Mesh::SetupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoord));

	glBindVertexArray(0);
}

unsigned int LoadImage(string path);

class Model
{
public:
	Model();

	void Load(string path);
	void Draw(Shader _shader);
private:
	vector<Mesh> meshes;
	string directory;

	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};

Model::Model(){}

void Model::Load(string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

void Model::Draw(Shader _shader)
{
	for(GLuint i = 0; i < meshes.size(); i++)
		meshes[i].Draw(_shader);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for(GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
		this->meshes.push_back(ProcessMesh(mesh, scene));			
	}

	for(GLuint i = 0; i < node->mNumChildren; i++)
	{
 		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	for(GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;

	 	vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
	    vertex.position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		if(mesh->mTextureCoords[0])
		{
			glm::vec2 vec;

			vec.x = mesh->mTextureCoords[0][i].x; 
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoord = vec;
		}else{
			vertex.texCoord = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}

	for(GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for(GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	 if(mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;
	
	for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		Texture texture;
		texture.id = LoadImage(directory+"/"+str.C_Str());
		texture.type = typeName;
		texture.path = str;
		textures.push_back(texture);
	}

	return textures;
}

SDL_Window* window;
SDL_GLContext gl;

Shader floorShader;
GLuint floorVAO, floorVBO;
unsigned int texFloor;

Shader grassShader;
GLuint grassVAO, grassVBO;
unsigned int texGrass;

Shader shader;
Model model;

Shader shaderSkybox;
GLuint skyVAO, skyVBO;
unsigned int cubeMapTex;

glm::vec3 camPos;
GLfloat camYaw, camPitch, lastX, lastY;

bool PolygonFill;

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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, formatTexture, img->w, img->h, 0, formatTexture, GL_UNSIGNED_BYTE, img->pixels);

		SDL_FreeSurface(img);
		img = NULL;
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return id;
}

unsigned int LoadCubeMap(vector<string> path)
{
	SDL_Surface* img;

	unsigned int id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for(int i = 0; i < path.size(); i++)
	{
		img = IMG_Load(path[i].c_str());

		if(img == NULL)
		{
			cout << "Can't Load Image : " << path[i] << endl;
		}else{
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

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, formatTexture, img->w, img->h, 0, formatTexture, GL_UNSIGNED_BYTE, img->pixels);
			SDL_FreeSurface(img);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

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
	window = SDL_CreateWindow("CubeMap and SkyBox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	gl = SDL_GL_CreateContext(window);
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	floorShader = LoadShader("shader/Shader4.vs", "shader/Shader4.fs");
	grassShader = LoadShader("shader/Shader4.vs", "shader/Shader4_2.fs");
	shader = LoadShader("shader/Shader11.vs", "shader/Shader11_2.fs");
	shaderSkybox = LoadShader("shader/Shader12.vs", "shader/Shader12.fs");
}

void Start()
{
	PolygonFill = true;

	model.Load("source/SpookyManor/Manor.obj");

	texFloor = LoadImage("source/grassFloor2.jpg");
	texGrass = LoadImage("source/grass.png");

	GLfloat vec[] = {
	    -0.5f, 0.0f, -0.5f,  0.0f, 1.0f,
	     0.5f, 0.0f, -0.5f,  1.0f, 1.0f,
	     0.5f, 0.0f,  0.5f,  1.0f, 0.0f,
	     0.5f, 0.0f,  0.5f,  1.0f, 0.0f,
	    -0.5f, 0.0f,  0.5f,  0.0f, 0.0f,
	    -0.5f, 0.0f, -0.5f,  0.0f, 1.0f,
	};

	vector<GLfloat> vec2(2400000);

	for(int i = 0; i < 2400000; i+=60)
	{
		GLfloat ranX = ((GLfloat)rand() / (GLfloat)RAND_MAX) * 30.0f;
		GLfloat ranZ = ((GLfloat)rand() / (GLfloat)RAND_MAX) * 30.0f;
		GLfloat size = ((GLfloat)rand() / (GLfloat)RAND_MAX) * 0.5f;

		vec2[i] = -size+ranX;    vec2[i+1] = size;  vec2[i+2] = 0.0f+ranZ;  vec2[i+3] = 1.0f;  vec2[i+4] = 1.0f;
        vec2[i+5] = -size+ranX;  vec2[i+6] = 0.0f;  vec2[i+7] = 0.0f+ranZ;  vec2[i+8] = 1.0f;  vec2[i+9] = 0.0f;
        vec2[i+10] = size+ranX;  vec2[i+11] = 0.0f; vec2[i+12] = 0.0f+ranZ; vec2[i+13] = 0.0f; vec2[i+14] = 0.0f;
        vec2[i+15] = -size+ranX; vec2[i+16] = size; vec2[i+17] = 0.0f+ranZ; vec2[i+18] = 1.0f; vec2[i+19] = 1.0f;
        vec2[i+20] = size+ranX;  vec2[i+21] = 0.0f; vec2[i+22] = 0.0f+ranZ; vec2[i+23] = 0.0f; vec2[i+24] = 0.0f;
        vec2[i+25] = size+ranX;  vec2[i+26] = size; vec2[i+27] = 0.0f+ranZ; vec2[i+28] = 0.0f; vec2[i+29] = 1.0f;

        vec2[i+30] = 0.0f+ranX; vec2[i+31] = size; vec2[i+32] = -size+ranZ; vec2[i+33] = 1.0f;  vec2[i+34] = 1.0f;
        vec2[i+35] = 0.0f+ranX; vec2[i+36] = 0.0f; vec2[i+37] = -size+ranZ; vec2[i+38] = 1.0f;  vec2[i+39] = 0.0f;
        vec2[i+40] = 0.0f+ranX; vec2[i+41] = 0.0f; vec2[i+42] = size+ranZ;  vec2[i+43] = 0.0f;  vec2[i+44] = 0.0f;
        vec2[i+45] = 0.0f+ranX; vec2[i+46] = size; vec2[i+47] = -size+ranZ; vec2[i+48] = 1.0f;  vec2[i+49] = 1.0f;
        vec2[i+50] = 0.0f+ranX; vec2[i+51] = 0.0f; vec2[i+52] = size+ranZ;  vec2[i+53] = 0.0f;  vec2[i+54] = 0.0f;
        vec2[i+55] = 0.0f+ranX; vec2[i+56] = size; vec2[i+57] = size+ranZ;  vec2[i+58] = 0.0f;  vec2[i+59] = 1.0f;
	}

	GLfloat skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
   
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);

	glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec), vec, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glGenVertexArrays(1, &grassVAO);
	glGenBuffers(1, &grassVBO);

	glBindVertexArray(grassVAO);
		glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
		glBufferData(GL_ARRAY_BUFFER, vec2.size(), &vec2[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);

    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    vector<string> cubeTex;
    cubeTex.push_back("source/sky2/right.jpg");
    cubeTex.push_back("source/sky2/left.jpg");
    cubeTex.push_back("source/sky2/up.jpg");
    cubeTex.push_back("source/sky2/down.jpg");
    cubeTex.push_back("source/sky2/back.jpg");
    cubeTex.push_back("source/sky2/front.jpg");

    cubeMapTex = LoadCubeMap(cubeTex);

	camPos = glm::vec3(0.0f, 1.0f, -3.0f);

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

	glm::mat4 _projection;
	glm::mat4 _view;
	glm::mat4 _model;

	_projection = glm::perspective(45.0f, 800.0f/600.0f, 0.1f, 100.0f);

	glm::vec3 camDir(cosf(camPitch) * sinf(camYaw), sinf(camPitch), cosf(camPitch) * cosf(camYaw));
	glm::vec3 camRight(sinf(camYaw - MATH_PI/2.0f), 0.0f, cosf(camYaw - MATH_PI/2.0f));
	glm::vec3 camUp = cross(camRight, camDir);

	_view = glm::lookAt(camPos, camPos + camDir, camUp);
	KeyboardController(camDir, camRight, 0.05f);

	//Floor
	glUseProgram(floorShader.program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFloor);
	glUniform1i(glGetUniformLocation(floorShader.program, "outTexture1"), 0);

	glBindVertexArray(floorVAO);
			glm::mat4 modelFloor;
			modelFloor = glm::scale(modelFloor, glm::vec3(30.0f, 0.1f, 30.0f));
			glUniformMatrix4fv(glGetUniformLocation(floorShader.program, "gWorld"), 1, GL_FALSE, glm::value_ptr(_projection * _view * modelFloor));
			glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	//Grass
	glUseProgram(grassShader.program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texGrass);
	glUniform1i(glGetUniformLocation(grassShader.program, "outTexture1"), 0);

	glBindVertexArray(grassVAO);
			glm::mat4 modelGrass;
			modelGrass = glm::translate(modelGrass, glm::vec3(-15.0f, 0.0f, -15.0f));
			glUniformMatrix4fv(glGetUniformLocation(grassShader.program, "gWorld"), 1, GL_FALSE, glm::value_ptr(_projection * _view * modelGrass));
			glDrawArrays(GL_TRIANGLES, 0, 480000);
	glBindVertexArray(0);

	//Model
	glUseProgram(shader.program);

	glUniformMatrix4fv(glGetUniformLocation(shader.program, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "view"), 1, GL_FALSE, glm::value_ptr(_view));

	glUniform3f(glGetUniformLocation(shader.program, "viewPos"), camPos.x, camPos.y, camPos.z);

	glUniform3f(glGetUniformLocation(shader.program, "directionalLight.direction"), 5.0f, 0.0f, -10.0f);
	glUniform3f(glGetUniformLocation(shader.program, "directionalLight.ambient"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader.program, "directionalLight.diffuse"), 0.5f, 0.5f, 0.2f);
	glUniform3f(glGetUniformLocation(shader.program, "directionalLight.specular"), 0.1f, 0.1f, 0.1f);

	_model = glm::translate(_model, glm::vec3(0.0f, 0.0f, 0.0f));
	_model = glm::scale(_model, glm::vec3(1.0f, 1.0f, 1.0f));
	_model = glm::rotate(_model, 180.0f*(MATH_PI/180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "model"), 1, GL_FALSE, glm::value_ptr(_model));
	model.Draw(shader);

	_model = glm::translate(_model, glm::vec3(3.0f, 0.0f, 0.0f));
	_model = glm::rotate(_model, -45.0f*(MATH_PI/180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "model"), 1, GL_FALSE, glm::value_ptr(_model));
	model.Draw(shader);

	//SkyBox
	glm::vec3 camDir2(cosf(-camPitch) * sinf(camYaw), sinf(-camPitch), cosf(-camPitch) * cosf(camYaw));
	_view = glm::lookAt(glm::vec3(0.0f), -camDir2, camUp);

	glDepthFunc(GL_LEQUAL);
    
    glUseProgram(shaderSkybox.program);

	glUniformMatrix4fv(glGetUniformLocation(shaderSkybox.program, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniformMatrix4fv(glGetUniformLocation(shaderSkybox.program, "view"), 1, GL_FALSE, glm::value_ptr(_view));

	glBindVertexArray(skyVAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderSkybox.program, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
}

void Close()
{
	glDetachShader(floorShader.program, floorShader.vertexShader);
	glDeleteShader(floorShader.vertexShader);
	glDetachShader(floorShader.program, floorShader.fragmentShader);
	glDeleteShader(floorShader.fragmentShader);
	glDeleteProgram(floorShader.program);

	glDetachShader(grassShader.program, grassShader.vertexShader);
	glDeleteShader(grassShader.vertexShader);
	glDetachShader(grassShader.program, grassShader.fragmentShader);
	glDeleteShader(grassShader.fragmentShader);
	glDeleteProgram(grassShader.program);

	glDetachShader(shader.program, shader.vertexShader);
	glDeleteShader(shader.vertexShader);
	glDetachShader(shader.program, shader.fragmentShader);
	glDeleteShader(shader.fragmentShader);
	glDeleteProgram(shader.program);
	
	glDetachShader(shaderSkybox.program, shaderSkybox.vertexShader);
	glDeleteShader(shaderSkybox.vertexShader);
	glDetachShader(shaderSkybox.program, shaderSkybox.fragmentShader);
	glDeleteShader(shaderSkybox.fragmentShader);
	glDeleteProgram(shaderSkybox.program);
	
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
				else if(e.key.keysym.sym == SDLK_SPACE)
				{
					if(PolygonFill)
					{
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						PolygonFill = false;
					}else{
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						PolygonFill = true;
					}
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