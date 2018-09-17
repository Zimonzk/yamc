#ifndef MATR_H_INCLUDED
#define MATR_H_INCLUDED
void print_mat4(float mat4[][4]);

void mult_mat4_mat4(float mfirst[][4], float msecond[][4], float mresult[][4]);
void mult_mat4_vec4(float mat[4][4], float vec[4], float vresult[4]);
void mult_mat4_scalar(float mat[4][4], float scalar, float mresult[4][4]);

float vec3_abs(float* vec3); /*can pass vec4 with w as w is the last entry*/
void vec3_normalize(float* vec3); /*  ---`´---  */
void vec3_multiply(float* vec3, float scalar, float* vresult); /*  ---`´---  */
void vec3_cross(float* vfirst, float* vsecond, float* vresult); /*  ---`´---  */
float vec3_dot(float* vfirst, float* vsecond); /*  ---`´---  */
void vec3_add(float* vfirst, float* vsecond, float* vresult); /*  ---`´---  */
void vec3_negate(float* vec3, float* vresult); /*  ---`´---  */

void lookAtRH(float* vec3_eye, float* vec3_center, float* vec3_up, float mresult[][4]);
void perspectiveRH(float fovRad, float aspectRatio, float zNear, float zFar, float mresult[][4]);

#endif // MATR_H_INCLUDED
