#define GLFW_INCLUDE_ES31
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"

static const GLuint WIDTH = 480;
static const GLuint HEIGHT = 360;

static const GLfloat vertices[] = {
  0.0f,  0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  -0.5f, -0.5f, 0.0f,
};

inline void InitContext()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
}

int main() 
{
  InitContext();
  GLFWwindow* window = nullptr;
  window = glfwCreateWindow(WIDTH, HEIGHT, "GL Test", nullptr, nullptr);
  if(!window)
  { 
    std::cout << "Window unable to be created." << std::endl;
    return 1;
  }
  glfwMakeContextCurrent(window);

  Shader* shader = new Shader("vertex.shader", "fragment.shader");
  if(!shader->GetId())
  {
    std::cout << "Shader program invalid." << std::endl;
    return 1;
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glViewport(0, 0, WIDTH, HEIGHT);
  GLuint vbo, vao;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLint posLoc = shader->GetAttribLocation("position");
  if(posLoc == -1)
  {
    std::cout << "Failed to get attribute location." << std::endl;
    return 1;
  }
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(posLoc);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind

  while(!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);
    shader->Use();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);
  }

  delete shader;
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glfwTerminate();

  return 0;
}
