/* Wrapped by raw string so it can be #included into the code*/
R"(#version 300 es

precision highp float;

layout(location = 0) in vec2 position;
layout(location = 1) in int faceid;

uniform vec2 offset;

out vec3 texCoord;

void main() {
  float tx[6] = float[6](0.0f,0.0f,1.0f,0.0f,1.0f,1.0f);
  float ty[6] = float[6](0.0f,1.0f,0.0f,1.0f,1.0f,0.0f);
  int i = gl_VertexID % 6;

  gl_Position = vec4(position+offset, 0.0f, 1.0f);
  texCoord = vec3(tx[i], ty[i], float(faceid));
}
)"
