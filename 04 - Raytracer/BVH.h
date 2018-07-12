#pragma once

#include "Ray.h"
#include "RayPacket.h"

namespace Tmpl8
{
	// 0 = None
	// 1 = Median
	// 2 = SAH
#define BVH_SPLIT_TYPE 2

	// More precision = better real-time performance and longer construction time
#define BVH_SAH_SPLIT_PRECISION 7

	struct RAABB
	{
		vec3& min;
		vec3& max;
		RAABB(vec3& min, vec3& max)
			: min(min), max(max)
		{ }
	};

	struct AABB
	{
		vec3 min;
		vec3 max;

		AABB() { }
		AABB(RAABB& raabb) { this->min = raabb.min; this->max = raabb.max; }
	};

	struct SAH
	{
		AABB left;
		int leftCount;
		float leftCost;

		AABB right;
		int rightCount;
		float rightCost;

		AABB bestBounds;
		float cost;
		bool worth;
	};

	// 32 bytes total
	struct BVHNode
	{
		union
		{
			struct
			{
				vec3 bmin; // 12 bytes
				int leftFirst; // 4 bytes
			};
			__m128 bmin4;
		};
		union
		{
			struct
			{
				vec3 bmax; // 12 bytes
				int count; // 4 bytes
			};
			__m128 bmax4;
		};

		RAABB bounds() { return RAABB(bmin, bmax); }
	};

	class BVHManager
	{
	public:
		BVHManager();
		~BVHManager();

		void SetBinaryPath(char* path);

		// Construct the BVH
		void Construct(struct Primitive* primitives, int count, bool forcerebuild = false);

		// Sort the BVH based on camera position and direction
		void Sort(vec3& camPos, vec3& camDir, BVHNode* node);

		void Traverse(Ray& ray) CONST;
		void Traverse(RayPacket& packet) CONST;
		void Traverse(RayPacket4& packet) CONST;
		void Traverse(BVHNode* node, Ray& ray) CONST;
		void Traverse(BVHNode* node, RayPacket& packet) CONST;
		void Traverse4(BVHNode* node, RayPacket4& packet, int firstActive) CONST;
		void TraverseDepth(int index, BVHNode* node, Ray& ray) CONST;
		void TraverseNear(int index, BVHNode* node, Ray& ray) CONST;
		bool IsOccluded(BVHNode* node, Ray& ray) CONST;
		void IsOccluded4(BVHNode* node, RayPacket4& packet, int firstActive, __m256* masks) CONST;

		// Counters
		uint trackMaxSize;
		uint maxDepth;

		uint countLeafes;
		double averagePrimitives;

		// Data
		std::string path;

		struct Primitive* primitives;
		BVHNode* nodePool;
		uint* indices;
		uint nodePoolSize;
		uint nodePoolptr;
		BVHNode* root;

	private:
		void ConstructNode(BVHNode* node, float parentSAHCost, int depth = 0);
		AABB CalculateBounds(int first, int count) CONST;

		void SubdivideMedian(BVHNode* node);
		void SubdivideSAH(SAH& sah, BVHNode* node, int first, int count, float parentSAHCost);
		void CalculateSplitBounds(SAH& sah, RAABB withinBounds, int first, int count);

		int Partition(RAABB rbounds, int first, int count);

		void IntersectsPrimitives(Ray& ray, BVHNode* node) CONST;
		void IntersectsPrimitives(Ray4& rays, BVHNode* node) CONST;
		void IntersectsPrimitivesEarlyOut(Ray& ray, BVHNode* node) CONST;
		void IntersectsPrimitivesEarlyOut(Ray4& rays, BVHNode* node, __m256& mask) CONST;
	};
}
