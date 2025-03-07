#version 460

layout (location = 1) in vec3 v_nrm;

layout (location = 0) out vec4 f_color;

void
main() {
  f_color = vec4(v_nrm * 0.5 + 0.5, 1.);
}
