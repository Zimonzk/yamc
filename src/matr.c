#include <SDL2/SDL.h>
#include <math.h>

void print_mat4(float mat4[][4])
{
	for(int i=0; i < 4; i++) {
		for(int n=0; n < 4; n++) {
			SDL_Log("row: %i, col: %i, val: %f\n", i+1, n+1, mat4[n][i]);
		}
	}

}

void mult_mat4_mat4(float mfirst[][4], float msecond[][4], float mresult[][4])
{
	char i,n,p;
	for(i = 0; i < 4; i++) {
		for(n = 0; n < 4; n++) {
			mresult[i][n] = 0;
			for(p = 0; p < 4; p++) {
				mresult[i][n] += mfirst[p][n] * msecond[i][p];
				//SDL_Log("%i, %i, %f\n", i, n, mresult[i][n]);
			}
		}
	}
}

void mult_mat4_vec4(float mat[4][4], float vec[4], float vresult[4])
{
	char i,n;
	for(i = 0; i < 4; i++) {
		vresult[i] = 0;
		for(n = 0; n < 4; n++) {
			vresult[i] += mat[n][i] * vec[n];
		}
	}
}

void mult_mat4_scalar(float mat[4][4], float scalar, float mresult[4][4])
{
	char i,n;
	for(i = 0; i < 4; i++) {
		for(n = 0; n < 4; n++) {
			mresult[i][n] = mat[i][n] * scalar;
		}
	}
}

float vec3_abs(float* vec3)
{
	return sqrt((vec3[0] * vec3[0]) + (vec3[1] * vec3[1]) + (vec3[2] * vec3[2]));
}

void vec3_normalize(float* vec3)
{
	float abs = vec3_abs(vec3);
	vec3[0] = vec3[0]/abs;
	vec3[1] = vec3[1]/abs;
	vec3[2] = vec3[2]/abs;
}

void vec3_mutiply(float* vec, float scalar, float* vresult)
{
	vresult[0] = vec[0] * scalar;
	vresult[1] = vec[1] * scalar;
	vresult[2] = vec[2] * scalar;
}

void vec3_cross(float* vfirst, float* vsecond, float* vresult)
{
	vresult[0] = (vfirst[1]*vsecond[2]) - (vfirst[2]*vsecond[1]);
	vresult[1] = (vfirst[2]*vsecond[0]) - (vfirst[0]*vsecond[2]);
	vresult[2] = (vfirst[0]*vsecond[1]) - (vfirst[1]*vsecond[0]);

}

float vec3_dot(float* vfirst, float* vsecond)
{
	return ((vfirst[0]*vsecond[0])+(vfirst[1]*vsecond[1])+(vfirst[2]*vsecond[2]));
}

void vec3_add(float* vfirst, float* vsecond, float* vresult)
{
	vresult[0] = vfirst[0] + vsecond[0];
	vresult[1] = vfirst[1] + vsecond[1];
	vresult[2] = vfirst[2] + vsecond[2];

}

void vec3_negate(float* vec3, float* vresult)
{
	vresult[0] = -vec3[0];
	vresult[1] = -vec3[1];
	vresult[2] = -vec3[2];
}


void lookAtRH(float* vec3_eye, float* vec3_center, float* vec3_up, float mresult[][4])
{
	float f[3];
	float s[3];
	float u[3];

	float v3help0[3];

	vec3_negate(vec3_eye, v3help0);
	vec3_add(vec3_center, v3help0, f);
	vec3_normalize(f);

	vec3_cross(f, vec3_up, s);
	vec3_normalize(s);

	vec3_cross(s, f, u);

	mresult[0][0] = s[0];
	mresult[1][0] = s[1];
	mresult[2][0] = s[2];
	mresult[0][1] = u[0];
	mresult[1][1] = u[1];
	mresult[2][1] = u[2];
	mresult[0][2] =-f[0];
	mresult[1][2] =-f[1];
	mresult[2][2] =-f[2];
	mresult[3][0] =-vec3_dot(s, vec3_eye);
	mresult[3][1] =-vec3_dot(u, vec3_eye);
	mresult[3][2] = vec3_dot(f, vec3_eye);
	mresult[0][3] = 0;
	mresult[1][3] = 0;
	mresult[2][3] = 0;
	mresult[3][3] = 1;

	/* tha's what glm says
	   detail::tvec3<T, P> f(normalize(center - eye));
	   detail::tvec3<T, P> s(normalize(cross(f, up)));
	   detail::tvec3<T, P> u(cross(s, f));

	   detail::tmat4x4<T, P> Result(1);
	   Result[0][0] = s.x;
	   Result[1][0] = s.y;
	   Result[2][0] = s.z;
	   Result[0][1] = u.x;
	   Result[1][1] = u.y;
	   Result[2][1] = u.z;
	   Result[0][2] =-f.x;
	   Result[1][2] =-f.y;
	   Result[2][2] =-f.z;
	   Result[3][0] =-dot(s, eye);
	   Result[3][1] =-dot(u, eye);
	   Result[3][2] = dot(f, eye);*/
}

void perspectiveRH(float fovRad, float aspectRatio, float zNear, float zFar, float mresult[][4])
{
	float tanfovh = tanf(fovRad/2.0);

	memset(mresult, 0, sizeof(float) * 16);

	mresult[0][0] = 1.0 / (aspectRatio * tanfovh);
	mresult[1][1] = 1.0 / (tanfovh);
	mresult[2][2] = - (zFar + zNear) / (zFar - zNear);
	mresult[2][3] = -1.0;
	mresult[3][2] = - (2.0 * zFar * zNear) / (zFar - zNear);

	/* that's what glm says
	   valType tanHalfFovy = tan(rad / valType(2));

	   detail::tmat4x4<valType, defaultp> Result(valType(0));
	   Result[0][0] = valType(1) / (aspect * tanHalfFovy);
	   Result[1][1] = valType(1) / (tanHalfFovy);
	   Result[2][2] = - (zFar + zNear) / (zFar - zNear);
	   Result[2][3] = - valType(1);
	   Result[3][2] = - (valType(2) * zFar * zNear) / (zFar - zNear);*/
}

void inv_mat4(float mat[4][4], float mresult[4][4]) {
	float t[6];
	float det;
	float a = mat[0][0], b = mat[0][1], c = mat[0][2], d = mat[0][3],
	e = mat[1][0], f = mat[1][1], g = mat[1][2], h = mat[1][3],
	i = mat[2][0], j = mat[2][1], k = mat[2][2], l = mat[2][3],
	m = mat[3][0], n = mat[3][1], o = mat[3][2], p = mat[3][3];

	t[0] = k * p - o * l; t[1] = j * p - n * l; t[2] = j * o - n * k;
	t[3] = i * p - m * l; t[4] = i * o - m * k; t[5] = i * n - m * j;

	mresult[0][0] =  f * t[0] - g * t[1] + h * t[2];
	mresult[1][0] =-(e * t[0] - g * t[3] + h * t[4]);
	mresult[2][0] =  e * t[1] - f * t[3] + h * t[5];
	mresult[3][0] =-(e * t[2] - f * t[4] + g * t[5]);

	mresult[0][1] =-(b * t[0] - c * t[1] + d * t[2]);
	mresult[1][1] =  a * t[0] - c * t[3] + d * t[4];
	mresult[2][1] =-(a * t[1] - b * t[3] + d * t[5]);
	mresult[3][1] =  a * t[2] - b * t[4] + c * t[5];

	t[0] = g * p - o * h; t[1] = f * p - n * h; t[2] = f * o - n * g;
	t[3] = e * p - m * h; t[4] = e * o - m * g; t[5] = e * n - m * f;

	mresult[0][2] =  b * t[0] - c * t[1] + d * t[2];
	mresult[1][2] =-(a * t[0] - c * t[3] + d * t[4]);
	mresult[2][2] =  a * t[1] - b * t[3] + d * t[5];
	mresult[3][2] =-(a * t[2] - b * t[4] + c * t[5]);

	t[0] = g * l - k * h; t[1] = f * l - j * h; t[2] = f * k - j * g;
	t[3] = e * l - i * h; t[4] = e * k - i * g; t[5] = e * j - i * f;

	mresult[0][3] =-(b * t[0] - c * t[1] + d * t[2]);
	mresult[1][3] =  a * t[0] - c * t[3] + d * t[4];
	mresult[2][3] =-(a * t[1] - b * t[3] + d * t[5]);
	mresult[3][3] =  a * t[2] - b * t[4] + c * t[5];

	det = 1.0f / (a * mresult[0][0] + b * mresult[1][0]
			+ c * mresult[2][0] + d * mresult[3][0]);

	mult_mat4_scalar(mresult, det, mresult);
}
