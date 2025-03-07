#version 460

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location = 0) out noperspective vec4 g_color;

void
main() {
  g_color = vec4(1, 0, 0, 1);
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  g_color = vec4(0, 1, 0, 1);
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();

  g_color = vec4(0, 0, 1, 1);
  gl_Position = gl_in[2].gl_Position;
  EmitVertex();

  EndPrimitive();
}
