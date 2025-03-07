#version 460

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;

layout (location = 0) out vec3 v_pos;
layout (location = 1) out vec3 v_nrm;

uniform mat4 u_vp;

void
main() {
  gl_Position = vec4(pos, 1.) * u_vp;
  v_pos = pos;
  v_nrm = nrm;
}
