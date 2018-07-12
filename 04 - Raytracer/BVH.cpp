#include "template.h"
#include "BVH.h"

#include "Utils.h"
#include "Primitive.h"
#include "BufferedStream.h"

#include <stdio.h>
#include <stdlib.h>

#define ROOT_INIT_COST 999999999999999999.0

inline bool IsPrimitiveWithinBounds(Primitive* priml, RAABB& bounds)
{
	// Calculate center point
	vec3 point = vec3(0.0f);
	for(int i = 0; i < priml->poscount; i++)
	{
		point += *priml->pos[i];
	}
	point /= priml->poscount;

	if(point.x >= bounds.min.x && point.y >= bounds.min.y && point.z >= bounds.min.z
	   && point.x <= bounds.max.x && point.y <= bounds.max.y && point.z <= bounds.max.z)
	{
		return true;
	}

	return false;
}

float CalculateBoxSurfaceAreaHalf(AABB& box)
{
	float w = box.max.x - box.min.x;
	float h = box.max.y - box.min.y;
	float l = box.max.z - box.min.z;

	return (w * h) + (h * l) + (l * w);
}

inline bool IntersectsNode(Ray& ray, BVHNode* node)
{
	return CheckBox4(node->bmin4, node->bmax4, ray.o4, ray.rD4);
}

inline MinMax IntersectsNodeMinMax(Ray& ray, BVHNode* node)
{
	return CheckBoxMinMax4(node->bmin4, node->bmax4, ray.o4, ray.rD4);
}

BVHManager::BVHManager()
{
	trackMaxSize = 0;
	indices = nullptr;
	nodePool = nullptr;
	root = nullptr;
	path = "";
}

BVHManager::~BVHManager()
{
	if(indices != nullptr) _aligned_free(indices); indices = nullptr;
	if(nodePool != nullptr) _aligned_free(nodePool); nodePool = nullptr;
}

void Tmpl8::BVHManager::SetBinaryPath(char* path)
{
	this->path = path;
}

void BVHManager::Construct(struct Primitive* primitives, int count, bool forcerebuild)
{
	this->primitives = primitives;

	nodePoolptr = 1;
	nodePoolSize = (count * 4) + 1;

	if((uint)count > trackMaxSize || trackMaxSize == 0)
	{
		trackMaxSize = count;

		// Clean up old data
		if(indices != nullptr) _aligned_free(indices);
		if(nodePool != nullptr) _aligned_free(nodePool);

		// Create index array
		indices = (uint*)_aligned_malloc(sizeof(uint) * nodePoolSize, sizeof(uint));

		// Allocate BVH root node
		nodePool = (BVHNode*)_aligned_malloc(sizeof(BVHNode) * (LONGLONG)(nodePoolSize), sizeof(BVHNode));
	}

	for(int i = 0; i < count; i++)
	{
		indices[i] = i;
	}

	// Counters
	maxDepth = 0;

	countLeafes = 0;
	averagePrimitives = 0.0f;

	// Subdivide root node
	root = nodePool;
	root->leftFirst = 0;
	root->count = count;

	// Binary saving/loading
	if (path.length() != 0)
	{
		{
			BufferedStream stream((char*)(path.c_str()));
			if (!forcerebuild && stream.exists())
			{
				// Load BVH
				stream.startReading();
				unsigned long long poolsize = stream.readULong();

				// Validate
				if (poolsize == nodePoolSize)
				{
					this->nodePoolptr = stream.readUInt();

					this->maxDepth = stream.readUInt();
					this->countLeafes = stream.readUInt();
					this->averagePrimitives = stream.readDouble();

					char* nodepool = stream.readArray(sizeof(BVHNode) * (LONGLONG)(nodePoolptr));
					char* indices = stream.readArray(sizeof(uint) * (LONGLONG)(nodePoolptr));

					std::memcpy(this->nodePool, nodepool, sizeof(BVHNode) * (LONGLONG)(nodePoolptr));
					std::memcpy(this->indices, indices, sizeof(uint) * (LONGLONG)(nodePoolptr));

					// Clean up
					delete[] nodepool;
					delete[] indices;

					stream.stopReading();

					return;
				}

				stream.stopReading();
				stream.remove();
			}
		}

		// Make new BVH
		ConstructNode(root, ROOT_INIT_COST);

		// Save
		{
			BufferedStream stream((char*)(path.c_str()));
			stream.create();
			stream.startWritting();
			stream.writeULong(nodePoolSize);
			stream.writeUInt(nodePoolptr);

			stream.writeUInt(maxDepth);
			stream.writeUInt(countLeafes);
			stream.writeDouble(averagePrimitives / countLeafes);

			stream.writeArray((char*)(nodePool), sizeof(BVHNode) * (LONGLONG)(nodePoolptr));
			stream.writeArray((char*)(indices), sizeof(uint) * (LONGLONG)(nodePoolptr));
			stream.flush();
			stream.stopWritting();
		}
	}
	else
	{
		// Make new BVH
		ConstructNode(root, ROOT_INIT_COST);
	}

	averagePrimitives /= countLeafes;
}

void BVHManager::Sort(vec3& camPos, vec3& camDir, BVHNode* node)
{
	if(node->count == -1)
	{
		BVHNode* leftnode = nodePool + node->leftFirst;
		BVHNode* rightnode = leftnode + 1;

		vec3 left = (((leftnode->bmin + leftnode->bmax) / 2.0f) - (-camDir * 1000000.0f));
		vec3 right = (((rightnode->bmin + rightnode->bmax) / 2.0f) - (-camDir * 1000000.0f));

		float distleft = dot(left, left);
		float distright = dot(right, right);

		if(distleft > distright)
		{
			__m128 tmpbmin4 = leftnode->bmin4;
			__m128 tmpbmax4 = leftnode->bmax4;
			leftnode->bmin4 = rightnode->bmin4;
			leftnode->bmax4 = rightnode->bmax4;
			rightnode->bmin4 = tmpbmin4;
			rightnode->bmax4 = tmpbmax4;
		}

		Sort(camPos, camDir, leftnode);
		Sort(camPos, camDir, rightnode);
	}
}

void BVHManager::Traverse(Ray& ray) CONST
{
	Traverse(root, ray);
}

void BVHManager::Traverse(RayPacket& packet) CONST
{
	Traverse(root, packet);
}

void BVHManager::Traverse(RayPacket4& packet) CONST
{
	Traverse4(root, packet, 0);
}

void BVHManager::Traverse(BVHNode* node, Ray& ray) CONST
{
	if(node->count == -1)
	{
		BVHNode* leftnode = nodePool + node->leftFirst;
		BVHNode* rightnode = leftnode + 1;

		MinMax minmaxleft = IntersectsNodeMinMax(ray, leftnode);
		MinMax minmaxright = IntersectsNodeMinMax(ray, rightnode);

		// Near (if sorted)
		if(minmaxleft.max >= minmaxleft.min && minmaxleft.max >= 0 && ray.t > minmaxleft.min)
		{
			Traverse(leftnode, ray);
		}
		// Far (if sorted)
		if(ray.t > minmaxright.min && minmaxright.max >= minmaxright.min && minmaxright.max >= 0)
		{
			Traverse(rightnode, ray);
		}
	}
	else
	{
		// Leaf
		IntersectsPrimitives(ray, node);
	}
}

struct TraversalStack { uint32 nodeId, firstActive; };
void BVHManager::Traverse(BVHNode* node, RayPacket& packet) CONST
{
	// Custom stack
	int stackPtr = 1;
	TraversalStack traversalStack[64];
	uint32 firstActive = 0;
	uint32 currentNode = 0;
	uint32 lastNode = 99999999;
	traversalStack[0].nodeId = currentNode; // terminator

	while(stackPtr)
	{
		if(currentNode == lastNode)
		{
			stackPtr--;
			currentNode = traversalStack[stackPtr].nodeId;
			firstActive = traversalStack[stackPtr].firstActive;
		}

		lastNode = currentNode;
		BVHNode* node = nodePool + currentNode;
		if(node->count == -1)
		{
			// Has children
			BVHNode* leftnode = nodePool + node->leftFirst;
			BVHNode* rightnode = leftnode + 1;

			int curFirstActive = firstActive;

			// Near
			for(int i = curFirstActive; i < PACKET_SIZE*PACKET_SIZE; i++)
			{
				// Near (if sorted)
				MinMax minmax = IntersectsNodeMinMax(packet.rays[i], leftnode);
				if(minmax.max >= minmax.min && minmax.max >= 0 && packet.rays[i].t > minmax.min)
				{
					currentNode = node->leftFirst;
					firstActive = i;
					break;
				}
			}

			// Far
			for(int i = curFirstActive; i < PACKET_SIZE*PACKET_SIZE; i++)
			{
				// Far (if sorted)
				MinMax minmax = IntersectsNodeMinMax(packet.rays[i], rightnode);
				if(packet.rays[i].t > minmax.min && minmax.max >= minmax.min && minmax.max >= 0)
				{
					traversalStack[stackPtr].nodeId = node->leftFirst + 1;
					traversalStack[stackPtr].firstActive = i;
					stackPtr++;
					break;
				}
			}
		}
		else
		{
			// Leaf brute force
			for(int i = firstActive; i < PACKET_SIZE*PACKET_SIZE; i++)
			{
				IntersectsPrimitives(packet.rays[i], node);
			}
		}
	}
}

void BVHManager::Traverse4(BVHNode* node, RayPacket4& packet, int firstActive) CONST
{
	if(node->count == -1)
	{
		BVHNode* leftnode = nodePool + node->leftFirst;
		BVHNode* rightnode = leftnode + 1;

		Node4 left4 =
		{
			_mm256_set1_ps(leftnode->bmin.x),
			_mm256_set1_ps(leftnode->bmin.y),
			_mm256_set1_ps(leftnode->bmin.z),
			_mm256_set1_ps(leftnode->bmax.x),
			_mm256_set1_ps(leftnode->bmax.y),
			_mm256_set1_ps(leftnode->bmax.z)
		};


		Node4 right4 =
		{
			_mm256_set1_ps(rightnode->bmin.x),
			_mm256_set1_ps(rightnode->bmin.y),
			_mm256_set1_ps(rightnode->bmin.z),
			_mm256_set1_ps(rightnode->bmax.x),
			_mm256_set1_ps(rightnode->bmax.y),
			_mm256_set1_ps(rightnode->bmax.z)
		};


		// Near (if sorted)
		for(int i = firstActive; i < 8; i++)
		{
			// Near (if sorted)
			if(CheckBox44(left4, packet.rays[i]))
			{
				Traverse4(leftnode, packet, i);
				break;
			}
		}

		// Far (if sorted)
		for(int i = firstActive; i < 8; i++)
		{
			// Far (if sorted)
			if(CheckBox44(right4, packet.rays[i]))
			{
				Traverse4(rightnode, packet, i);
				break;
			}
		}
	}
	else
	{
		// Leaf brute force
		for(int i = firstActive; i < 8; i++)
		{
			IntersectsPrimitives(packet.rays[i], node);
		}
	}
}

void BVHManager::TraverseDepth(int index, BVHNode* node, Ray& ray) CONST
{
	if(index >= 0 && index < nodePoolptr)
	{
		BVHNode* n = nodePool + index;
		if(!IntersectsNode(ray, n))
			return;
		else
		{
			ray.t = 125.0f;

			if(node->count == -1)
			{
				ray.t = 255.0f;
				IntersectsPrimitives(ray, n);
			}
		}
	}

	if(node->count == -1)
	{
		BVHNode* leftnode = nodePool + node->leftFirst;
		BVHNode* rightnode = leftnode + 1;

		MinMax minmaxleft = IntersectsNodeMinMax(ray, leftnode);
		MinMax minmaxright = IntersectsNodeMinMax(ray, rightnode);

		ray.t += 1.0f;

		// Near (if sorted)
		if(minmaxleft.max >= minmaxleft.min && minmaxleft.max >= 0)
		{
			TraverseDepth(index, leftnode, ray);
		}
		// Far (if sorted)
		if(minmaxright.min && minmaxright.max >= minmaxright.min && minmaxright.max >= 0)
		{
			TraverseDepth(index, rightnode, ray);
		}
	}
}

void BVHManager::TraverseNear(int index, BVHNode* node, Ray& ray) CONST
{
	if(node->count == -1)
	{
		BVHNode* leftnode = nodePool + node->leftFirst;
		BVHNode* rightnode = leftnode + 1;

		MinMax minmaxleft = IntersectsNodeMinMax(ray, leftnode);
		MinMax minmaxright = IntersectsNodeMinMax(ray, rightnode);

		// Near (if sorted)
		if(minmaxleft.max >= minmaxleft.min && minmaxleft.max >= 0 && ray.t > minmaxleft.min)
		{
			ray.t += 1.0f;
			TraverseNear(index, leftnode, ray);
		}
		// Far (if sorted)
		if(minmaxright.max >= minmaxright.min && minmaxright.max >= 0)
		{
			TraverseNear(index, rightnode, ray);
		}
	}
}

bool BVHManager::IsOccluded(BVHNode* node, Ray& ray) CONST
{
	if(!IntersectsNode(ray, node))
		return false;

	if(node->count == -1)
	{
		// Traverse nodes
		if(IsOccluded(nodePool + node->leftFirst, ray)) return true;
		if(IsOccluded(nodePool + node->leftFirst + 1, ray)) return true;
	}
	else
	{
		// Leaf
		IntersectsPrimitivesEarlyOut(ray, node);
		return ray.hit;
	}
	return false;
}

void BVHManager::IsOccluded4(BVHNode* node, RayPacket4& packet, int firstActive, __m256* masks) CONST
{
	if(node->count == -1)
	{
		BVHNode* leftnode = nodePool + node->leftFirst;
		BVHNode* rightnode = leftnode + 1;

		Node4 left4 =
		{
			_mm256_set1_ps(leftnode->bmin.x),
			_mm256_set1_ps(leftnode->bmin.y),
			_mm256_set1_ps(leftnode->bmin.z),
			_mm256_set1_ps(leftnode->bmax.x),
			_mm256_set1_ps(leftnode->bmax.y),
			_mm256_set1_ps(leftnode->bmax.z)
		};

		Node4 right4 =
		{
			_mm256_set1_ps(rightnode->bmin.x),
			_mm256_set1_ps(rightnode->bmin.y),
			_mm256_set1_ps(rightnode->bmin.z),
			_mm256_set1_ps(rightnode->bmax.x),
			_mm256_set1_ps(rightnode->bmax.y),
			_mm256_set1_ps(rightnode->bmax.z)
		};

		// Near (if sorted)
		for(int i = firstActive; i < 8; i++)
		{
			// Near (if sorted)
			if(CheckBox44(left4, packet.rays[i]))
			{
				IsOccluded4(leftnode, packet, i, masks);
				break;
			}
		}

		// Far (if sorted)
		for(int i = firstActive; i < 8; i++)
		{
			// Far (if sorted)
			if(CheckBox44(right4, packet.rays[i]))
			{
				IsOccluded4(rightnode, packet, i, masks);
				break;
			}
		}

		// Early out (if all masked)
		__m256 finalmask =
			_mm256_and_ps(masks[0],
						  _mm256_and_ps(masks[1],
										_mm256_and_ps(masks[2],
													  _mm256_and_ps(masks[3],
																	_mm256_and_ps(masks[4],
																				  _mm256_and_ps(masks[5],
																								_mm256_and_ps(masks[6],
																											  masks[7]
																								)))))));

		if(_mm256_movemask_ps(finalmask) == 64) return;
	}
	else
	{
		// Leaf brute force
		for(int i = firstActive; i < 8; i++)
		{
			IntersectsPrimitivesEarlyOut(packet.rays[i], node, masks[i]);

			// Early out (if all masked)
			__m256 finalmask =
				_mm256_and_ps(masks[0],
							  _mm256_and_ps(masks[1],
											_mm256_and_ps(masks[2],
														  _mm256_and_ps(masks[3],
																		_mm256_and_ps(masks[4],
																					  _mm256_and_ps(masks[5],
																									_mm256_and_ps(masks[6],
																												  masks[7]
																									)))))));

			if(_mm256_movemask_ps(finalmask) == 64) return;
		}
	}
}

void BVHManager::ConstructNode(BVHNode* node, float parentSAHCost, int depth)
{
	if((uint)(depth + 1) >= maxDepth) maxDepth = depth + 1;

	// Calculate bounds
	AABB bounds = CalculateBounds(node->leftFirst, node->count);
	node->bmin = bounds.min;
	node->bmax = bounds.max;

	// This lode is a leaf
	if(node->count < 3)
	{
		countLeafes++;
		averagePrimitives += node->count;
		return;
	}

	// Make this node into a root node
	if(nodePoolptr >= nodePoolSize - 1)
	{
		return;
	}

	int first = node->leftFirst;
	int count = node->count;

	node->leftFirst = nodePoolptr; nodePoolptr += 2;
	node->count = -1;

	BVHNode* left = nodePool + node->leftFirst;
	BVHNode* right = nodePool + node->leftFirst + 1;

	// Split up
	float parentSAHCostL = 0.0f;
	float parentSAHCostR = 0.0f;

	// Median
#if BVH_SPLIT_TYPE == 1
	SubdivideMedian(node);
#endif

	// SAH
#if BVH_SPLIT_TYPE == 2
	SAH SAHResult;
	SubdivideSAH(SAHResult, node, first, count, parentSAHCost);
	if(!SAHResult.worth)
	{
		// Abort, not worth it, convert into leaf
		node->leftFirst = first;
		node->count = count;
		nodePoolptr -= 2;

		countLeafes++;
		averagePrimitives += node->count;

		return;
	}

	left->bmin = SAHResult.bestBounds.min;
	left->bmax = SAHResult.bestBounds.max;

	parentSAHCostL = SAHResult.leftCost;
	parentSAHCostR = SAHResult.rightCost;
#endif

	// Partition
	int partitionPivot = Partition(left->bounds(), first, count);

	left->leftFirst = first;
	left->count = partitionPivot - first;

	right->leftFirst = partitionPivot;
	right->count = count - left->count;

#if BVH_SPLIT_TYPE == 1
	// Could not split up
	if(left->count == 0 || right->count == 0)
	{
		// TEMP FIX: Just cut it in half ignoring objects positions
		int mid = count / 2;

		left->leftFirst = first;
		left->count = mid;
		right->leftFirst = first + mid;
		right->count = count - left->count;
	}
#endif

	// Construct childs
	ConstructNode(left, parentSAHCostL, depth + 1);
	ConstructNode(right, parentSAHCostR, depth + 1);
}

AABB BVHManager::CalculateBounds(int first, int count) CONST
{
	AABB bounds;
	bounds.min = vec3(9999999.0f);
	bounds.max = vec3(-9999999.0f);

	bool contains = false;

	for(int i = first; i < first + count; i++)
	{
		Primitive* prim = primitives + indices[i];
		for(int j = 0; j < prim->poscount; j++)
		{
			bounds.min = min(*prim->pos[j] - prim->radius, bounds.min);
			bounds.max = max(*prim->pos[j] + prim->radius, bounds.max);

			contains = true;
		}
	}

	// Prevent infinite bounding boxes
	if(!contains)
	{
		bounds.min = vec3(0.0f);
		bounds.max = vec3(0.0f);
	}

	return bounds;
}

void BVHManager::SubdivideMedian(BVHNode* node)
{
	BVHNode* left = nodePool + node->leftFirst;
	BVHNode* right = nodePool + node->leftFirst + 1;

	// Split bounds
	RAABB rbounds = node->bounds();
	AABB bounds(rbounds);
	vec3 diff = rbounds.max - rbounds.min;
	if(diff.x >= diff.y && diff.x >= diff.z)
	{
		// split x
		bounds.max.x -= diff.x / 2.0f;
	}
	else if(diff.y >= diff.z)
	{
		// split y
		bounds.max.y -= diff.y / 2.0f;
	}
	else
	{
		// split z
		bounds.max.z -= diff.z / 2.0f;
	}

	left->bmin = bounds.min;
	left->bmax = bounds.max;

	right->bmin = bounds.max;
	right->bmax = rbounds.max;
}

void BVHManager::SubdivideSAH(SAH& sah, BVHNode* node, int first, int count, float parentSAHCost)
{
	// Parent bounds
	RAABB rParentBounds = node->bounds();

	// My bounds
	AABB bounds;
	RAABB rbounds(bounds.min, bounds.max);
	AABB bestBounds;

	bool hasBestArea = false;
	SAH bestSAH;

	// Check X splits
	for(int cx = 0; cx < BVH_SAH_SPLIT_PRECISION; cx++)
	{
		bounds.min = rParentBounds.min;
		bounds.max = rParentBounds.max;
		float diff = bounds.max.x - bounds.min.x;
		bounds.max.x -= (diff / BVH_SAH_SPLIT_PRECISION) * cx;

		CalculateSplitBounds(sah, rbounds, first, count);
		if(sah.cost < bestSAH.cost || !hasBestArea)
		{
			hasBestArea = true;
			bestSAH = sah;
			bestBounds = bounds;
		}
	}

	// Check Y splits
	for(int cy = 0; cy < BVH_SAH_SPLIT_PRECISION; cy++)
	{
		bounds.min = rParentBounds.min;
		bounds.max = rParentBounds.max;
		float diff = bounds.max.y - bounds.min.y;
		bounds.max.y -= (diff / BVH_SAH_SPLIT_PRECISION) * cy;

		CalculateSplitBounds(sah, rbounds, first, count);
		if(sah.cost < bestSAH.cost || !hasBestArea)
		{
			hasBestArea = true;
			bestSAH = sah;
			bestBounds = bounds;
		}
	}

	// Check Z splits
	for(int cz = 0; cz < BVH_SAH_SPLIT_PRECISION; cz++)
	{
		bounds.min = rParentBounds.min;
		bounds.max = rParentBounds.max;
		float diff = bounds.max.z - bounds.min.z;
		bounds.max.z -= (diff / BVH_SAH_SPLIT_PRECISION) * cz;

		CalculateSplitBounds(sah, rbounds, first, count);
		if(sah.cost < bestSAH.cost || !hasBestArea)
		{
			hasBestArea = true;
			bestSAH = sah;
			bestBounds = bounds;
		}
	}

	bestSAH.worth = bestSAH.cost < parentSAHCost;
	bestSAH.bestBounds = bestBounds;
	sah = bestSAH;
}

void BVHManager::CalculateSplitBounds(SAH& sah, RAABB withinBounds, int first, int count)
{
	sah.left.min = vec3(9999999.0f);
	sah.left.max = vec3(-9999999.0f);
	sah.leftCount = 0;

	sah.right.min = vec3(9999999.0f);
	sah.right.max = vec3(-9999999.0f);
	sah.rightCount = 0;

	RAABB left(sah.left.min, sah.left.max);
	RAABB right(sah.right.min, sah.right.max);

	for(int i = first; i < first + count; i++)
	{
		Primitive* prim = primitives + indices[i];
		if(IsPrimitiveWithinBounds(prim, withinBounds))
		{
			for(int j = 0; j < prim->poscount; j++)
			{
				sah.left.min = min(*prim->pos[j] - prim->radius, sah.left.min);
				sah.left.max = max(*prim->pos[j] + prim->radius, sah.left.max);
			}
			sah.leftCount++;
		}
		else
		{
			for(int j = 0; j < prim->poscount; j++)
			{
				sah.right.min = min(*prim->pos[j] - prim->radius, sah.right.min);
				sah.right.max = max(*prim->pos[j] + prim->radius, sah.right.max);
			}
			sah.rightCount++;
		}
	}

	// Prevent infinite bounding boxes
	if(sah.leftCount == 0)
	{
		sah.left.min = vec3(0.0f);
		sah.left.max = vec3(0.0f);
	}
	if(sah.rightCount == 0)
	{
		sah.right.min = vec3(0.0f);
		sah.right.max = vec3(0.0f);
	}

	sah.leftCost = CalculateBoxSurfaceAreaHalf(sah.left) * sah.leftCount;
	sah.rightCost = CalculateBoxSurfaceAreaHalf(sah.right) * sah.rightCount;
	sah.cost = sah.leftCost + sah.rightCost;
}

int BVHManager::Partition(RAABB rbounds, int first, int count)
{
	// Partition
	int i = first;
	int j = first + (count - 1);

	while(i <= j)
	{
		while(i <= j && IsPrimitiveWithinBounds(primitives + indices[i], rbounds))
		{
			i++;
		}

		while(i <= j && !IsPrimitiveWithinBounds(primitives + indices[j], rbounds))
		{
			j--;
		}

		if(i < j)
		{
			int temp = indices[i];
			indices[i] = indices[j];
			indices[j] = temp;
		}
	}

	return i;
}

void BVHManager::IntersectsPrimitives(Ray& ray, BVHNode* node) CONST
{
	for(int i = 0; i < node->count; i++)
	{
		Primitive* prim = primitives + indices[i + node->leftFirst];
		if(prim->type == Primitive::TRIANGLE)
		{
			float t = 99999999.0f;
			if(IntersectTriangle(*prim->pos[0], *prim->pos[1], *prim->pos[2], ray.O, ray.D, &t))
			{
				if((t < ray.t) && (t > 0))
				{
					ray.t = t;
					ray.hit = prim;
					ray.hitIndex = prim->index;
				}
			}
		}
		else
		{
			vec3 c = *prim->pos[0] - ray.O;
			float r2 = prim->radius * prim->radius;

			float t = dot(c, ray.D);
			vec3 q = c - t * ray.D;
			float p2 = dot(q, q);
			if(p2 <= r2)
			{
				float s = (r2 - dot(c, c) < 0 ? -1.f : 1.f);
				t = t + s * sqrtf(r2 - p2);
				if((t < ray.t) && (t > 0))
				{
					ray.t = t;
					ray.hit = prim;
				}
			}
		}
	}
}

void BVHManager::IntersectsPrimitives(Ray4& rays, BVHNode* node) CONST
{
	for(int i = 0; i < node->count; i++)
	{
		Primitive* prim = primitives + indices[i + node->leftFirst];
		if(prim->type == Primitive::TRIANGLE)
		{
			__m256 T4 = _mm256_mul_ps(_mm256_set1_ps(2.f), rays.t4);
			__m256 mask4 = IntersectTriangle4(*prim->pos[0], *prim->pos[1], *prim->pos[2], rays.Ox4, rays.Oy4, rays.Oz4, rays.Dx4, rays.Dy4, rays.Dz4, rays.t4, T4);

			// if((t < ray.t) && (t > 0))
			mask4 = _mm256_and_ps(mask4, _mm256_cmp_ps(T4, _mm256_setzero_ps(), _CMP_GT_OQ));

			int mask = _mm256_movemask_ps(mask4);
			if(mask)
			{
				// Hmm should we store this?
				__m256i hit = _mm256_set1_epi32(indices[i + node->leftFirst]);
				__m256i index = _mm256_set1_epi32(prim->index);

				// Cast ps32 to i32
				__m256i maski4 = _mm256_castps_si256(mask4);

				// Set
				rays.t4 = _mm256_blendv_ps(rays.t4, T4, mask4);
				rays.hit4 = _mm256_blendv_epi8(rays.hit4, hit, maski4);
				rays.hitindex4 = _mm256_blendv_epi8(rays.hitindex4, index, maski4);
			}
		}
		else
		{
			__m256 posX4 = _mm256_set1_ps(prim->pos[0]->x);
			__m256 posY4 = _mm256_set1_ps(prim->pos[0]->y);
			__m256 posZ4 = _mm256_set1_ps(prim->pos[0]->z);
			__m256 r42 = _mm256_set1_ps(prim->radius);
			r42 = _mm256_mul_ps(r42, r42);

			__m256 cX4 = _mm256_sub_ps(posX4, rays.Ox4);
			__m256 cY4 = _mm256_sub_ps(posY4, rays.Oy4);
			__m256 cZ4 = _mm256_sub_ps(posZ4, rays.Oz4);
			__m256 c42 = _mm256_add_ps(_mm256_mul_ps(cX4, cX4),
									   _mm256_add_ps(_mm256_mul_ps(cY4, cY4),
													 _mm256_mul_ps(cZ4, cZ4)));

			__m256 t4 = _mm256_add_ps(_mm256_mul_ps(cX4, rays.Dx4),
									  _mm256_add_ps(_mm256_mul_ps(cY4, rays.Dy4),
													_mm256_mul_ps(cZ4, rays.Dz4)));

			__m256 qX4 = _mm256_sub_ps(cX4, _mm256_mul_ps(t4, rays.Dx4));
			__m256 qY4 = _mm256_sub_ps(cY4, _mm256_mul_ps(t4, rays.Dy4));
			__m256 qZ4 = _mm256_sub_ps(cZ4, _mm256_mul_ps(t4, rays.Dz4));


			__m256 pX42 = _mm256_mul_ps(qX4, qX4);
			__m256 pY42 = _mm256_mul_ps(qY4, qY4);
			__m256 pZ42 = _mm256_mul_ps(qZ4, qZ4);

			__m256 p42 = _mm256_add_ps(pX42, _mm256_add_ps(pY42, pZ42));

			__m256 hitMask4 = _mm256_cmp_ps(p42, r42, _CMP_LE_OQ);

			__m256 sign = _mm256_add_ps(_mm256_and_ps(_mm256_cmp_ps(r42, c42, _CMP_LE_OQ), _mm256_set1_ps(-2.f)), _mm256_set1_ps(1.f));
			t4 = _mm256_and_ps(_mm256_add_ps(t4, _mm256_mul_ps(sign, _mm256_sqrt_ps(_mm256_sub_ps(r42, p42)))), hitMask4);
			hitMask4 = _mm256_and_ps(_mm256_cmp_ps(t4, rays.t4, _CMP_LE_OQ), _mm256_cmp_ps(t4, _mm256_setzero_ps(), _CMP_GT_OQ));


			__m256i hitMaski4 = _mm256_castps_si256(hitMask4);
			__m256i primIndex4 = _mm256_set1_epi32(indices[i + node->leftFirst]);
			rays.hit4 = _mm256_blendv_epi8(rays.hit4, primIndex4, hitMaski4);
			rays.t4 = _mm256_blendv_ps(rays.t4, t4, hitMask4);
		}
	}
}

void BVHManager::IntersectsPrimitivesEarlyOut(Ray& ray, BVHNode* node) CONST
{
	for(int i = 0; i < node->count; i++)
	{
		Primitive* prim = primitives + indices[i + node->leftFirst];
		if(prim->type == Primitive::TRIANGLE)
		{
			float t = 99999999.0f;
			if(IntersectTriangle(*prim->pos[0], *prim->pos[1], *prim->pos[2], ray.O, ray.D, &t))
			{
				if(t > 0 && t < ray.t && prim->mat->transparency < 0.001f)
				{
					//ray.t = t;
					ray.hit = prim;
					return;
				}
			}
		}
		else if(prim->mat->transparency < 0.001f)
		{
			vec3 c = *prim->pos[0] - ray.O;
			float r2 = prim->radius * prim->radius;

			float t = dot(c, ray.D);
			vec3 q = c - t * ray.D;
			float p2 = dot(q, q);
			if(p2 <= r2)
			{
				t -= sqrtf(r2 - p2);
				if((t < ray.t) && (t > 0) && r2 < dot(c, c))
				{
					ray.hit = prim;
					return;
				}
			}
		}
	}
}

void BVHManager::IntersectsPrimitivesEarlyOut(Ray4& rays, BVHNode* node, __m256& mask) CONST
{
	for(int i = 0; i < node->count; i++)
	{
		Primitive* prim = primitives + indices[i + node->leftFirst];
		if(prim->type == Primitive::TRIANGLE)
		{
			__m256 T4;
			__m256 mask4 = IntersectTriangle4(*prim->pos[0], *prim->pos[1], *prim->pos[2], rays.Ox4, rays.Oy4, rays.Oz4, rays.Dx4, rays.Dy4, rays.Dz4, rays.t4, T4);

			mask4 = _mm256_and_ps(mask4, _mm256_cmp_ps(T4, _mm256_setzero_ps(), _CMP_GT_OQ));

			mask = _mm256_or_ps(mask4, mask);
		}
		else
		{
			__m256 posX4 = _mm256_set1_ps(prim->pos[0]->x);
			__m256 posY4 = _mm256_set1_ps(prim->pos[0]->y);
			__m256 posZ4 = _mm256_set1_ps(prim->pos[0]->z);
			__m256 r42 = _mm256_set1_ps(prim->radius);
			r42 = _mm256_mul_ps(r42, r42);

			__m256 cX4 = _mm256_sub_ps(posX4, rays.Ox4);
			__m256 cY4 = _mm256_sub_ps(posY4, rays.Oy4);
			__m256 cZ4 = _mm256_sub_ps(posZ4, rays.Oz4);
			__m256 c42 = _mm256_add_ps(_mm256_mul_ps(cX4, cX4),
									   _mm256_add_ps(_mm256_mul_ps(cY4, cY4),
													 _mm256_mul_ps(cZ4, cZ4)));

			__m256 t4 = _mm256_add_ps(_mm256_mul_ps(cX4, rays.Dx4),
									  _mm256_add_ps(_mm256_mul_ps(cY4, rays.Dy4),
													_mm256_mul_ps(cZ4, rays.Dz4)));

			__m256 qX4 = _mm256_sub_ps(cX4, _mm256_mul_ps(t4, rays.Dx4));
			__m256 qY4 = _mm256_sub_ps(cY4, _mm256_mul_ps(t4, rays.Dy4));
			__m256 qZ4 = _mm256_sub_ps(cZ4, _mm256_mul_ps(t4, rays.Dz4));


			__m256 pX42 = _mm256_mul_ps(qX4, qX4);
			__m256 pY42 = _mm256_mul_ps(qY4, qY4);
			__m256 pZ42 = _mm256_mul_ps(qZ4, qZ4);

			__m256 p42 = _mm256_add_ps(pX42, _mm256_add_ps(pY42, pZ42));

			__m256 hitMask4 = _mm256_cmp_ps(p42, r42, _CMP_LE_OQ);

			__m256 sign = _mm256_add_ps(_mm256_and_ps(_mm256_cmp_ps(r42, c42, _CMP_LE_OQ), _mm256_set1_ps(-2.f)), _mm256_set1_ps(1.f));
			t4 = _mm256_and_ps(_mm256_add_ps(t4, _mm256_mul_ps(sign, _mm256_sqrt_ps(_mm256_sub_ps(r42, p42)))), hitMask4);
			hitMask4 = _mm256_and_ps(_mm256_cmp_ps(t4, rays.t4, _CMP_LE_OQ), _mm256_cmp_ps(t4, _mm256_setzero_ps(), _CMP_GT_OQ));

			mask = _mm256_or_ps(hitMask4, mask);
		}
	}
}
