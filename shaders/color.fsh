#version 460

layout (location = 0) in noperspective vec4 g_color;

layout (location = 0) out vec4 f_color;

uniform float u_t;
uniform vec2 u_size;

void
main() {
  // assume width is greater...
  vec2 corrected = gl_FragCoord.xy / u_size.x;

  if (u_t < (corrected.x + corrected.y) / 2.) {
    discard;
  } 

  f_color = g_color;
}
