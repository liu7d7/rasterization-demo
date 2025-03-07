#include "Windows.h"
#include <stdio.h>
#include "typedefs.h"
#include <GLFW/glfw3.h>
#include "lib/glad/glad.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_truetype.h"

__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

/*-- typedefs --*/

enum state {
  s_title
};

/*-- globals --*/

GLFWwindow *g_win;
int g_w = 2304, g_h = 2304;
enum state g_state = s_title;
struct camera camera;
struct shader lines, points, bary, norm, lit, lines_transition;
struct mesh mesh;
int g_n = 0;
float g_t = 0;

/*-- shaders --*/

struct uni {
  char const *name;
  int loc;
};

struct shader {
  int id;
  int n_unis;
  struct uni *unis;
};

void 
shader_new(
    struct shader *dst, 
    char const *vsh, 
    char const *fsh, 
    char const *gsh) {
  size_t len;
  char *src;
  char buf[1024];
  int vsh_id, fsh_id, gsh_id, status;
  int prog = gl_create_program();

#define one_shader(id, path, stage) \
  id = gl_create_shader(stage); \
  src = read_txt_file_len(path, &len); \
\
  gl_shader_source(id, 1, (char const *[]){src}, (int[]){len}); \
  gl_compile_shader(id); \
\
  gl_get_shaderiv(id, GL_COMPILE_STATUS, &status); \
  if (status == GL_FALSE) { \
    gl_get_shader_info_log(id, sizeof(buf), NULL, buf); \
    err("failed to compile shader %s because\n%s", path, buf); \
  } \
\
  free(src); \
  gl_attach_shader(prog, id);

  one_shader(vsh_id, vsh, GL_VERTEX_SHADER);
  one_shader(fsh_id, fsh, GL_FRAGMENT_SHADER);

  if (gsh) {
    one_shader(gsh_id, gsh, GL_GEOMETRY_SHADER);
  }

  gl_link_program(prog);

  gl_get_programiv(prog, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    gl_get_program_info_log(prog, sizeof(buf), NULL, buf);
    err("failed to link program\n%s", buf);
  }

  dst->id = prog;

  gl_get_programiv(prog, GL_ACTIVE_UNIFORMS, &status);
  dst->n_unis = status;
  dst->unis = malloc(sizeof(struct uni) * status);

  for (int i = 0; i < status; i++) {
    int a;
    gl_get_active_uniform(prog, i, sizeof(buf), &a, &a, &a, buf);
    dst->unis[i].name = strdup(buf);
    dst->unis[i].loc = gl_get_uniform_location(prog, buf);
  }

#undef one_shader
}

void
shader_set_m4f(struct shader *dst, char const *name, m4 *m) {
  for (int i = 0; i < dst->n_unis; i++) {
    if (strcmp(dst->unis[i].name, name) == 0) {
      gl_program_uniform_matrix_4fv(dst->id, dst->unis[i].loc, 1, GL_TRUE, &m->_00);
      break;
    }
  }
}

void
shader_set_1f(struct shader *dst, char const *name, float v) {
  for (int i = 0; i < dst->n_unis; i++) {
    if (strcmp(dst->unis[i].name, name) == 0) {
      gl_program_uniform_1f(dst->id, dst->unis[i].loc, v);
      break;
    }
  }
}

void
shader_set_3f(struct shader *dst, char const *name, v3 v) {
  for (int i = 0; i < dst->n_unis; i++) {
    if (strcmp(dst->unis[i].name, name) == 0) {
      gl_program_uniform_3f(dst->id, dst->unis[i].loc, v.x, v.y, v.z);
      break;
    }
  }
}

void
shader_set_2f(struct shader *dst, char const *name, v2 v) {
  for (int i = 0; i < dst->n_unis; i++) {
    if (strcmp(dst->unis[i].name, name) == 0) {
      gl_program_uniform_2f(dst->id, dst->unis[i].loc, v.x, v.y);
      break;
    }
  }
}

/*-- meshes --*/

enum attr {
  attr_1f,
  attr_2f,
  attr_3f,
  attr_4f
};

struct mesh_gpu {
  int va, vb, ib;
};

void
mesh_gpu_new(struct mesh_gpu *dst, int n, ...) {
  gl_create_vertex_arrays(1, &dst->va);
  gl_create_buffers(1, &dst->vb);
  gl_create_buffers(1, &dst->ib);

  enum attr attrs[n];

  int stride = 0;
  va_list va;
  va_start(va, n);
  for (int i = 0; i < n; i++) {
    attrs[i] = va_arg(va, enum attr);
    stride += (attrs[i] + 1) * sizeof(float);
  }

  va_end(va);

  gl_vertex_array_vertex_buffer(dst->va, 0, dst->vb, 0, stride);
  gl_vertex_array_element_buffer(dst->va, dst->ib);

  unsigned off = 0;
  for (int i = 0; i < n; i++) {
    gl_enable_vertex_array_attrib(dst->va, i);
    gl_vertex_array_attrib_format(dst->va, i, attrs[i] + 1, GL_FLOAT, GL_FALSE, off);
    gl_vertex_array_attrib_binding(dst->va, i, 0);
    off += (attrs[i] + 1) * sizeof(float);
  }
}

struct mesh {
  struct mesh_gpu g;
  int n_data, n_inds;
  void *data;
  int *inds;
};

struct vt {
  v3 p, n;
};

/**
 * precondition: dst.g is already populated
 */
void
mesh_from_obj(struct mesh *dst, char const *file, bool t) {
  int np = 0, cp = 4;
  v3 *p = malloc(sizeof(v3) * cp);

  int nn = 0, cn = 4;
  v3 *n = malloc(sizeof(v3) * cn);

  int nv = 0, cv = 4;
  struct vt *v = malloc(sizeof(struct vt) * cv);


  FILE *fp = fopen(file, "r");
  char buf[128];

  while (fgets(buf, sizeof(buf), fp)) {
    if (buf[0] == 'v') {
      if (buf[1] == ' ') {
        resize(p);
        sscanf(buf, "v %f %f %f", &p[np].x, &p[np].y, &p[np].z);
        np++;
      } else if (buf[1] == 'n') {
        resize(n);
        sscanf(buf, "vn %f %f %f", &n[nn].x, &n[nn].y, &n[nn].z);
        v3_norm(&n[nn]);
        nn++;
      } else if (buf[1] == 't') {
        // don't care
      }
    } else if (buf[0] == 'f') {
      int p0, n0, p1, n1, p2, n2;
      int dummy;
      if (t) {
        sscanf(buf, "f %d/%d/%d %d/%d/%d %d/%d/%d", &p0, &dummy, &n0, &p1, &dummy, &n1, &p2, &dummy, &n2);
      } else {
        sscanf(buf, "f %d//%d %d//%d %d//%d", &p0, &n0, &p1, &n1, &p2, &n2);
      }

      p0--, n0--, p1--, n1--, p2--, n2--;

      struct vt a, b, c;
      a.p = p[p0];
      b.p = p[p1];
      c.p = p[p2];

      a.n = n[n0];
      b.n = n[n1];
      c.n = n[n2];

      resize(v);
      v[nv++] = a;

      resize(v);
      v[nv++] = b;

      resize(v);
      v[nv++] = c;
    }
  }

  fclose(fp);

  dst->n_data = nv;
  dst->data = v;
  dst->n_inds = 0;
  dst->inds = 0;

  gl_named_buffer_data(dst->g.vb, nv * sizeof(struct vt), v, GL_STATIC_DRAW);
  free(p);
  free(n);
}

/*-- camera --*/

struct camera {
  v3 front, right, up, world_up, pos;
  float target_yaw, target_pitch, yaw, pitch;
  m4 v, p, vp;
};

void
camera_tick(struct camera *dst) {
  dst->yaw = lerp(dst->yaw, dst->target_yaw, 0.25);
  dst->pitch = lerp(dst->pitch, dst->target_pitch, 0.25);

  dst->front = (v3){
    sin(rad(dst->yaw)) * cos(rad(dst->pitch)),
    sin(rad(dst->pitch)),
    cos(rad(dst->yaw)) * cos(rad(dst->pitch)),
  };

  v3_norm(&dst->front);

  dst->right = v3_cross(dst->world_up, dst->front);

  dst->up = v3_mul(v3_cross(dst->right, dst->front), -1);

  dst->v = m4_look(dst->pos, dst->front, dst->up);
  dst->p = m4_persp(rad(45.f), (float)g_w / g_h, 0.001, 100.f);
  dst->vp = m4_mul(dst->v, dst->p);
}

void
camera_new(struct camera *dst) {
  dst->world_up = v3_uy;
  dst->pos = v3_zero;
  dst->yaw = 0;
  dst->pitch = 0;

  camera_tick(dst);
}

void
camera_move(struct camera *dst) {
  float f, r, u;
  f = ((float)(glfw_get_key(g_win, GLFW_KEY_W) == GLFW_PRESS) - (float)(glfw_get_key(g_win, GLFW_KEY_S) == GLFW_PRESS));
  r = (-(float)(glfw_get_key(g_win, GLFW_KEY_D) == GLFW_PRESS) + (float)(glfw_get_key(g_win, GLFW_KEY_A) == GLFW_PRESS));
  u = ((float)(glfw_get_key(g_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (float)(glfw_get_key(g_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS));

  v3 delta = v3_add(v3_add(v3_mul(dst->front, f), v3_mul(dst->right, r)), v3_mul(dst->world_up, u));
  if (v3_len(delta) > 0.0001) v3_norm(&delta);
  delta = v3_mul(delta, 0.05);
  dst->pos = v3_add(dst->pos, delta);
}

/*-- callbacks --*/

double last_xpos, last_ypos;
bool g_first = true;

void
cursor_pos_cb(GLFWwindow *win, double xpos, double ypos) {
  if (g_first) {
    last_xpos = xpos;
    last_ypos = ypos;
    g_first = false;
    return;
  }

  double dx = xpos - last_xpos, dy = ypos - last_ypos;

  camera.target_yaw -= (float)dx * 0.2;
  camera.target_pitch -= (float)dy * 0.2;

  last_xpos = xpos;
  last_ypos = ypos;

  if (camera.target_pitch >= 89.f) camera.target_pitch = 89.f;
  if (camera.target_pitch <= -89.f) camera.target_pitch = -89.f;
}

void
key_cb(GLFWwindow *win, int key, int scan, int act, int mods) {
  if (act == GLFW_PRESS && key == GLFW_KEY_N) {
    g_n++;
    g_t = 0;
    g_n %= 5;
  }

}

void
fb_size_cb(GLFWwindow *win, int w, int h) {
  gl_viewport(0, 0, w, h);
  g_w = w;
  g_h = h;
}

/*-- main --*/

int 
main() {
  if (!glfw_init()) {
    err("failed to init glfw %d", 3);
  }

  glfw_window_hint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfw_window_hint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfw_window_hint(GLFW_VERSION_MAJOR, 3);
  glfw_window_hint(GLFW_VERSION_MINOR, 3);
  glfw_window_hint(GLFW_SAMPLES, 4);
  glfw_window_hint(GLFW_RESIZABLE, GLFW_TRUE);
  glfw_window_hint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfw_window_hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfw_window_hint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  glfw_window_hint(GLFW_DECORATED, GLFW_TRUE);

  if ((g_win = glfw_create_window(g_w, g_h, "rasterization", NULL, NULL)) == NULL) {
    err("failed to create a GLFW window!");
  }

  glfw_make_context_current(g_win);

  if (!glad_load_gl_loader((GLADloadproc)glfw_get_proc_address)) {
    err("failed to load glad!");
  }

  glfw_set_input_mode(g_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfw_swap_interval(1);
  gl_enable(GL_MULTISAMPLE);

  glfw_set_cursor_pos_callback(g_win, cursor_pos_cb);
  glfw_set_key_callback(g_win, key_cb);
  glfw_set_framebuffer_size_callback(g_win, fb_size_cb);

  camera_new(&camera);
  camera.target_yaw = 180;
  camera.pos = (v3){0, 0, 4};

  shader_new(&points, "./shaders/noop.vsh", "./shaders/noop.fsh", "./shaders/points.gsh");
  shader_new(&lines, "./shaders/noop.vsh", "./shaders/noop.fsh", "./shaders/lines.gsh");
  shader_new(&lines_transition, "./shaders/noop.vsh", "./shaders/lines_transition.fsh", "./shaders/lines.gsh");
  shader_new(&bary, "./shaders/vp.vsh", "./shaders/color.fsh", "./shaders/bary.gsh");
  shader_new(&norm, "./shaders/norm.vsh", "./shaders/norm.fsh", NULL);
  shader_new(&lit, "./shaders/norm.vsh", "./shaders/lit.fsh", NULL);

  mesh_gpu_new(&mesh.g, 2, attr_3f, attr_3f);

  mesh_from_obj(&mesh, "./res/models/monkey.obj", true);

  struct mesh tree;
  mesh_gpu_new(&tree.g, 2, attr_3f, attr_3f);
  mesh_from_obj(&tree, "./res/models/tree.obj", true);

  gl_viewport(0, 0, g_w, g_h);
  
  gl_point_size(8);
  gl_line_width(2);
  gl_enable(GL_DEPTH_TEST);

  while (!glfw_window_should_close(g_win)) {
    gl_clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_move(&camera);
    camera_tick(&camera);

    g_t = lerp(g_t, 1, 0.05);

    switch (g_n) {
      case 4:
        gl_use_program(lit.id);
        shader_set_m4f(&lit, "u_vp", &camera.vp);
        shader_set_3f(&lit, "u_eye", camera.pos);
        shader_set_1f(&lines, "u_time", g_t);
  
        gl_bind_vertex_array(mesh.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, mesh.n_data);

        gl_bind_vertex_array(tree.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, tree.n_data);
        break;
      case 3:
        gl_use_program(norm.id);
        shader_set_m4f(&norm, "u_vp", &camera.vp);
        shader_set_1f(&lines, "u_time", g_t);
  
        gl_bind_vertex_array(mesh.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, mesh.n_data);

        gl_bind_vertex_array(tree.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, tree.n_data);
        break;
      case 2:
        gl_use_program(bary.id);
        shader_set_m4f(&bary, "u_vp", &camera.vp);
        shader_set_1f(&bary, "u_t", 1);
        shader_set_2f(&bary, "u_size", (v2){g_w, g_h});
  
        gl_bind_vertex_array(mesh.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, mesh.n_data);

        gl_bind_vertex_array(tree.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, tree.n_data);
        break;
      case 1:
        gl_use_program(lines.id);
        shader_set_m4f(&lines, "u_vp", &camera.vp);
        shader_set_1f(&lines, "u_time", g_t);
  
        gl_bind_vertex_array(mesh.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, mesh.n_data);

        gl_bind_vertex_array(tree.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, tree.n_data);
        break;
      case 0:
        gl_use_program(points.id);
        shader_set_m4f(&points, "u_vp", &camera.vp);

        gl_bind_vertex_array(mesh.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, mesh.n_data);

        gl_bind_vertex_array(tree.g.va);
        gl_draw_arrays(GL_TRIANGLES, 0, tree.n_data);
        break;
    }

    glfw_poll_events();
    glfw_swap_buffers(g_win);
  }

  return 0;
}
