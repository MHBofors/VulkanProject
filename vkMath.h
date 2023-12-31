//
//  vkMath.h
//  vkProject
//
//  Created by Markus Höglin on 2023-09-18.
//

#ifndef vkMath_h
#define vkMath_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct vec3 {
    float x;
    float y;
    float z;
} vector;

typedef struct Quaternion {
    float r;
    float i;
    float j;
    float k;
} quaternion;

float dot(vector U, vector V);

float norm(vector V);

vector normalise(vector V);

vector v_scale(float a, vector V);

vector v_sub(vector U, vector V);

vector crossproduct(vector U, vector V);

quaternion q_conjugate(quaternion p);

float q_norm(quaternion q);

quaternion q_mult(quaternion p, quaternion q);

quaternion q_angle_vector(float phi, vector V);

quaternion q_vector_vector(vector U, vector V);

void transform(float A[4][4], float v[4]);

void matcpy(float source[4][4], float destination[4][4]);

void matmul(float A[4][4], float B[4][4]);

void transposeMatrix(float matrix[4][4]);

void identityMatrix(float matrix[4][4]);

void translationMatrix(float matrix[4][4], vector V);

void scalingMatrix(float matrix[4][4], vector V);

void projectionMatrix(float matrix[4][4], vector V);

void quaternionMatrix(float matrix[4][4], quaternion q);

void rotationMatrix(float matrix[4][4], float angle, vector axis);

void cameraTransform(float matrix[4][4], vector eye_basis[3], vector eye, vector object);

void cameraMatrix(float matrix[4][4], vector eye, vector object, vector up);

void cameraMatrixOld(float matrix[4][4], vector position_camera, vector position_object, vector up);

void perspectiveMatrix(float matrix[4][4], float fov, float aspect_ratio, float near, float far);

void matprint(float matrix[4][4]);

#endif /* vkMath_h */
