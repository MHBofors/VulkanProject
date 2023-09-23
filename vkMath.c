//
//  vkMath.c
//  vkProject
//
//  Created by Markus Höglin on 2023-09-18.
//

#include "vkMath.h"

float dot(vector U, vector V)
{
    return U.x*V.x + U.y*V.y + U.z*V.z;
}

float norm(vector V)
{
    return sqrt(dot(V,V));
}

vector normalise(vector V)
{
    float norm_V = norm(V);
    if(norm_V != 0)
    {
        v_scale(1/norm_V, V);
    }
    
    return V;
}

vector v_scale(float a, vector V)
{
    V.x *= a;
    V.y *= a;
    V.z *= a;
    return V;
}

vector v_sub(vector U, vector V)
{
    U.x -= V.x;
    U.y -= V.y;
    U.z -= V.z;
    return U;
}

quaternion q_conjugate(quaternion q)
{
    q.u *= -1;
    q.v *= -1;
    q.w *= -1;
    return q;
}

float q_norm(quaternion q)
{
    return sqrt(q.a*q.a + q.u*q.u + q.v*q.v + q.w*q.w);
}

quaternion q_mult(quaternion p, quaternion q)
{
    quaternion r = {
        .a = p.a*q.a - p.u*q.u - p.v*q.v - p.w*q.w,
        .u = p.a*q.u + p.u*q.a + p.v*q.w - p.w*q.v,
        .v = p.a*q.v - p.u*q.w + p.v*q.a + p.w*q.u,
        .w = p.a*q.w + p.u*q.v - p.v*q.u + p.w*q.a
    };
    return r;
}

quaternion q_angle_vector(float phi, vector V)
{
    float c = cos(phi/2);
    float s = sin(phi/2);
    float norm_V = norm(V);
    
    if(norm_V == 0){
        exit(1);
    }
    
    vector V_normalised = {
        .x = V.x/norm_V,
        .y = V.y/norm_V,
        .z = V.z/norm_V
    };
    
    quaternion q = {
        .a = c,
        .u = s * V_normalised.x,
        .v = s * V_normalised.y,
        .w = s * V_normalised.z
    };
    return q;
}

quaternion q_vector_vector(vector U, vector V)
{
    float tolerance = 0.001f;
    U = normalise(U);
    V = normalise(V);
    
    float cos_phi = dot(U,V);
    
    vector W = {
        .x = 0.0f,
        .y = 0.0f,
        .z = 1.0f
    };
    
    if(cos_phi < -1 + tolerance){
        vector axis = v_sub(W, v_scale(dot(W,U)/dot(U,U), U));
        
        quaternion rotation = {
            .a = 0,
            .u = axis.x,
            .v = axis.y,
            .w = axis.z
        };
        
        return rotation;
    }
    
    V = v_sub(V, v_scale(dot(V,U)/dot(U,U), U));
    
    vector axis = v_sub(v_sub(W, v_scale(dot(W,U)/dot(U,U), U)), v_scale(dot(W,V)/dot(V,V), V));
    axis = normalise(axis);
    
    float A = sqrt(2*(1 + cos_phi));
    float B = sqrt(2*(1 - cos_phi));
    
    quaternion rotation = {
        .a = A * 0.5f,
        .u = axis.x * B * 0.5f,
        .v = axis.y * B * 0.5f,
        .w = axis.z * B * 0.5f
    };
    
    return rotation;
}

void matcpy(float source[4][4], float destination[4][4])
{
    for(int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            destination[i][j] = source[i][j];
        }
    }
}

void matmul(float A[4][4], float B[4][4]) {
    float result[4][4];
    
    for(int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            float value = 0;
            
            for(int k = 0; k < 4; k++)
            {
                value += A[i][k] * B[k][j];
            }
            
            result[i][j] = value;
        }
    }
    
    matcpy(result, B);
}

void translationMatrix(float matrix[4][4], vector V)
{
    float translation[4][4] = {
        {1, 0, 0, V.x},
        {0, 1, 0, V.y},
        {0, 0, 1, V.z},
        {0, 0, 0,   1}
    };
    
    matcpy(translation, matrix);
    
    return;
}

void scalingMatrix(float matrix[4][4], vector V)
{
    float translation[4][4] = {
        {V.x, 0, 0, 0},
        {0, V.y, 0, 0},
        {0, 0, V.z, 0},
        {0, 0, 0,   1}
    };
    
    matcpy(translation, matrix);
    
    return;
}

void rotationMatrix(float matrix[4][4], quaternion q)
{
    float rotation[4][4] = {
        {1 - 2*(q.v*q.v + q.w*q.w), 2*(q.u*q.v + q.w*q.a), 2*(q.u*q.w-q.v*q.a), 0},
        {2*(q.u*q.v-q.w*q.a), 1 - 2*(q.u*q.u + q.w*q.w), 2*(q.v*q.w + q.u*q.a), 0},
        {2*(q.u*q.w + q.u*q.a), 2*(q.v*q.w-q.u*q.a), 1 - 2*(q.v*q.v + q.v*q.v), 0},
        {0                        , 0                    , 0                  , 1}
    };
    
    matcpy(rotation, matrix);
    
    return;
}

void matprint(float matrix[4][4]) {
    printf("\n");
    for(int i = 0; i < 4; i++)
    {
        printf("|");
        for (int j = 0; j < 4; j++)
        {
            printf(" %4.1f ", matrix[i][j]);
        }
        printf("|\n");
    }
}
