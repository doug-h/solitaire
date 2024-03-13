/* Wrapped by raw string so it can be #included into the code*/
R"(#version 300 es

precision highp float;
precision lowp sampler2DArray;

in vec3 texCoord;

uniform sampler2DArray face_textures;

out vec4 colour_out;


void main() {
  colour_out = texture(face_textures, texCoord);
  if(colour_out.a < 1.0f) { discard; }
}
)"
