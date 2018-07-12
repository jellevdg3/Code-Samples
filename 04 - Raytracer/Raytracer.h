#pragma once

namespace Tmpl8
{
#define WORKING_THREADS 8
#define MAX_REFLECTION_BOUNCES 4

	// -----------------------------------------------------------
	// Raytracer class
	// -----------------------------------------------------------
	class Raytracer
	{
	public:
		// Constructor / destructor
		Raytracer();
		~Raytracer();

		// Methods
		void Init(Surface* screen, Scene* scene);
		void ConstructBVH(bool forcerebuild = false);

		void IntersectScene(Ray& ray);
		void IntersectScene(RayPacket& packet);
		void IntersectScene(RayPacket4& packet);

		void TraceLine(RayCounter& counter, int y, HitData& data);
		void TraceLineBasic(RayCounter& counter, int y, HitData& data);
		void TracePacket(RayCounter& counter, int packetID, RayPacket& packet);
		void TracePacket(RayCounter& counter, int packetID, RayPacket4& packet);

		vec4 SampleSky(Ray& ray);
		vec4 CalcShadedColor(RayCounter& counter, HitData& data, Ray& ray);
		vec4 Reflect(RayCounter& counter, HitData& data, Ray& ray);
		vec4 Refract(RayCounter& counter, HitData& data, Ray& ray);
		
		vec4 CastRay(RayCounter& counter, HitData& data, Ray& ray);
		vec4 CastRayBasic(RayCounter& counter, HitData& data, Ray& ray);

		void CastRay(RayCounter& counter, DataPacket& data, RayPacket& rays);
		void CastRay(RayCounter& counter, DataPacket& data, RayPacket4& rays);
		void CalcHitData(RayCounter& counter, DataPacket& datas, RayPacket& rays);
		void CalcHitData(RayCounter& counter, DataPacket& datas, RayPacket4& rays);

	private:
		// Data members
		Surface* screen;
		Scene* scene;

		int tmpindex;

		bool running;
		Surface* skySphere;
		struct RayTracerThreads* threads;

		class BVHManager* bvhManager;
		int primitiveCount;
		struct Primitive* primitives;

		int debugBVHType;

		// Raycast values
		int rayLineTasks;
		int rayTotalTasks;
		int rayLineCompletedTasks;
		float tmpTheta;

		float deltaRenderCompletion;
		LONGLONG lastRenderCompleted;

		vec3 p0;
		vec3 p1;
		vec3 p2;
		vec3 camPos;

		bool useReflections = true;
		bool useInternalReflections = true;
		bool useSimd = true;

		int completedThreads;
	};
}; // namespace Tmpl8
