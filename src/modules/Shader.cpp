#include "Shader.h"

Shader::Shader() : handle(0) {}

Shader::~Shader()
{
	if (vs != 0)
	{
		glDeleteShader(vs);
		vs = 0;
	}
	if (fs != 0)
	{
		glDeleteShader(fs);
		fs = 0;
	}
	if (handle != 0)
	{
		glDeleteShader(handle);
		handle = 0;
	}
}

void Shader::load(const char *vertex_file_path, const char *fragment_file_path)
{

	// Create the shaders
	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else
	{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return exit(1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const *VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(vs, 1, &VertexSourcePointer, NULL);
	glCompileShader(vs);

	// Check Vertex Shader
	glGetShaderiv(vs, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(vs, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const *FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(fs, 1, &FragmentSourcePointer, NULL);
	glCompileShader(fs);

	// Check Fragment Shader
	glGetShaderiv(fs, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(fs, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	handle = glCreateProgram();
	glAttachShader(handle, vs);
	glAttachShader(handle, fs);
	glLinkProgram(handle);

	// Check the program
	glGetProgramiv(handle, GL_LINK_STATUS, &Result);
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(handle, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(handle, vs);
	glDetachShader(handle, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);
}

void Shader::use()
{
	glUseProgram(handle);
}

int Shader::getHandle()
{
	return handle;
}

void Shader::setUniform(const char *name, float x, float y, float z)
{
	GLint loc = getUniformLocation(name);
	glUniform3f(loc, x, y, z);
}

void Shader::setUniform(const char *name, const glm::vec3 &v)
{
	this->setUniform(name, v.x, v.y, v.z);
}

void Shader::setUniform(const char *name, const glm::vec4 &v)
{

	GLint loc = getUniformLocation(name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void Shader::setUniform(const char *name, const glm::vec2 &v)
{
	GLint loc = getUniformLocation(name);
	glUniform2f(loc, v.x, v.y);
}

void Shader::setUniform(const char *name, const glm::mat4 &m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

void Shader::setUniform(const char *name, const glm::mat3 &m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

void Shader::setUniform(const char *name, float val)
{
	GLint loc = getUniformLocation(name);
	glUniform1f(loc, val);
}

void Shader::setUniform(const char *name, int val)
{
	GLint loc = getUniformLocation(name);
	glUniform1i(loc, val);
}

void Shader::setUniform(const char *name, GLuint val)
{
	GLint loc = getUniformLocation(name);
	glUniform1ui(loc, val);
}

void Shader::setUniform(const char *name, bool val)
{
	int loc = getUniformLocation(name);
	glUniform1i(loc, val);
}

int Shader::getUniformLocation(const char *name)
{
	auto pos = uniformLocations.find(name);

	if (pos == uniformLocations.end())
	{
		uniformLocations[name] = glGetUniformLocation(handle, name);
	}

	return uniformLocations[name];
}
