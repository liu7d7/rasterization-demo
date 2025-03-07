#version 460

layout (location = 0) in vec3 pos;

uniform mat4 u_vp;

void
main() {
  gl_Position = vec4(pos, 1.) * u_vp;
}
