#version 460

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_nrm;

layout (location = 0) out vec4 f_color;

vec3 light_dir = normalize(vec3(1, 2, 1));

uniform vec3 u_eye;

void
main() {
  vec3 ambient = vec3(0.3, 0.2, 0);

  vec3 diff = vec3(max(dot(v_nrm, light_dir), 0.)) * vec3(1., 0.84, 0.);

  vec3 view_dir = normalize(u_eye - v_pos);
  vec3 reflect_dir = reflect(-light_dir, v_nrm);

  vec3 spec = vec3(pow(max(dot(view_dir, reflect_dir), 0.0), 32) * 0.5);

  vec3 result = (ambient + diff + spec);
  f_color = vec4(result, 1.0);
}
