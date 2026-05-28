#pragma once

#include "cpbase.h"
#include "cpvec.h"

#define CPM_PI 3.14159265358979323846f

#define CPM_MIN(x, y) ((x) < (y) ? (x) : (y))
#define CPM_MAX(x, y) ((x) > (y) ? (x) : (y))
#define CPM_ABS(x) ((x) > 0 ? (x) : -(x))
#define CPM_CLAMP(x, n, m) ((x) > (n) ? ((x) < (m) ? (x) : (m)) : (n))

// {{{ Arithmetic

f32 cpm_factorial(i32 n);
f32 cpm_expf(f32 x);
f32 cpm_powf(f32 x, i32 n);
f32 cpm_sinf(f32 x);
f32 cpm_cosf(f32 x);
f32 cpm_tanf(f32 x);
f32 cpm_sinhf(f32 x);
f32 cpm_coshf(f32 x);
f32 cpm_tanhf(f32 x);
f32 cpm_sqrt(f32 n);
f32 cpm_logf(f32 x);
f32 cpm_modf(f32 x, f32 y);
f32 cpm_floorf(f32 x);
f32 cpm_ceilf(f32 x);
f32 cpm_rad(f32 deg);
b8 cpm_isnan(f32 x);

#ifdef CPM_IMPL
f32 cpm_factorial(i32 n) {
    if (n == 0 || n == 1) {
        return 1.0f;
    }
    f32 result = 1.0f;
    for (int i = 2; i <= n; i++) {
        result *= (f32)i;
    }
    return result;
}

f32 cpm_expf(f32 x) {
    if (x > 88.0f) {
        return F32_MAX;
    }
    if (x < -88.0f) {
        return 0.0f;
    }

    f32 ln2 = 0.693147180559945f;
    i32 k = (i32)((x / ln2) + 0.5f);
    f32 r = x - ((f32)k * ln2);

    f32 result = 1.0f + r + ((r * r) / 2.0f) + ((r * r * r) / 6.0f) +
                 ((r * r * r * r) / 24.0f) + ((r * r * r * r * r) / 120.0f) +
                 ((r * r * r * r * r * r) / 720.0f);

    i32 bits = (k + 127) << 23;
    f32 pow2k;
    memcpy(&pow2k, &bits, sizeof(f32));

    return result * pow2k;
}

f32 cpm_powf(f32 x, i32 n) {
    f32 result = 1.0f;
    for (int i = 0; i < n; i++) {
        result *= x;
    }
    return n > 0 ? result : 1 / result;
}

f32 cpm_sinf(f32 x) {
    f32 result = 0.0f;

    while (x > CPM_PI) {
        x -= 2 * CPM_PI;
    }
    while (x < -CPM_PI) {
        x += 2 * CPM_PI;
    }

    i32 n = 10;
    for (int i = 0; i < n; i++) {
        f32 sign = cpm_powf(-1, i);
        f32 num = cpm_powf(x, (2 * i) + 1);
        f32 den = cpm_factorial((2 * i) + 1);
        result += sign * (num / den);
    }
    return result;
}

f32 cpm_cosf(f32 x) {
    f32 result = 1.0f;
    f32 term = 1.0f;

    while (x > CPM_PI) {
        x -= 2 * CPM_PI;
    }
    while (x < -CPM_PI) {
        x += 2 * CPM_PI;
    }

    i32 n = 10;
    for (int i = 1; i <= n; i++) {
        f32 sign = -cpm_powf(x, 2);
        f32 num = 2.0f * (f32)i;
        term *= sign / (num * (num - 1));
        result += term;
    }
    return result;
}

f32 cpm_tanf(f32 x) { return cpm_sinf(x) / cpm_cosf(x); }

f32 cpm_sinhf(f32 x) { return (cpm_expf(x) - cpm_expf(-x)) / 2; }

f32 cpm_coshf(f32 x) { return (cpm_expf(x) + cpm_expf(-x)) / 2; }

f32 cpm_tanhf(f32 x) { return cpm_sinhf(x) / cpm_coshf(x); }

f32 cpm_sqrt(f32 n) {
    if (n < 0) {
        return -1.0f;
    }
    if (n == 0) {
        return 0.0f;
    }
    f32 tolerance = 1e-5f;
    f32 guess = n / 2.0f;
    for (u32 i = 0; i < 100; i++) {
        f32 newGuess = (guess + (n / guess)) / 2.0f;
        if (CPM_ABS((newGuess * newGuess) - n) < tolerance) {
            return newGuess;
        }
        guess = newGuess;
    }
    return guess;
}

f32 cpm_logf(f32 x) {
    if (x <= 0) {
        return -F32_MAX;
    }
    f32 y = x - 1.0f;
    for (i32 i = 0; i < 100; i++) {
        f32 ey = cpm_expf(y);
        y -= (ey - x) / ey;
    }
    return y;
}

f32 cpm_modf(f32 x, f32 y) {
    u32 fit = (u32)x / (u32)y;
    return x - (y * (float)fit);
}

f32 cpm_floorf(f32 x) { return (f32)((i32)x); }

f32 cpm_ceilf(f32 x) {

    return x > (f32)((i32)x) ? (f32)((i32)x + 1) : (f32)((i32)x);
}

f32 cpm_rad(f32 deg) { return deg * (CPM_PI / 180.0f); }

b8 cpm_isnan(f32 x) {
    /* Prevent this being changed
     * by compiler optimizations
     * (fastmath)
     */
    volatile f32 y = x;
    return (b8)(y != y);
}
#endif

// }}}

// {{{ Linear algebra

typedef union {
    f32 data[4];
    struct {
        f32 x, y, z, w;
    };
    struct {
        f32 r, g, b, a;
    };
} vec4f;

typedef union {
    f32 data[3];
    struct {
        f32 x, y, z;
    };
} vec3f;

typedef union {
    i32 data[2];
    struct {
        i32 x, y;
    };
} vec2i;

#define VEC2I_INIT(v)                                                          \
    (vec2i) { v, v }
#define VEC2I(x, y)                                                            \
    (vec2i) { x, y }

typedef union {
    f32 data[2];
    struct {
        f32 x, y;
    };
} vec2f;

#define VEC2F_INIT(v)                                                          \
    (vec2f) { v, v }
#define VEC2F(x, y)                                                            \
    (vec2f) { x, y }

b8 vec2f_cmp(vec2f a, vec2f b);
vec2f vec2f_add(vec2f *a, vec2f *b);
vec2f vec2f_sub(vec2f *a, vec2f *b);
vec2f vec2f_mul(vec2f *a, vec2f *b);
vec2f vec2f_div(vec2f *a, vec2f *b);
vec2f vec2f_f32_mul(vec2f *a, f32 b);
f32 vec2f_dist(vec2f *v1, vec2f *v2);
f32 vec2f_dist2(vec2f *v1, vec2f *v2);
f32 vec2f_dot(vec2f *a, vec2f *b);
f32 vec2f_length(vec2f *a);
vec2f vec2f_clamp(vec2f *v, vec2f *n, vec2f *m);

#ifdef CPM_IMPL
b8 vec2f_cmp(vec2f a, vec2f b) { return a.x == b.x && a.y == b.y; }

vec2f vec2f_add(vec2f *a, vec2f *b) {
    return (vec2f){a->x + b->x, a->y + b->y};
}
vec2f vec2f_sub(vec2f *a, vec2f *b) {
    return (vec2f){a->x - b->x, a->y - b->y};
}
vec2f vec2f_mul(vec2f *a, vec2f *b) {
    return (vec2f){a->x * b->x, a->y * b->y};
}
vec2f vec2f_div(vec2f *a, vec2f *b) {
    return (vec2f){a->x / b->x, a->y / b->y};
}

vec2f vec2f_f32_mul(vec2f *a, f32 b) { return (vec2f){a->x * b, a->y * b}; }

f32 vec2f_dist(vec2f *v1, vec2f *v2) {
    f32 a = CPM_ABS(v1->x - v2->x);
    f32 b = CPM_ABS(v1->y - v2->y);

    return cpm_sqrt((a * a) + (b * b));
}

f32 vec2f_dist2(vec2f *v1, vec2f *v2) {
    f32 a = CPM_ABS(v1->x - v2->x);
    f32 b = CPM_ABS(v1->y - v2->y);

    return (a * a) + (b * b);
}

f32 vec2f_dot(vec2f *a, vec2f *b) { return (a->x * b->x) + (a->y * b->y); }

f32 vec2f_length(vec2f *a) { return cpm_sqrt((a->x * a->x) + (a->y * a->y)); }

vec2f vec2f_clamp(vec2f *v, vec2f *n, vec2f *m) {
    return (vec2f){CPM_CLAMP(v->x, n->x, m->x), CPM_CLAMP(v->y, n->y, m->y)};
}
#endif

typedef struct {
    f32 data[4][4];
} mat4f;

void mat4f_identity(mat4f *m);
void mat4f_translate(mat4f *m, vec3f *v);
void mat4f_rotate(mat4f *m, f32 angle_rad, vec3f *axis);
void mat4f_scale(mat4f *m, vec3f *v);
vec4f mat4f_mul_vec4f(mat4f *m, vec4f v);
void mat4f_mul(mat4f *a, mat4f *b, mat4f *dest);
void mat4f_ortho(mat4f *m, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                 f32 far);
static f32 minor_mat3f_det(f32 data[4][4], u32 r, u32 c);
f32 mat4f_det(mat4f *m);
void mat4f_inv(mat4f *m, mat4f *out);

#ifdef CPM_IMPL
void mat4f_identity(mat4f *m) {
    for (u32 j = 0; j < 4; j++) {
        for (u32 i = 0; i < 4; i++) {
            m->data[i][j] = i == j ? 1.0f : 0.0f;
        }
    }
}

void mat4f_translate(mat4f *m, vec3f *v) {
    m->data[3][0] += (m->data[0][0] * v->x) + (m->data[1][0] * v->y) +
                     (m->data[2][0] * v->z);
    m->data[3][1] += (m->data[0][1] * v->x) + (m->data[1][1] * v->y) +
                     (m->data[2][1] * v->z);
    m->data[3][2] += (m->data[0][2] * v->x) + (m->data[1][2] * v->y) +
                     (m->data[2][2] * v->z);
    m->data[3][3] += (m->data[0][3] * v->x) + (m->data[1][3] * v->y) +
                     (m->data[2][3] * v->z);
}

void mat4f_rotate(mat4f *m, f32 angle_rad, vec3f *axis) {
    f32 c = cpm_cosf(angle_rad);
    f32 s = cpm_sinf(angle_rad);
    f32 t = 1.0f - c;

    f32 x = axis->x;
    f32 y = axis->y;
    f32 z = axis->z;

    mat4f rot;
    mat4f_identity(&rot);

    rot.data[0][0] = (t * x * x) + c;
    rot.data[0][1] = (t * x * y) + (s * z);
    rot.data[0][2] = (t * x * z) - (s * y);

    rot.data[1][0] = (t * x * y) - (s * z);
    rot.data[1][1] = (t * y * y) + c;
    rot.data[1][2] = (t * y * z) + (s * x);

    rot.data[2][0] = (t * x * z) + (s * y);
    rot.data[2][1] = (t * y * z) - (s * x);
    rot.data[2][2] = (t * z * z) + c;

    mat4f result;
    mat4f_identity(&result);
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (u32 k = 0; k < 4; k++) {
                sum += m->data[k][row] * rot.data[col][k];
            }
            result.data[col][row] = sum;
        }
    }
    *m = result;
}

void mat4f_scale(mat4f *m, vec3f *v) {
    m->data[0][0] *= v->x;
    m->data[1][1] *= v->y;
    m->data[2][2] *= v->z;
}

vec4f mat4f_mul_vec4f(mat4f *m, vec4f v) {
    vec4f out;
    for (u32 i = 0; i < 4; i++) {
        out.data[i] = (m->data[i][0] * v.data[0]) +
                      (m->data[i][1] * v.data[1]) +
                      (m->data[i][2] * v.data[2]) + (m->data[i][3] * v.data[3]);
    }
    return out;
}

void mat4f_mul(mat4f *a, mat4f *b, mat4f *dest) {
    mat4f_identity(dest);
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (u32 k = 0; k < 4; k++) {
                sum += a->data[k][row] * b->data[col][k];
            }
            dest->data[col][row] = sum;
        }
    }
}

void mat4f_ortho(mat4f *m, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                 f32 far) {
    mat4f_identity(m);
    m->data[0][0] = 2.0f / (right - left);
    m->data[1][1] = 2.0f / (top - bottom);
    m->data[2][2] = -2.0f / (far - near);
    m->data[3][0] = -(right + left) / (right - left);
    m->data[3][1] = -(top + bottom) / (top - bottom);
    m->data[3][2] = -(far + near) / (far - near);
}

static f32 minor_mat3f_det(f32 data[4][4], u32 r, u32 c) {
    f32 sub[3][3];
    u32 si = 0;
    for (int i = 0; i < 4; i++) {
        if (i == r) {
            continue;
        }
        u32 sj = 0;
        for (u32 j = 0; j < 4; j++) {
            if (j == c) {
                continue;
            }
            sub[si][sj++] = data[i][j];
        }
        si++;
    }
    return (sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1])) -
           (sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0])) +
           (sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]));
}

f32 mat4f_det(mat4f *m) {
    f32 det = 0.0f;
    for (u32 j = 0; j < 4; j++) {
        f32 cofactor = minor_mat3f_det(m->data, 0, j);
        if (j % 2 != 0) {
            cofactor = -cofactor;
        }
        det += m->data[0][j] * cofactor;
    }
    return det;
}

void mat4f_inv(mat4f *m, mat4f *out) {
    f32 cofactors[4][4];
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            f32 c = minor_mat3f_det(m->data, i, j);
            if ((i + j) % 2 != 0) {
                c = -c;
            }
            cofactors[i][j] = c;
        }
    }
    f32 det = 0.0f;
    for (u32 j = 0; j < 4; j++) {
        det += m->data[0][j] * cofactors[0][j];
    }
    f32 inv_det = 1.0f / det;
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            out->data[i][j] = cofactors[j][i] * inv_det;
        }
    }
}
#endif

// }}}
