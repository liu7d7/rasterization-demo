#version 460

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

uniform float u_time;
uniform mat4 u_vp;

void main() {
  gl_Position = gl_in[0].gl_Position * u_vp;
  EmitVertex();

  gl_Position = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, u_time) * u_vp;
  EmitVertex();

  EndPrimitive();

  gl_Position = gl_in[1].gl_Position * u_vp;
  EmitVertex();

  gl_Position = mix(gl_in[1].gl_Position, gl_in[2].gl_Position, u_time) * u_vp;
  EmitVertex();

  EndPrimitive();

  gl_Position = gl_in[2].gl_Position * u_vp;
  EmitVertex();

  gl_Position = mix(gl_in[2].gl_Position, gl_in[0].gl_Position, u_time) * u_vp;
  EmitVertex();

  EndPrimitive();
}
