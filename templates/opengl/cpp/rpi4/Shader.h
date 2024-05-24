#pragma once

#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class Shader
{
  public:
    Shader(const char*, const char*);
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    ~Shader();
    GLuint GetId();
    GLuint GetAttribLocation(const char*);
    void Use();

  private:
    std::string FileToString(const char*);
    GLuint CompileVertexShader(const char*);
    GLuint CompileFragmentShader(const char*);
    GLuint LinkShaderProgram(GLuint, GLuint);

    GLuint id;
    static const unsigned int msgBuf = 512;
};

Shader::Shader(const char* vertexShaderFile, const char* fragmentShaderFile) : id(0)
{
  std::string vss = FileToString(vertexShaderFile);
  std::string fss = FileToString(fragmentShaderFile);
  const char *vertexSource = vss.c_str();
  const char *fragmentSource = fss.c_str();

  GLuint vertexShader = CompileVertexShader(vertexSource);
  GLuint fragmentShader = CompileFragmentShader(fragmentSource);
  id = LinkShaderProgram(vertexShader, fragmentShader);

  if(vertexShader) glDeleteShader(vertexShader);
  if(fragmentShader) glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
  if(id) glDeleteProgram(id);
}

std::string Shader::FileToString(const char* filename)
{
  std::ifstream file(filename);
  std::ostringstream oss;
  oss << file.rdbuf();
  std::string content = oss.str();
  return content;
}

GLuint Shader::CompileVertexShader(const char* vertexSource)
{
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  if(!vertexShader)
  {
    std::cout << "Unable to create vertex shader." << std::endl;
    return 0;
  }

  GLchar msg[msgBuf];
  GLint success;

  glShaderSource(vertexShader, 1, &vertexSource, nullptr);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    glGetShaderInfoLog(vertexShader, msgBuf, nullptr, msg);
    std::cout << "VERTEX SHADER FAILED TO COMPILE:\n" << msg << std::endl;
    return 0;
  }

  return vertexShader;
}

GLuint Shader::CompileFragmentShader(const char* fragmentSource)
{
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  if(!fragmentShader)
  {
    std::cout << "Unable to create fragment shader." << std::endl;
    return 0;
  }

  GLchar msg[msgBuf];
  GLint success;

  glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    glGetShaderInfoLog(fragmentShader, msgBuf, nullptr, msg);
    std::cout << "FRAGMENT SHADER FAILED TO COMPILE:\n" << msg << std::endl;
    return 0;
  }

  return fragmentShader;
}

GLuint Shader::LinkShaderProgram(GLuint vertexShader, GLuint fragmentShader)
{
  if(!vertexShader)
  {
    std::cout << "Invalid vertex shader." << std::endl;
    return 0;
  }

  if(!fragmentShader)
  {
    std::cout << "Invalid fragment shader." << std::endl;
    return 0;
  }

  GLuint shaderProgram = glCreateProgram();
  if(!shaderProgram)
  {
    std::cout << "Failed to create shader program." << std::endl;
    return 0;
  }

  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  GLchar msg[msgBuf];
  GLint success;

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if(!success)
  {
    glGetProgramInfoLog(shaderProgram, msgBuf, nullptr, msg);
    std::cout << "SHADER PROGRAM FAILED TO LINK:\n" << msg << std::endl;
    return 0;
  }

  return shaderProgram;
}

GLuint Shader::GetId()
{
  return id;
}

GLuint Shader::GetAttribLocation(const char* attribute)
{
  return glGetAttribLocation(id, attribute);
}

void Shader::Use()
{
  glUseProgram(id);
}
