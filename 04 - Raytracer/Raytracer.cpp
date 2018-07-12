#include "template.h"

#include "Raytracer.h"
#include "BVH.h"
#include "Primitive.h"

#include "RayTraceLineWorker.h"

#include <thread>
#include <mutex>
#include <string>

struct Tmpl8::RayTracerThreads
{
    std::thread threads[WORKING_THREADS];
    std::mutex taskObtainLock;
};

static void RayTraceLineWorker(void* rayTracer);

Raytracer::Raytracer()
    : scene(0)
{
    threads = nullptr;
    running = false;
    skySphere = nullptr;
    primitiveCount = 0;
    primitives = nullptr;
    bvhManager = nullptr;
}

Raytracer::~Raytracer()
{
    // Stop threads first before cleaning up any other data
    running = false;
    if (threads != nullptr)
    {
        for (int i = 0; i < WORKING_THREADS; i++)
        {
            threads->threads[i].join();
        }

        delete threads;
        threads = nullptr;
    }

    // Clean up
    if (bvhManager != nullptr)
        delete bvhManager;
    bvhManager = nullptr;

    if (primitives != nullptr)
        _aligned_free(primitives);
    primitives = nullptr;

    if (skySphere != nullptr)
        delete skySphere;
    skySphere = nullptr;
}

void Raytracer::Init(Surface* screen, Scene* scene)
{
    Mesh::screen = screen;
    this->screen = screen;
    this->scene = scene;

    rayLineTasks = -1;
	
    threads = new RayTracerThreads();
    bvhManager = new BVHManager();
	
	rayTotalTasks = (SCRWIDTH * SCRHEIGHT) / (PACKET_SIZE * PACKET_SIZE);

	// Start thread workers
    running = true;
    for (int i = 0; i < WORKING_THREADS; i++)
    {
        threads->threads[i] = thread(RayTraceLineWorker, this);
    }
}

void Raytracer::ConstructBVH(bool forcerebuild)
{
    // Count tris
    for (Model* model : scene->meshes)
    {
        primitiveCount += model->mesh->tris;
    }

    // Count spheres
    primitiveCount += (int)(scene->spheres.size());

    primitives = (Primitive*)_aligned_malloc(primitiveCount * sizeof(Primitive), 512);

    int j = 0;

    // Initialize tris
    for (Model* model : scene->meshes)
    {
        for (int i = 0; i < model->mesh->tris; i++)
        {
            int v1 = model->mesh->tri[i * 3 + 0];
            int v2 = model->mesh->tri[i * 3 + 1];
            int v3 = model->mesh->tri[i * 3 + 2];

            primitives[j].index = i;
            primitives[j].poscount = 3;
            primitives[j].pos[0] = &model->tpos[v1];
            primitives[j].pos[1] = &model->tpos[v2];
            primitives[j].pos[2] = &model->tpos[v3];
            primitives[j].radius = 0.0f;
            primitives[j].type = Primitive::TRIANGLE;
            primitives[j].owner = model->mesh;
            primitives[j].mat = model->mat;

            j++;
        }
    }

    // Initialize spheres
    for (RTSphere* sphere : scene->spheres)
    {
        primitives[j].index = 0;
        primitives[j].poscount = 1;
        primitives[j].pos[0] = &sphere->position;
        primitives[j].radius = sphere->radius;
        primitives[j].type = Primitive::SPHERE;
        primitives[j].owner = sphere;
        primitives[j].mat = &sphere->material;

        j++;
    }

    bvhManager->Construct(primitives, primitiveCount, forcerebuild);
}


void Raytracer::IntersectScene(Ray& ray)
{
    ray.t = 9999999.f;
    ray.hit = nullptr;
    ray.rD = 1.f / ray.D;
    ray.o4 = _mm_set_ps(0.0f, ray.O.z, ray.O.y, ray.O.x);
    ray.rD4 = _mm_set_ps(0.0f, ray.rD.z, ray.rD.y, ray.rD.x);

    bvhManager->Traverse(bvhManager->root, ray);
}

void Raytracer::IntersectScene(RayPacket& packet)
{
    for (int i = 0; i < PACKET_SIZE * PACKET_SIZE; ++i)
    {
        packet.rays[i].t = 9999999.f;
        packet.rays[i].hit = nullptr;
        packet.rays[i].rD = 1.f / packet.rays[i].D;
        packet.rays[i].o4 = _mm_set_ps(0.0f, packet.rays[i].O.z, packet.rays[i].O.y, packet.rays[i].O.x);
        packet.rays[i].rD4 = _mm_set_ps(0.0f, packet.rays[i].rD.z, packet.rays[i].rD.y, packet.rays[i].rD.x);
    }
    bvhManager->Traverse(packet);
}

void Raytracer::IntersectScene(RayPacket4& packet)
{
    __m256 ONE4 = _mm256_set1_ps(1.f);

    for (int i = 0; i < 8; ++i)
    {
        packet.rays[i].t4 = _mm256_set1_ps(9999999.f);
        packet.rays[i].hit4 = _mm256_set1_epi32(-1);

        packet.rays[i].rDx4 = _mm256_div_ps(ONE4, packet.rays[i].Dx4);
        packet.rays[i].rDy4 = _mm256_div_ps(ONE4, packet.rays[i].Dy4);
        packet.rays[i].rDz4 = _mm256_div_ps(ONE4, packet.rays[i].Dz4);
    }
    bvhManager->Traverse(packet);
}

void Raytracer::TracePacket(RayCounter& counter, int packetID, RayPacket& packet)
{
    int packetx = packetID % (SCRWIDTH / PACKET_SIZE);
    int packety = packetID / (SCRWIDTH / PACKET_SIZE);

    Pixel* packetbuff = screen->GetBuffer();
    Pixel color;

    float dx = 1.f / SCRWIDTH;
    float dy = 1.f / SCRHEIGHT;

    DataPacket data;

    // Set up RayPacket and DataPacket
    for (int i = 0; i < PACKET_SIZE*PACKET_SIZE; i++)
    {
        int r = morton.toLinearEIGHT[i];
        int x = (packetx * PACKET_SIZE) + (r % PACKET_SIZE);
        int y = (packety * PACKET_SIZE) + (r / PACKET_SIZE);

        vec3 p = p0 + static_cast<float>(x) * (p1 - p0) * dx +
            static_cast<float>(y) * (p2 - p0) * dy;
        packet.rays[i].O = camPos;
        packet.rays[i].D = normalize(p - camPos);
    }

    if (debugBVHType)	// not using packets for debugging
    {
        for (int i = 0; i < PACKET_SIZE * PACKET_SIZE; i++)
        {
            int r = morton.toLinearEIGHT[i];

            int x = (packetx * PACKET_SIZE) + (r % PACKET_SIZE);
            int y = (packety * PACKET_SIZE) + (r / PACKET_SIZE);
            Pixel* buf = packetbuff + (x)+(y * SCRWIDTH);

            // Background
            color = 0x87cefa;

            packet.rays[i].t = 0.0f;
            packet.rays[i].rD = 1.0f / packet.rays[i].D;
            packet.rays[i].o4 = _mm_set_ps(0.0f, packet.rays[i].O.z, packet.rays[i].O.y, packet.rays[i].O.x);
            packet.rays[i].rD4 = _mm_set_ps(0.0f, packet.rays[i].rD.z, packet.rays[i].rD.y, packet.rays[i].rD.x);
            bvhManager->TraverseDepth(tmpindex, bvhManager->root, packet.rays[i]);

            packet.rays[i].t = 0.0f;
            packet.rays[i].rD = 1.0f / packet.rays[i].D;
            packet.rays[i].o4 = _mm_set_ps(0.0f, packet.rays[i].O.z, packet.rays[i].O.y, packet.rays[i].O.x);
            packet.rays[i].rD4 = _mm_set_ps(0.0f, packet.rays[i].rD.z, packet.rays[i].rD.y, packet.rays[i].rD.x);

            vec4 col;
            if (debugBVHType == 1)
            {
                bvhManager->TraverseDepth(tmpindex, bvhManager->root, packet.rays[i]);
                col = vec4(packet.rays[i].t);
            }
            if (debugBVHType == 2)
            {
                bvhManager->TraverseNear(tmpindex, bvhManager->root, packet.rays[i]);
                col.g = packet.rays[i].t * 4;
            }

            if (packet.rays[i].t != 0.0f)
            {
                col = min(col, vec4(255.0f));
                Pixel pix = ((unsigned char)(col.a) << 24) | ((unsigned char)(col.r) << 16) | ((unsigned char)(col.g) << 8) | ((unsigned char)(col.b));
                color = pix;
            }

            *buf = color;
        }
    }
    else
    {
        // Cast primary rays
        counter.PrimalNormalRays += PACKET_SIZE*PACKET_SIZE;
        CastRay(counter, data, packet);

        Pixel* buf = packetbuff + packetx * PACKET_SIZE + packety * PACKET_SIZE * SCRWIDTH;

        // Apply colors to buffer
        for (int i = 0; i < PACKET_SIZE; i++)
        {
            memcpy(buf, &data.color[i * PACKET_SIZE], PACKET_SIZE * sizeof(Pixel));
            buf += SCRWIDTH;
        }
    }
}

void Raytracer::TracePacket(RayCounter& counter, int packetID, RayPacket4& packet)
{
    int packetx = packetID % (SCRWIDTH >> 3);
    int packety = packetID / (SCRWIDTH >> 3);

    Pixel* packetbuff = screen->GetBuffer();
    Pixel color;

    float dx = 1.f / SCRWIDTH;
    float dy = 1.f / SCRHEIGHT;

    DataPacket data;

    if (debugBVHType)	// not using packets for debugging
    {
        Ray ray;
        for (int i = 0; i < 64; i++)
        {
            int r = morton.toLinearEIGHT[i];

            int x = (packetx * PACKET_SIZE) + (r % PACKET_SIZE);
            int y = (packety * PACKET_SIZE) + (r / PACKET_SIZE);
            vec3 p = p0 + static_cast<float>(x) * (p1 - p0) * dx +
                static_cast<float>(y) * (p2 - p0) * dy;
            Pixel* buf = packetbuff + (x)+(y * SCRWIDTH);

            // Background
            color = 0x87cefa;

            ray.t = 0.0f;
            ray.O = camPos;
            ray.D = normalize(p - ray.O);
            ray.rD = 1.0f / ray.D;
            ray.o4 = _mm_set_ps(0.0f, ray.O.z, ray.O.y, ray.O.x);
            ray.rD4 = _mm_set_ps(0.0f, ray.rD.z, ray.rD.y, ray.rD.x);

            vec4 col;
            if (debugBVHType == 1)
            {
                bvhManager->TraverseDepth(tmpindex, bvhManager->root, ray);
                col = vec4(ray.t);
            }
            if (debugBVHType == 2)
            {
                bvhManager->TraverseNear(tmpindex, bvhManager->root, ray);
                col.g = ray.t * 4;
            }

            if (ray.t != 0.0f)
            {
                col = min(col, vec4(255.0f));
                Pixel pix = ((unsigned char)(col.a) << 24) | ((unsigned char)(col.r) << 16) | ((unsigned char)(col.g) << 8) | ((unsigned char)(col.b));
                color = pix;
            }

            *buf = color;
        }
    }
    else
    {
        // Set up RayPacket and DataPacket
        for (int i = 0; i < 64; i++)
        {
            int r = morton.toLinearEIGHT[i];
            int x = (packetx * PACKET_SIZE) + (r % PACKET_SIZE);
            int y = (packety * PACKET_SIZE) + (r / PACKET_SIZE);

            vec3 p = p0 + static_cast<float>(x) * (p1 - p0) * dx +
                static_cast<float>(y) * (p2 - p0) * dy;
            packet.rays[i >> 3].Ox[i & 7] = camPos.x;
            packet.rays[i >> 3].Oy[i & 7] = camPos.y;
            packet.rays[i >> 3].Oz[i & 7] = camPos.z;

            p = normalize(p - camPos);
            packet.rays[i >> 3].Dx[i & 7] = p.x;
            packet.rays[i >> 3].Dy[i & 7] = p.y;
            packet.rays[i >> 3].Dz[i & 7] = p.z;
        }

        // Cast primary rays
        counter.PrimalNormalRays += PACKET_SIZE*PACKET_SIZE;
        CastRay(counter, data, packet);

        Pixel* buf = packetbuff + packetx * 8 + packety * 8 * SCRWIDTH;

        // Apply colors to buffer
        for (int i = 0; i < 8; i++)
        {
            memcpy(buf, &data.color[i * 8], 8 * sizeof(Pixel));
            buf += SCRWIDTH;
        }
    }
}

vec4 Raytracer::SampleSky(Ray& ray)
{
    Surface* texture = skySphere;
    vec2 uv = GetUVPoints(-ray.D, 0.0f);
    int uvx = (int)(uv.x * texture->GetWidth()) % texture->GetWidth();
    if (uvx < 0) uvx = (texture->GetWidth() + uvx);
    int uvy = (int)(uv.y * texture->GetHeight()) % texture->GetHeight();
    if (uvy < 0) uvy = (texture->GetHeight() + uvy);
    return PixelToColor(texture->GetBuffer()[uvx + uvy * texture->GetWidth()]);
}

vec4 Raytracer::CalcShadedColor(RayCounter& counter, HitData& data, Ray& ray)
{
    vec4 c = data.mat->CalcDiffuse(data.uv);

    // test here if color is (nearly) black for an early out
    if (c.r < 0.001f && c.g < 0.001f && c.b < 0.001f)
    {
        return vec4{ 0.f };
    }

    vec4 lightCol{ 0.f };
    ApplyLights(counter, data, ray, lightCol);
    return c * lightCol;
}

vec4 Raytracer::CastRay(RayCounter& counter, HitData& data, Ray& ray)
{
    IntersectScene(ray);
    data.iter++;
    if (ray.hit)
    {
        if (data.iter > MAX_REFLECTION_BOUNCES)
        {
            return vec4{ 0.f };
        }

        // Find Material, HitNormal, and uv coords of hit
        CalcHitData(counter, data, ray);

        float reflectivity = data.mat->reflectivity;
        float transparency = data.mat->transparency;
        vec4 reflectColor{ 0.f };
        vec4 refractColor{ 0.f };
        vec4 selfColor{ 0.f };

        if (dot(ray.D, data.hitNormal) < 0)
        {
            // Calculate selfColor BEFORE casting to reflection and refraction,
            //	because 'data' is modified there
            if (reflectivity < 0.999f || transparency < 0.999f)
            {
                selfColor = CalcShadedColor(counter, data, ray);
            }

            // Apply reflection
            if (reflectivity > 0.001f)
            {
                reflectColor = Reflect(counter, data, ray);
                selfColor *= (1.f - reflectivity);
            }
        }

        // Apply transparency/refraction
        if (transparency > 0.001f)
        {
            refractColor = Refract(counter, data, ray);
            selfColor *= (1.f - transparency);
        }

        return selfColor + reflectColor + refractColor;
    }

    // No hit. Terminate with skysphere
    return PixelToColor(0x87cefa);//SampleSky(ray);
}

vec4 Raytracer::CastRayBasic(RayCounter& counter, HitData& data, Ray& ray)
{
    IntersectScene(ray);
    if (ray.hit)
    {
        // Find Material, HitNormal, and uv coords of hit
        CalcHitData(counter, data, ray);
        return CalcShadedColor(counter, data, ray);
    }

    // No hit. Terminate with skysphere
    return SampleSky(ray);
}

void Raytracer::CastRay(RayCounter& counter, DataPacket& data, RayPacket& rays)
{
    IntersectScene(rays);
    CalcHitData(counter, data, rays);

    for (int i = 0; i < PACKET_SIZE * PACKET_SIZE; ++i)
    {
        data.hits[i].iter++;

        if (rays.rays[i].hit)
        {
            if (data.hits[i].iter > MAX_REFLECTION_BOUNCES + 1)
            {
                data.color[i] = 0;
                continue;
            }

            float reflectivity = data.hits[i].mat->reflectivity;
            float transparency = data.hits[i].mat->transparency;
            vec4 reflectColor{ 0.f };
            vec4 refractColor{ 0.f };
            vec4 selfColor{ 0.f };

            if (dot(rays.rays[i].D, data.hits[i].hitNormal) < 0)
            {
                // Calculate selfColor BEFORE casting to reflection and refraction,
                //	because 'data' is modified there
                if (reflectivity < 0.999f || transparency < 0.999f)
                {
                    selfColor = CalcShadedColor(counter, data.hits[i], rays.rays[i]);
                }

                // Apply reflection
                if (reflectivity > 0.001f)
                {
                    reflectColor = Reflect(counter, data.hits[i], rays.rays[i]);
                    selfColor *= (1.f - reflectivity);
                }
            }

            // Apply transparency/refraction
            if (transparency > 0.001f)
            {
                refractColor = Refract(counter, data.hits[i], rays.rays[i]);
                selfColor *= (1.f - transparency);
            }

            data.color[morton.toLinearEIGHT[i]] = ColorToPixel(clamp(selfColor + reflectColor + refractColor, vec4{ 0.f }, vec4{ 255.f }));
            continue;
        }

        data.color[morton.toLinearEIGHT[i]] = 0x87cefa;
    }
}

void Raytracer::CastRay(RayCounter& counter, DataPacket& data, RayPacket4& rays)
{
    IntersectScene(rays);
    CalcHitData(counter, data, rays);

    union
    {
        __m256i reflectors8[8];
        std::uint32_t reflectors[64];
    };
    union
    {
        __m256i refractors8[8];
        std::uint32_t refractors[64];
    };

    Ray ray;
    __m256 ZERO8 = _mm256_setzero_ps();
    __m256 ONE8 = _mm256_set1_ps(1.f);
    for (int i = 0; i < 8; ++i)
    {
        HitData* hd = &data.hits[i * 8];
        hd->iter++;
        hd++; hd->iter++;
        hd++; hd->iter++;
        hd++; hd->iter++;
        hd++; hd->iter++;
        hd++; hd->iter++;
        hd++; hd->iter++;
        hd++; hd->iter++;

        // Figure out hitPoints that need reflection and refraction
        __m256 h = _mm256_cvtepi32_ps(data.hasHit8[i]);
        reflectors8[i] = _mm256_cvtps_epi32(_mm256_and_ps(_mm256_and_ps(_mm256_cmp_ps(data.reflectivity8[i], ZERO8, _CMP_GT_OQ), ONE8), h));
        refractors8[i] = _mm256_cvtps_epi32(_mm256_and_ps(_mm256_and_ps(_mm256_cmp_ps(data.transparency8[i], ZERO8, _CMP_GT_OQ), ONE8), h));
    }

    // Shade the hitpoints
    ProcessPointLights(counter, data, rays);

    // apply reflectivity and refraction
    for (int i = 0; i < 64; ++i)
    {
        if (data.hits[i].iter > MAX_REFLECTION_BOUNCES + 1)
        {
            data.color[i] = 0;
            continue;
        }
        else if (data.hasHit[i] == 0)
        {
            data.color[morton.toLinearEIGHT[i]] = 0x87cefa;
            continue;
        }

        int a = i >> 3;		// i / 8
        int b = i & 7;		// i % 8
        ray.hit = data.hits[i].hit;
        ray.D = std::move(vec3{ rays.rays[a].Dx[b], rays.rays[a].Dy[b], rays.rays[a].Dz[b] });
        ray.O = std::move(vec3{ rays.rays[a].Ox[b], rays.rays[a].Oy[b], rays.rays[a].Oz[b] });
        ray.t = rays.rays[a].t[b];

        vec4 reflectColor{ 0.f };
        vec4 refractColor{ 0.f };
        vec4 selfColor{ data.r[i], data.g[i], data.b[i], data.a[i] };

        // Apply reflection
        if (reflectors[i] != 0 && dot(ray.D, data.hits[i].hitNormal) < 0)
        {
            reflectColor = Reflect(counter, data.hits[i], ray);
            selfColor *= (1.f - data.reflectivity[i]);
        }

        // Apply transparency/refraction
        if (refractors[i] != 0)
        {
            refractColor = Refract(counter, data.hits[i], ray);
            selfColor *= (1.f - data.transparency[i]);
        }

        data.color[morton.toLinearEIGHT[i]] = ColorToPixel(clamp(selfColor + reflectColor + refractColor, vec4{ 0.f }, vec4{ 255.f }));
    }
}

void Raytracer::CalcHitData(RayCounter& counter, DataPacket& datas, RayPacket& rays)
{
    for (int i = 0; i < PACKET_SIZE * PACKET_SIZE; ++i)
    {
        HitData& data = datas.hits[i];
        Ray& ray = rays.rays[i];
        if (ray.hit)
        {
            data.hitPoint = ray.O + ray.t * ray.D;
            if (ray.hit->type == Primitive::SPHERE)
            {
                data.mat = ray.hit->mat;
                data.hitNormal = normalize(data.hitPoint - *ray.hit->pos[0]);
                data.uv = GetUVPoints(data.hitNormal, tmpTheta);
            }
            else
            {
                Mesh* mesh = (Mesh*)ray.hit->owner;
                Model* model = mesh->model;
                data.mat = ray.hit->mat;
                int i = ray.hitIndex; // triangle
                int v1 = mesh->tri[i * 3 + 0];
                int v2 = mesh->tri[i * 3 + 1];
                int v3 = mesh->tri[i * 3 + 2];
                vec3 interpolation = FindInterpolation(model->tpos[v1], model->tpos[v2], model->tpos[v3], data.hitPoint);
                data.hitNormal = model->tnorm[v1] * interpolation.x + model->tnorm[v2] * interpolation.y + model->tnorm[v3] * interpolation.z;
                data.uv = mesh->uv[v1] * interpolation.x + mesh->uv[v2] * interpolation.y + mesh->uv[v3] * interpolation.z;
            }
        }
    }
}

void Raytracer::CalcHitData(RayCounter& counter, DataPacket& datas, RayPacket4& rays)
{
    for (int i = 0; i < 8; ++i)
    {
        Ray4& ray4 = rays.rays[i];
        datas.hitPointX8[i] = _mm256_add_ps(ray4.Ox4, _mm256_mul_ps(ray4.t4, ray4.Dx4));
        datas.hitPointY8[i] = _mm256_add_ps(ray4.Oy4, _mm256_mul_ps(ray4.t4, ray4.Dy4));
        datas.hitPointZ8[i] = _mm256_add_ps(ray4.Oz4, _mm256_mul_ps(ray4.t4, ray4.Dz4));

        for (int j = 0; j < 8; ++j)
        {
            HitData& data = datas.hits[i * 8 + j];
            datas.hasHit[i * 8 + j] = 0;
            if (ray4.hit[j] > -1)
            {
                datas.hasHit[i * 8 + j] = 1;
                data.hit = &primitives[static_cast<int>(ray4.hit[j])];
                data.hitPoint = vec3{ datas.hitPointX[i * 8 + j], datas.hitPointY[i * 8 + j], datas.hitPointZ[i * 8 + j] };
                if (data.hit->type == Primitive::SPHERE)
                {
                    data.mat = data.hit->mat;
                    data.hitNormal = normalize(data.hitPoint - *data.hit->pos[0]);
                    datas.normalX[i * 8 + j] = data.hitNormal.x;
                    datas.normalY[i * 8 + j] = data.hitNormal.y;
                    datas.normalZ[i * 8 + j] = data.hitNormal.z;
                    datas.reflectivity[i * 8 + j] = data.hit->mat->reflectivity;
                    datas.transparency[i * 8 + j] = data.hit->mat->transparency;
                    datas.specularityExponent[i * 8 + j] = data.hit->mat->specularityExponent;
                    datas.specularityScale[i * 8 + j] = data.hit->mat->specularityScale;
                    data.uv = GetUVPoints(data.hitNormal, tmpTheta);
                }
                else
                {
                    Mesh* mesh = (Mesh*)data.hit->owner;
                    Model* model = mesh->model;
                    data.mat = data.hit->mat;
                    int k = ray4.hitIndex[j]; // triangle
                    int v1 = mesh->tri[k * 3 + 0];
                    int v2 = mesh->tri[k * 3 + 1];
                    int v3 = mesh->tri[k * 3 + 2];
                    vec3 interpolation = FindInterpolation(model->tpos[v1], model->tpos[v2], model->tpos[v3], data.hitPoint);
                    data.hitNormal = model->tnorm[v1] * interpolation.x + model->tnorm[v2] * interpolation.y + model->tnorm[v3] * interpolation.z;
                    datas.normalX[i * 8 + j] = data.hitNormal.x;
                    datas.normalY[i * 8 + j] = data.hitNormal.y;
                    datas.normalZ[i * 8 + j] = data.hitNormal.z;
                    datas.reflectivity[i * 8 + j] = data.hit->mat->reflectivity;
                    datas.transparency[i * 8 + j] = data.hit->mat->transparency;
                    datas.specularityExponent[i * 8 + j] = data.hit->mat->specularityExponent;
                    datas.specularityScale[i * 8 + j] = data.hit->mat->specularityScale;
                    data.uv = mesh->uv[v1] * interpolation.x + mesh->uv[v2] * interpolation.y + mesh->uv[v3] * interpolation.z;
                }
                vec4 c = data.mat->CalcDiffuse(data.uv);
                datas.r[i * 8 + j] = c.r;
                datas.g[i * 8 + j] = c.g;
                datas.b[i * 8 + j] = c.b;
                datas.a[i * 8 + j] = c.a;
            }
        }
    }
}

vec4 Raytracer::Reflect(RayCounter& counter, HitData& data, Ray& ray)
{
    counter.ReflectionRays++;

    ray.D = ray.D - 2.f * dot(ray.D, data.hitNormal) * data.hitNormal;
    ray.O = data.hitPoint + 0.001f * ray.D;

    float ref = data.mat->reflectivity;
    return ref * CastRay(counter, data, ray);
}

vec4 Raytracer::Refract(RayCounter& counter, HitData& data, Ray& ray)
{
    vec3 normal = data.hitNormal;
    vec3 dir = ray.D;
    float di = dot(dir, normal);
    float n1, n2;
    bool inAir = true;
    if (di >= 0)
    {
        // Inside the mesh
        normal = -normal;
        di = -di;	// Don't forget to invert the dot product too :P
        n1 = data.mat->refractiveIndex;
        n2 = 1.f;
        inAir = false;
    }
    else
    {
        // In air
        n1 = 1.f;
        n2 = data.mat->refractiveIndex;
    }
    float cosI = -di;	// di < 0  ==>  cosI > 0
    float n = n1 / n2;
    float sinTsqrd = n * n * (1.f - cosI * cosI);
    float cosTsqrd = 1.0f - sinTsqrd;

    vec3 reflectDir = dir + 2.f * cosI * normal;


    vec3 hitPoint = data.hitPoint;
    float transparency = data.mat->transparency;
    float reflectAmount = 1.f;
    float refractAmount = 0.f;
    vec4 refractCol{ 0.f };
    vec4 reflectCol{ 0.f };
    if (cosTsqrd > 0)
    {
        float cosT = sqrtf(cosTsqrd);
        // We have refraction as well.
        // Use Fresnel equations to find refraction and reflection contributions
        vec3 refractDir = n * dir + normal * (n * cosI - cosT);
        float Rperp = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
        float Rpar = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
        reflectAmount = 0.5f * (Rperp * Rperp + Rpar * Rpar);
        refractAmount = 1.f - reflectAmount;

        ray.D = refractDir;
        ray.O = hitPoint + 0.001f * refractDir;

        counter.RefractionRays++;
        refractCol = CastRay(counter, data, ray);
    }
    if (useInternalReflections || inAir)
    {
        data.iter = (refractAmount > 0 ? data.iter - 1 : data.iter);
        ray.D = reflectDir;
        ray.O = hitPoint + 0.001f * reflectDir;

        counter.ReflectionRays++;
        reflectCol = CastRay(counter, data, ray);
    }

    return transparency * (reflectAmount * reflectCol + refractAmount * refractCol);
}

void Raytracer::CalcHitData(RayCounter& counter, HitData& data, Ray& ray)
{
    data.hitPoint = ray.O + ray.t * ray.D;
    if (ray.hit->type == Primitive::SPHERE)
    {
        data.mat = ray.hit->mat;
        data.hitNormal = normalize(data.hitPoint - *ray.hit->pos[0]);
        data.uv = GetUVPoints(data.hitNormal, tmpTheta);
    }
    else
    {
        Mesh* mesh = (Mesh*)ray.hit->owner;
        Model* model = mesh->model;
        data.mat = ray.hit->mat;
        int i = ray.hitIndex; // triangle
        int v1 = mesh->tri[i * 3 + 0];
        int v2 = mesh->tri[i * 3 + 1];
        int v3 = mesh->tri[i * 3 + 2];
        vec3 interpolation = FindInterpolation(model->tpos[v1], model->tpos[v2], model->tpos[v3], data.hitPoint);
        data.hitNormal = model->tnorm[v1] * interpolation.x + model->tnorm[v2] * interpolation.y + model->tnorm[v3] * interpolation.z;
        data.uv = mesh->uv[v1] * interpolation.x + mesh->uv[v2] * interpolation.y + mesh->uv[v3] * interpolation.z;
    }
}
