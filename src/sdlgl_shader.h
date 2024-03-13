#pragma once

#include <GLES3/gl3.h>

#include <cstdio>

GLuint sdlgl_create_shader(const char *vs_code, const char *fs_code) {
  GLuint vertex, fragment;
  vertex   = glCreateShader(GL_VERTEX_SHADER);
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vertex, 1, &vs_code, NULL);
  glShaderSource(fragment, 1, &fs_code, NULL);

  int  success;
  char infoLog[512];
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    printf("Vertex shader compilation failed: \n %.*s\n" , 512, infoLog);
  }
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    printf("Fragment shader compilation failed: \n %.*s\n" , 512, infoLog);
  }

  GLuint ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);

  glLinkProgram(ID);
  // Catch linking errors
  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    printf("Shader linking failed: \n %.*s\n" , 512, infoLog);
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  return ID;
}
