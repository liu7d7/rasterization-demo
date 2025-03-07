#version 460

layout (triangles) in;
layout (points, max_vertices = 3) out;

uniform mat4 u_vp;

void main() {
  gl_Position = gl_in[0].gl_Position * u_vp;
  EmitVertex();

  gl_Position = gl_in[1].gl_Position * u_vp;
  EmitVertex();

  gl_Position = gl_in[2].gl_Position * u_vp;
  EmitVertex();

  EndPrimitive();
}
