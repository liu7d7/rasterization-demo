#pragma once

#include "tgmath.h"
#include "stdint.h"

#define err(fmt, ...) do { \
  fprintf(stderr, "%s:%s:%d :: ", __FILE__, __func__, __LINE__); \
  fprintf(stderr, fmt, ##__VA_ARGS__); \
  exit(-1); \
} while (false);

#define resize(a) do { \
  if (n##a < c##a) break; \
  typeof(*a) *new_ptr = realloc(a, c##a * 2 * sizeof(*a)); \
  a = new_ptr; \
  c##a *= 2; \
} while (false);

#define splat_arr(a) a.data, a.count, a.cap

static char *read_txt_file_len(char const *path, size_t *len)
{
  FILE *file = fopen(path, "r");
  char *result = 0;
  if (!file)
  {
    err("read_txt_file: failed to read text file at %s!", path);
  }

  size_t file_size = 0;
  fseek(file, 0L, SEEK_END);
  file_size = (size_t)ftell(file);
  fseek(file, 0L, SEEK_SET);
  result = (char *)calloc(file_size + 1, sizeof(char));
  if (!fread(result, sizeof(char), file_size, file))
  {
    err("read_txt_file: failed to read from text file!");
  }

  result[file_size] = '\0';
  fclose(file);

  if (len) *len = file_size;
  return result;
}

static uint32_t hash_murmur3(const void *data, size_t nbytes)
{
  if (data == NULL || nbytes == 0)
    return 0;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  const int nblocks = nbytes / 4;
  const uint32_t *blocks = (const uint32_t *)(data);
  const uint8_t *tail = (const uint8_t *)(data + (nblocks * 4));

  uint32_t h = 0;

  int i;
  uint32_t k;
  for (i = 0; i < nblocks; i++)
  {
    k = blocks[i];

    k *= c1;
    k = (k << 15) | (k >> (32 - 15));
    k *= c2;

    h ^= k;
    h = (h << 13) | (h >> (32 - 13));
    h = (h * 5) + 0xe6546b64;
  }

  k = 0;
  switch (nbytes & 3)
  {
    case 3:k ^= tail[2] << 16;
    case 2:k ^= tail[1] << 8;
    case 1:k ^= tail[0];
      k *= c1;
      k = (k << 15) | (k >> (32 - 15));
      k *= c2;
      h ^= k;
  };

  h ^= nbytes;

  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

typedef union v2
{
  struct
  {
    float x, y;
  };

  float v[2];
} v2;

static const v2 v2_ux = (v2){.x = 1};
static const v2 v2_uy = (v2){.y = 1};
static const v2 v2_zero = (v2){0};
static const v2 v2_one = (v2){1, 1};


inline static v2 v2_max(v2 lhs, v2 rhs)
{
  return (v2){max(lhs.x, rhs.x), max(lhs.y, rhs.y)};
}

[[gnu::always_inline]]

inline static v2 v2_min(v2 lhs, v2 rhs)
{
  return (v2){min(lhs.x, rhs.x), min(lhs.y, rhs.y)};
}

[[gnu::always_inline]]

inline static v2 v2_add(v2 lhs, v2 rhs)
{
  return (v2){lhs.x + rhs.x, lhs.y + rhs.y};
}

[[gnu::always_inline]]

inline static v2 v2_sub(v2 lhs, v2 rhs)
{
  return (v2){lhs.x - rhs.x, lhs.y - rhs.y};
}

[[gnu::always_inline]]

inline static v2 v2_mul(v2 lhs, float scalar)
{
  return (v2){lhs.x * scalar, lhs.y * scalar};
}

[[gnu::always_inline]]

inline static v2 v2_div(v2 lhs, float scalar)
{
  return (v2){lhs.x / scalar, lhs.y / scalar};
}

[[gnu::always_inline]]

inline static float v2_dot(v2 lhs, v2 rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

[[gnu::always_inline]]

inline static float v2_len(v2 a)
{
  return sqrtf(v2_dot(a, a));
}

[[gnu::always_inline]]

inline static float v2_dist(v2 lhs, v2 rhs)
{
  v2 delta = v2_sub(lhs, rhs);
  return v2_len(delta);
}

[[gnu::always_inline]]

inline static v2 v2_norm(v2 v)
{
  float len = v2_len(v);
  return (v2){.x = v.x / len, .y = v.y / len};
}

typedef union iv2
{
  struct
  {
    int x, y;
  };

  int v[2];
} iv2;

static const iv2 iv2_ux = (iv2){.x = 1};
static const iv2 iv2_uy = (iv2){.y = 1};
static const iv2 iv2_zero = (iv2){0};

[[gnu::always_inline]]

inline static iv2 iv2_max(iv2 lhs, iv2 rhs)
{
  return (iv2){max(lhs.x, rhs.x), max(lhs.y, rhs.y)};
}

[[gnu::always_inline]]

inline static iv2 iv2_min(iv2 lhs, iv2 rhs)
{
  return (iv2){min(lhs.x, rhs.x), min(lhs.y, rhs.y)};
}

[[gnu::always_inline]]

inline static iv2 iv2_add(iv2 lhs, iv2 rhs)
{
  return (iv2){lhs.x + rhs.x, lhs.y + rhs.y};
}

[[gnu::always_inline]]

inline static bool iv2_eq(iv2 lhs, iv2 rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

[[gnu::always_inline]]

inline static iv2 iv2_sub(iv2 lhs, iv2 rhs)
{
  return (iv2){lhs.x - rhs.x, lhs.y - rhs.y};
}

[[gnu::always_inline]]

inline static iv2 iv2_mul(iv2 lhs, int scalar)
{
  return (iv2){lhs.x * scalar, lhs.y * scalar};
}

[[gnu::always_inline]]

inline static iv2 iv2_div(iv2 lhs, int scalar)
{
  return (iv2){lhs.x / scalar, lhs.y / scalar};
}

[[gnu::always_inline]]

inline static int iv2_dot(iv2 lhs, iv2 rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

[[gnu::always_inline]]

inline static float iv2_len(iv2 a)
{
  return sqrtf((float)iv2_dot(a, a));
}

[[gnu::always_inline]]

inline static float iv2_dist(iv2 lhs, iv2 rhs)
{
  iv2 delta = iv2_sub(lhs, rhs);
  return iv2_len(delta);
}

typedef union v3
{
  struct
  {
    float x, y, z;
  };

  float v[3];
} v3;

static const v3 v3_ux = (v3){.x = 1};
static const v3 v3_uy = (v3){.y = 1};
static const v3 v3_uz = (v3){.z = 1};
static const v3 v3_zero = (v3){0};
static const v3 v3_one = (v3){1, 1, 1};

[[gnu::always_inline]]

inline static void v3_inc(v3 *lhs, v3 rhs)
{
  lhs->x += rhs.x;
  lhs->y += rhs.y;
  lhs->z += rhs.z;
}

[[gnu::always_inline]]

inline static v3 v3_max(v3 lhs, v3 rhs)
{
  return (v3){max(lhs.x, rhs.x), max(lhs.y, rhs.y), max(lhs.z, rhs.z)};
}

[[gnu::always_inline]]

inline static v3 v3_min(v3 lhs, v3 rhs)
{
  return (v3){min(lhs.x, rhs.x), min(lhs.y, rhs.y), min(lhs.z, rhs.z)};
}

inline static void v3_print(v3 v, FILE *out)
{
  fprintf(out, "<%f, %f, %f>", v.x, v.y, v.z);
}

[[gnu::always_inline]]

inline static v3 v3_add(v3 lhs, v3 rhs)
{
  return (v3){lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

[[gnu::always_inline]]

inline static v3 v3_neg(v3 v)
{
  return (v3){-v.x, -v.y, -v.z};
}

[[gnu::always_inline]]

inline static v3 v3_sub(v3 lhs, v3 rhs)
{
  return (v3){lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

[[gnu::always_inline]]

inline static v3 v3_mul(v3 lhs, float scalar)
{
  return (v3){lhs.x * scalar, lhs.y * scalar, lhs.z * scalar};
}

[[gnu::always_inline]]

inline static v3 v3_mul_v(v3 lhs, v3 rhs)
{
  return (v3){lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

[[gnu::always_inline]]

inline static v3 v3_div(v3 lhs, float scalar)
{
  return (v3){lhs.x / scalar, lhs.y / scalar, lhs.z / scalar};
}

[[gnu::always_inline]]

inline static v3 v3_cross(v3 lhs, v3 rhs)
{
  // 23 31 12
  return (v3){lhs.y * rhs.z - lhs.z * rhs.y,
              lhs.z * rhs.x - lhs.x * rhs.z,
              lhs.x * rhs.y - lhs.y * rhs.x};
}

[[gnu::always_inline]]

inline static float v3_dot(v3 lhs, v3 rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

[[gnu::always_inline]]

inline static float v3_len(v3 v)
{
  return sqrtf(v3_dot(v, v));
}

[[gnu::always_inline]]

inline static float v3_angle(v3 lhs, v3 rhs)
{
  return acosf(v3_dot(lhs, rhs) / v3_len(lhs) / v3_len(rhs));
}

[[gnu::always_inline]]

inline static float v3_dist(v3 lhs, v3 rhs)
{
  v3 delta = v3_sub(lhs, rhs);
  return v3_len(delta);
}

[[gnu::always_inline]]

inline static v3 v3_normed(v3 v)
{
  float len = v3_len(v);
  return (v3){.x = v.x / len, .y = v.y / len, .z = v.z / len};
}

[[gnu::always_inline]]

inline static void v3_norm(v3 *v)
{
  *v = v3_normed(*v);
}

[[gnu::always_inline]]

inline static v3 v3_lerp(v3 a, v3 b, float d)
{
  return v3_add(a, v3_mul(v3_sub(b, a), d));
}

typedef union v4
{
  struct
  {
    float x, y, z, w;
  };

  struct
  {
    float r, g, b, a;
  };

  float v[4];
} v4;

static const v4 v4_ux = (v4){.x = 1};
static const v4 v4_uy = (v4){.y = 1};
static const v4 v4_uz = (v4){.z = 1};
static const v4 v4_uw = (v4){.w = 1};
static const v4 v4_zero = (v4){0};
static const v4 v4_one = (v4){1, 1, 1, 1};

[[gnu::always_inline]]

inline static v4 v4_neg(v4 lhs)
{
  return (v4){-lhs.x, -lhs.y, -lhs.z, -lhs.w};
}

[[gnu::always_inline]]

inline static v4 v4_add(v4 lhs, v4 rhs)
{
  return (v4){lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
}

[[gnu::always_inline]]

inline static v4 v4_sub(v4 lhs, v4 rhs)
{
  return (v4){lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w};
}

[[gnu::always_inline]]

inline static v4 v4_mul(v4 lhs, float scalar)
{
  return (v4){lhs.x * scalar, lhs.y * scalar, lhs.z * scalar, lhs.w * scalar};
}

[[gnu::always_inline]]

inline static v4 v4_div(v4 lhs, float scalar)
{
  return (v4){lhs.x / scalar, lhs.y / scalar, lhs.z / scalar, lhs.w / scalar};
}

[[gnu::always_inline]]

inline static float v4_dot(v4 lhs, v4 rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

[[gnu::always_inline]]

inline static float v4_len(v4 v)
{
  return sqrtf(v4_dot(v, v));
}

[[gnu::always_inline]]

inline static v4 v4_lerp(v4 a, v4 b, float d)
{
  return v4_add(a, v4_mul(v4_sub(b, a), d));
}

typedef union m4
{
  float e[16];
  float v[4][4];
  v4 r[4];
  struct
  {
    float
      _00, _01, _02, _03,
      _10, _11, _12, _13,
      _20, _21, _22, _23,
      _30, _31, _32, _33;
  };
} m4;

static void m4_print(m4 m)
{
  printf("%f %f %f %f\n", m._00, m._01, m._02, m._03);
  printf("%f %f %f %f\n", m._10, m._11, m._12, m._13);
  printf("%f %f %f %f\n", m._20, m._21, m._22, m._23);
  printf("%f %f %f %f\n", m._30, m._31, m._32, m._33);
}

static const m4 m4_ident = (m4){.r = {v4_ux, v4_uy, v4_uz, v4_uw}};

[[gnu::always_inline]]

inline static v4 m4_col(m4 *m, int col)
{
  return (v4){m->v[0][col], m->v[1][col], m->v[2][col], m->v[3][col]};
}

[[gnu::always_inline]]

inline static m4 m4_tpose(m4 orig)
{
  m4 out;
  out.r[0] = m4_col(&orig, 0);
  out.r[1] = m4_col(&orig, 1);
  out.r[2] = m4_col(&orig, 2);
  out.r[3] = m4_col(&orig, 3);
  return out;
}

[[gnu::always_inline]]

inline static m4 m4_trans(float x, float y, float z)
{
  m4 out = m4_ident;
  out.r[3] = (v4){x, y, z, 1};
  return out;
}

[[gnu::always_inline]]

inline static v4 v4_mul_m(v4 v, m4 mat)
{
  return (v4){
    v.x * mat._00 + v.y * mat._10 + v.z * mat._20 + mat._30,
    v.x * mat._01 + v.y * mat._11 + v.z * mat._21 + mat._31,
    v.x * mat._02 + v.y * mat._12 + v.z * mat._22 + mat._32,
    v.x * mat._03 + v.y * mat._13 + v.z * mat._23 + mat._33};
}

[[gnu::always_inline]]

inline static m4 m4_trans_v(v3 p)
{
  return m4_trans(p.x, p.y, p.z);
}

[[gnu::always_inline]]

inline static m4 m4_scale(float x, float y, float z)
{
  m4 out = m4_ident;
  out.r[0].x = x;
  out.r[1].y = y;
  out.r[2].z = z;
  return out;
}

[[gnu::always_inline]]

inline static m4 m4_scale_v(v3 scale)
{
  return m4_scale(scale.x, scale.y, scale.z);
}

static m4 m4_mul(m4 lhs, m4 rhs)
{
  m4 out;
  rhs = m4_tpose(rhs);
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      out.v[i][j] = v4_dot(lhs.r[i], rhs.r[j]);
    }
  }

  return out;
}

static m4 m4_chg_axis(v3 axis, int ax_num)
{
  v3 t = v3_normed(axis);
  int min = 0;
  if (fabsf(t.v[min]) > fabsf(t.v[1])) min = 1;
  if (fabsf(t.v[min]) > fabsf(t.v[2])) min = 2;

  v3 m = {0};
  m.v[min] = 1;

  v3 f = v3_normed(v3_cross(t, m));
  v3 s = v3_normed(v3_cross(t, f));

  switch (ax_num)
  {
    case 0:
      return (m4){
        .e = {
          t.x, t.y, t.z, 0,
          f.x, f.y, f.z, 0,
          s.x, s.y, s.z, 0,
          0, 0, 0, 1
        }
      };
    case 1:
      return (m4){
        .e = {
          s.x, s.y, s.z, 0,
          t.x, t.y, t.z, 0,
          f.x, f.y, f.z, 0,
          0, 0, 0, 1
        }
      };
    case 2:
      return (m4){
        .e = {
          f.x, f.y, f.z, 0,
          s.x, s.y, s.z, 0,
          t.x, t.y, t.z, 0,
          0, 0, 0, 1
        }
      };
    default:err("m4_chg_axis: invalid axis number!");
  }
}

static m4 m4_look(v3 pos, v3 dir, v3 up)
{
  v3 f = v3_normed(dir);
  v3 s = v3_normed(v3_cross(f, up));
  v3 u = v3_cross(s, f);

  m4 out = {0};

  out.v[0][0] = s.v[0];
  out.v[0][1] = u.v[0];
  out.v[0][2] = -f.v[0];
  out.v[1][0] = s.v[1];
  out.v[1][1] = u.v[1];
  out.v[1][2] = -f.v[1];
  out.v[2][0] = s.v[2];
  out.v[2][1] = u.v[2];
  out.v[2][2] = -f.v[2];
  out.v[3][0] = -v3_dot(s, pos);
  out.v[3][1] = -v3_dot(u, pos);
  out.v[3][2] = v3_dot(f, pos);
  out.v[0][3] = out.v[1][3] = out.v[2][3] = 0.0f;
  out.v[3][3] = 1.0f;

  return out;
}

static m4 m4_persp(float fovy, float aspect, float z_near, float z_far)
{
  m4 out = {0};

  float f = 1.f / tanf(fovy * 0.5f);
  float fn = 1.f / (z_near - z_far);

  out.v[0][0] = f / aspect;
  out.v[1][1] = f;
  out.v[2][2] = (z_near + z_far) * fn;
  out.v[2][3] = -1.0f;
  out.v[3][2] = 2.0f * z_near * z_far * fn;

  return out;
}

static m4
m4_ortho(float left, float right, float bottom, float top, float z_near,
         float z_far)
{
  m4 out = m4_ident;

  float inv_rl = 1.0f / (right - left);
  float inv_tb = 1.0f / (top - bottom);
  float inv_fn = 1.0f / (z_far - z_near);

  out.r[0].x = 2 * inv_rl;
  out.r[1].y = 2 * inv_tb;
  out.r[2].z = -2 * inv_fn;

  out.r[3].x = -(right + left) * inv_rl;
  out.r[3].y = -(top + bottom) * inv_tb;
  out.r[3].z = -(z_far + z_near) * inv_fn;

  return out;
}

static m4 m4_rot_y(float rad)
{
  float c = cosf(rad), s = sinf(rad);
  m4 out = m4_ident;
  out.r[0].x = c;
  out.r[0].z = -s;
  out.r[2].x = s;
  out.r[2].z = c;

  return out;
}

[[gnu::always_inline]]

inline static float lerp(float start, float end, float delta)
{
  return start + (end - start) * delta;
}

[[gnu::always_inline]]

inline static float clamp(float val, float least, float most)
{
  return fminf(fmaxf(val, least), most);
}

[[gnu::always_inline]]

inline static float rad(float deg)
{
  return deg * 3.1415926f / 180.f;
}

static uint32_t iv2_hash(void *key)
{
  return hash_murmur3(key, sizeof(iv2));
}

static bool iv2_peq(void *_lhs, void *_rhs)
{
  return memcmp(_lhs, _rhs, sizeof(iv2)) == 0;
}
