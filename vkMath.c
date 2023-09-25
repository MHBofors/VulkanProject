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
        V = v_scale(1/norm_V, V);
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

vector crossproduct(vector U, vector V)
{
    vector result = {.x = U.y*V.z - U.z*V.y,
                     .y = U.z*V.x - U.x*V.z,
                     .z = U.x*V.y - U.y*V.x};
    return result;
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
    
    if(cos_phi < -1 + tolerance) {
        
        vector W = {
            .x = 0.0f,
            .y = 0.0f,
            .z = 1.0f
        };
        cos_phi = dot(U,W);
        
        if(cos_phi < -1 + tolerance) {
            W.y = 1;
            W.z = 0;
        }
        
        vector axis = crossproduct(U, W);
        
        quaternion rotation = {
            .a = 0,
            .u = axis.x,
            .v = axis.y,
            .w = axis.z
        };
        
        return rotation;
    }
    
    vector axis = crossproduct(U, V);
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

void transform(float A[4][4], float v[4])
{
    float v_transformed[4];
    
    for(int i = 0; i < 4; i++)
    {
        v_transformed[i] = 0;
        
        for (int j = 0; j < 4; j++)
        {
            v_transformed[i] += A[i][j] * v[j];
        }
    }
    for (int i = 0; i < 4; i++) {
        v[i] = v_transformed[i];
    }
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

void transposeMatrix(float matrix[4][4])
{
    float transpose[4][4];
    for(int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            transpose[j][i] = matrix[i][j];
        }
    }
    
    matcpy(transpose, matrix);
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

void identityMatrix(float matrix[4][4])
{
    float identity[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    
    matcpy(identity, matrix);
    
    return;
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

void quaternionMatrix(float matrix[4][4], quaternion q)
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

void rotationMatrix(float matrix[4][4], float angle, vector axis)
{
    quaternion rotation = q_angle_vector(angle, axis);
    
    quaternionMatrix(matrix, rotation);
    return;
}

void vectorVectorMatrix(float matrix[4][4], vector U, vector V)
{
    quaternion rotation = q_vector_vector(U, V);
    
    quaternionMatrix(matrix, rotation);
    return;
}

void cameraTransform(float matrix[4][4], vector eye, vector eye_basis[3])
{
    float translation[4][4] = {
        {1, 0, 0, eye.x},
        {0, 1, 0, eye.y},
        {0, 0, 1, eye.z},
        {0, 0, 0,   1}
    };
    
    
    
    float transition[4][4] = {
        {eye_basis[0].x, eye_basis[0].y, eye_basis[0].z, 0},
        {eye_basis[1].x, eye_basis[1].y, eye_basis[1].z, 0},
        {eye_basis[2].x, eye_basis[2].y, eye_basis[2].z, 0},
        {0             , 0             , 0             , 1}
    };
    
    matmul(translation, transition);
    
    transposeMatrix(transition);
    
    matcpy(transition, matrix);
    
    return;
}

void cameraMatrix(float matrix[4][4], vector position_camera, vector position_object, vector up)
{
    //vector Z = normalise(v_sub(position_camera, position_object));
    
    vector view = normalise(v_sub(position_object, position_camera));
    
    /*
    float rotation[4][4];
    
    vector Z = {0, 0, 1};
    
    float x[4] = {1, 0, 0, 1};
    float y[4] = {0, 1, 0, 1};
    float z[4] = {0, 0, 1, 1};
    
    vectorVectorMatrix(rotation, Z, view);
    
    transform(rotation, x);
    transform(rotation, y);
    transform(rotation, z);
    
    vector X = {x[0], x[1], x[2]};
    vector Y = {y[0], y[1], y[2]};
    Z.x = z[0]; Z.x = z[1]; Z.x = z[2];
    */
    
    vector Z = normalise(v_sub(position_camera, position_object));
    
    vector X = normalise(crossproduct(up, Z));
    
    vector Y = crossproduct(Z, X);
    /*
    float camera[4][4] = {
        {X.x               , X.y              , X.z              , 0},
        {Y.x               , Y.y              , Y.z              , 0},
        {Z.x               , Z.y              , Z.z              , 0},
        {position_camera.x , position_camera.y, position_camera.z, 1}
    };//For coordinate system where x is width, y is height, and z is depth
    */
    
    float transition[4][4] = {
        {X.x, X.y, X.z, 0},
        {Y.x, Y.y, Y.z, 0},
        {Z.x, Z.y, Z.z, 0},
        {0  , 0  , 0  , 1}
    };
    
    float translation[4][4] = {
        {1, 0, 0, -position_camera.x},
        {0, 1, 0, -position_camera.y},
        {0, 0, 1, -position_camera.z},
        {0, 0, 0, 1                 }
    };
    
    
    matmul(transition, translation);
    //transposeMatrix(camera);
    
    matcpy(translation, matrix);
    
    return;
}

void perspectiveMatrix(float matrix[4][4], float fov, float aspect_ratio, float near, float far) {
    float tan_half_angle = tan(fov/2);
    
    float A = far/(far - near);
    float B = -(near*far)/(far - near);
    
    float camera[4][4] = {
        {1/(aspect_ratio*tan_half_angle),  0.0f               ,  0.0f, 0.0f},
        {0.0f                           , -1/(tan_half_angle),  0.0f, 0.0f},
        {0.0f                           ,  0.0f               , -A   , B   },
        {0.0f                           ,  0.0f               , -1.0f, 0.0f}
    };
    
    matcpy(camera, matrix);
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
