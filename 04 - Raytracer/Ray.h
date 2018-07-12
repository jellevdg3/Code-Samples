#pragma once

namespace Tmpl8
{
    // -----------------------------------------------------------
    // Raytracer struct
    // Generic ray
    // -----------------------------------------------------------
    class Ray
    {
    public:
        // Constructor / destructor
        Ray() {}
        Ray(vec3 origin, vec3 direction, float distance) : O(origin), D(direction), t(distance) { }

        // Data members
        vec3 O, D, rD;
        __m128 o4;
        __m128 rD4;

        float t;
        struct Primitive* hit;
        int hitIndex;
    };

    struct Node4
    {
        __m256 minX;
        __m256 minY;
        __m256 minZ;

        __m256 maxX;
        __m256 maxY;
        __m256 maxZ;
    };

    class Ray4
    {
    public:
        union
        {
            __m256 Ox4;
            float Ox[8];
        };
        union
        {
            __m256 Oy4;
            float Oy[8];
        };
        union
        {
            __m256 Oz4;
            float Oz[8];
        };

        union
        {
            __m256 Dx4;
            float Dx[8];
        };
        union
        {
            __m256 Dy4;
            float Dy[8];
        };
        union
        {
            __m256 Dz4;
            float Dz[8];
        };

        union
        {
            __m256 rDx4;
            float rDx[8];
        };
        union
        {
            __m256 rDy4;
            float rDy[8];
        };
        union
        {
            __m256 rDz4;
            float rDz[8];
        };

        union
        {
            __m256 t4;
            float t[8];
        };

        union
        {
            __m256i hitindex4;
            int hitIndex[8];
        };
        union
        {
            __m256i hit4;
            std::int32_t hit[8];
        };
    };
}
