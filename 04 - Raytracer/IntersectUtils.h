#pragma once

inline float Rand(float range) { return ((float)rand() / RAND_MAX) * range; }
inline int IRand(int range) { return rand() % range; }
int filesize(FILE* f);
#define MALLOC64(x) _aligned_malloc(x,64)
#define FREE64(x) _aligned_free(x)

namespace Tmpl8
{

#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define PI					3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f

#define PREFETCH(x)			_mm_prefetch((const char*)(x),_MM_HINT_T0)
#define PREFETCH_ONCE(x)	_mm_prefetch((const char*)(x),_MM_HINT_NTA)
#define PREFETCH_WRITE(x)	_m_prefetchw((const char*)(x))
#define loadss(mem)			_mm_load_ss((const float*const)(mem))
#define broadcastps(ps)		_mm_shuffle_ps((ps),(ps), 0)
#define broadcastss(ss)		broadcastps(loadss((ss)))

	struct timer
	{
		typedef long long value_type;
		static double inv_freq;
		value_type start;
		timer() : start(get()) { init(); }
		float elapsed() const { return (float)((get() - start) * inv_freq); }
		static value_type get()
		{
			LARGE_INTEGER c;
			QueryPerformanceCounter(&c);
			return c.QuadPart;
		}
		static double to_time(const value_type vt) { return double(vt) * inv_freq; }
		void reset() { start = get(); }
		static void init()
		{
			LARGE_INTEGER f;
			QueryPerformanceFrequency(&f);
			inv_freq = 1000. / double(f.QuadPart);
		}
	};

	typedef unsigned int uint;
	typedef unsigned char uchar;
	typedef unsigned char byte;

#define BADFLOAT(x) ((*(uint*)&x & 0x7f000000) == 0x7f000000)

	struct MinMax
	{
		float min;
		float max;
		MinMax() {}
		MinMax(float min, float max) { this->min = min; this->max = max; }
	};

	static vec3 raxis[3] = {vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)};

	inline bool CheckBox(vec3& bmin, vec3& bmax, vec3 O, vec3 rD)
	{
		float tx1 = (bmin.x - O.x) * rD.x;
		float tx2 = (bmax.x - O.x) * rD.x;
		float tmin = min(tx1, tx2);
		float tmax = max(tx1, tx2);
		float ty1 = (bmin.y - O.y) * rD.y;
		float ty2 = (bmax.y - O.y) * rD.y;
		tmin = max(tmin, min(ty1, ty2));
		tmax = min(tmax, max(ty1, ty2));
		float tz1 = (bmin.z - O.z) * rD.z;
		float tz2 = (bmax.z - O.z) * rD.z;
		tmin = max(tmin, min(tz1, tz2));
		tmax = min(tmax, max(tz1, tz2));

		return tmax >= tmin && tmax >= 0;
	}

	inline bool CheckBox4(__m128& bmin4, __m128& bmax4, __m128& O4, __m128& rD4)
	{
		__m128 t1 = _mm_mul_ps(_mm_sub_ps(bmin4, O4), rD4);
		__m128 t2 = _mm_mul_ps(_mm_sub_ps(bmax4, O4), rD4);
		__m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
		float* vmax = (float*)&vmax4, *vmin = (float*)&vmin4;
		float tmax = min(vmax[0], min(vmax[1], vmax[2]));
		float tmin = max(vmin[0], max(vmin[1], vmin[2]));
		return tmax >= tmin && tmax >= 0;
	}

	inline bool CheckBox44(Node4& node, Ray4& ray4)
	{
		__m256 txMin = _mm256_mul_ps(_mm256_sub_ps(node.minX, ray4.Ox4), ray4.rDx4);
		__m256 tyMin = _mm256_mul_ps(_mm256_sub_ps(node.minY, ray4.Oy4), ray4.rDy4);
		__m256 tzMin = _mm256_mul_ps(_mm256_sub_ps(node.minZ, ray4.Oz4), ray4.rDz4);
		__m256 txMax = _mm256_mul_ps(_mm256_sub_ps(node.maxX, ray4.Ox4), ray4.rDx4);
		__m256 tyMax = _mm256_mul_ps(_mm256_sub_ps(node.maxY, ray4.Oy4), ray4.rDy4);
		__m256 tzMax = _mm256_mul_ps(_mm256_sub_ps(node.maxZ, ray4.Oz4), ray4.rDz4);

		__m256 t1x = _mm256_min_ps(txMin, txMax);
		__m256 t1y = _mm256_min_ps(tyMin, tyMax);
		__m256 t1z = _mm256_min_ps(tzMin, tzMax);
		__m256 t2x = _mm256_max_ps(txMin, txMax);
		__m256 t2y = _mm256_max_ps(tyMin, tyMax);
		__m256 t2z = _mm256_max_ps(tzMin, tzMax);

		__m256 tNear = _mm256_max_ps(_mm256_max_ps(t1x, t1y), t1z);
		__m256 tFar  = _mm256_min_ps(_mm256_min_ps(t2x, t2y), t2z);

		__m256 maxGEmin = _mm256_cmp_ps(tFar, tNear, _CMP_GE_OQ);	// tFar >= tNear
		__m256 minGEzero = _mm256_cmp_ps(tFar, _mm256_setzero_ps(), _CMP_GT_OQ);	// tFar > 0
		__m256 minLTt = _mm256_cmp_ps(tNear, ray4.t4, _CMP_LT_OQ);		// tNear < t

		__m256 result = _mm256_and_ps(_mm256_and_ps(maxGEmin, minGEzero), minLTt);
		return _mm256_movemask_ps(result);
	}

	inline float CheckBoxMin4(__m128& bmin4, __m128& bmax4, __m128& O4, __m128& rD4)
	{
		__m128 t1 = _mm_mul_ps(_mm_sub_ps(bmin4, O4), rD4);
		__m128 t2 = _mm_mul_ps(_mm_sub_ps(bmax4, O4), rD4);
		__m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
		float* vmax = (float*)&vmax4, *vmin = (float*)&vmin4;
		float tmin = max(vmin[0], max(vmin[1], vmin[2]));
		return tmin;
	}

	inline MinMax CheckBoxMinMax4(__m128& bmin4, __m128& bmax4, __m128& O4, __m128& rD4)
	{
		__m128 t1 = _mm_mul_ps(_mm_sub_ps(bmin4, O4), rD4);
		__m128 t2 = _mm_mul_ps(_mm_sub_ps(bmax4, O4), rD4);
		__m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
		float* vmax = (float*)&vmax4, *vmin = (float*)&vmin4;
		float tmax = min(vmax[0], min(vmax[1], vmax[2]));
		float tmin = max(vmin[0], max(vmin[1], vmin[2]));
		return MinMax(tmin, tmax);
	}

	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
#define EPSILON 0.000001

	inline int IntersectTriangle(const vec3 V1, // Triangle vertices
								 const vec3 V2,
								 const vec3 V3,
								 const vec3 O, //Ray origin
								 const vec3 D, //Ray direction
								 float* out)
	{
		vec3 e1, e2;
		vec3 P, Q, T;
		float det, inv_det, u, v;
		float t;

		e1 = V2 - V1;
		e2 = V3 - V1;
		P = cross(D, e2);
		det = dot(e1, P);
		if(det > -EPSILON && det < EPSILON) return 0;
		inv_det = 1.f / det;
		T = O - V1;
		u = dot(T, P) * inv_det;
		if(u < 0.f || u > 1.f) return 0;
		Q = cross(T, e1);
		v = dot(D, Q) * inv_det;
		if(v < 0.f || u + v > 1.f) return 0;
		t = dot(e2, Q) * inv_det;
		if(t > EPSILON)
		{
			*out = t;
			return 1;
		}

		return 0;
	}

	inline __m256 CrossX4(const __m256& ax4, const __m256& ay4, const __m256& az4, const __m256& bx4, const __m256& by4, const __m256& bz4)
	{
		return _mm256_sub_ps(_mm256_mul_ps(ay4, bz4), _mm256_mul_ps(by4, az4));
	}
	inline __m256 CrossY4(const __m256& ax4, const __m256& ay4, const __m256& az4, const __m256& bx4, const __m256& by4, const __m256& bz4)
	{
		return _mm256_sub_ps(_mm256_mul_ps(az4, bx4), _mm256_mul_ps(bz4, ax4));
	}
	inline __m256 CrossZ4(const __m256& ax4, const __m256& ay4, const __m256& az4, const __m256& bx4, const __m256& by4, const __m256& bz4)
	{
		return _mm256_sub_ps(_mm256_mul_ps(ax4, by4), _mm256_mul_ps(bx4, ay4));
	}
	inline __m256 Dot4(const __m256& ax4, const __m256& ay4, const __m256& az4, const __m256& bx4, const __m256& by4, const __m256& bz4)
	{
		return _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(ax4, bx4), _mm256_mul_ps(ay4, by4)), _mm256_mul_ps(az4, bz4));
	}

	inline __m256 IntersectTriangle4(const vec3& v1, // Triangle vertices
		const vec3& v2,
		const vec3& v3,
		const __m256& ox4, const __m256& oy4, const __m256& oz4,  //Ray origin
		const __m256& dx4, const __m256& dy4, const __m256& dz4, //Ray direction
		__m256& tin4, __m256& tout4)
	{
		static const __m256 ZERO4 = _mm256_setzero_ps();
		static const __m256 ONEDOTZERO4 = _mm256_set1_ps(1.0f);
		static const __m256 MINUSEPSILON4 = _mm256_set1_ps(-EPSILON);
		static const __m256 EPSILON4 = _mm256_set1_ps(EPSILON);

		// move to Primitive?
		__m256 v1x4 = _mm256_set1_ps(v1.x); __m256 v1y4 = _mm256_set1_ps(v1.y); __m256 v1z4 = _mm256_set1_ps(v1.z);
		__m256 v2x4 = _mm256_set1_ps(v2.x); __m256 v2y4 = _mm256_set1_ps(v2.y); __m256 v2z4 = _mm256_set1_ps(v2.z);
		__m256 v3x4 = _mm256_set1_ps(v3.x); __m256 v3y4 = _mm256_set1_ps(v3.y); __m256 v3z4 = _mm256_set1_ps(v3.z);

		// e1 = V2 - V1;
		// e2 = V3 - V1;
		__m256 e1x4 = _mm256_sub_ps(v2x4, v1x4); __m256 e1y4 = _mm256_sub_ps(v2y4, v1y4); __m256 e1z4 = _mm256_sub_ps(v2z4, v1z4);
		__m256 e2x4 = _mm256_sub_ps(v3x4, v1x4); __m256 e2y4 = _mm256_sub_ps(v3y4, v1y4); __m256 e2z4 = _mm256_sub_ps(v3z4, v1z4);
		
		// P = cross(D, e2);
		__m256 px4 = CrossX4(dx4, dy4, dz4, e2x4, e2y4, e2z4); __m256 py4 = CrossY4(dx4, dy4, dz4, e2x4, e2y4, e2z4); __m256 pz4 = CrossZ4(dx4, dy4, dz4, e2x4, e2y4, e2z4);

		// det = dot(e1, P);
		__m256 det4 = Dot4(e1x4, e1y4, e1z4, px4, py4, pz4);

		// if(det > -EPSILON && det < EPSILON) return 0;
		__m256 mask1 = _mm256_cmp_ps(det4, MINUSEPSILON4, _CMP_LE_OQ); //_mm256_cmple_ps(det4, MINUSEPSILON4);
		__m256 mask2 = _mm256_cmp_ps(det4, EPSILON4, _CMP_GE_OQ); //_mm256_cmpge_ps(det4, EPSILON4);
		__m256 det4mask = _mm256_or_ps(mask1, mask2);
		if (_mm256_movemask_ps(det4mask) == 0) return ZERO4;

		//  inv_det = 1.f / det;
		__m256 inv_det4 = _mm256_div_ps(ONEDOTZERO4, det4);

		// T = O - V1;
		__m256 tx4 = _mm256_sub_ps(ox4, v1x4); __m256 ty4 = _mm256_sub_ps(oy4, v1y4); __m256 tz4 = _mm256_sub_ps(oz4, v1z4);

		// u = dot(T, P) * inv_det;
		__m256 u4 = _mm256_mul_ps(Dot4(tx4, ty4, tz4, px4, py4, pz4), inv_det4);

		// if(u < 0.f || u > 1.f) return 0;
		mask1 = _mm256_cmp_ps(u4, ZERO4, _CMP_GE_OQ); //_mm256_cmpge_ps(u4, ZERO4);
		mask2 = _mm256_cmp_ps(u4, ONEDOTZERO4, _CMP_LE_OQ); //_mm256_cmple_ps(u4, ONEDOTZERO4);
		__m256 umask = _mm256_and_ps(mask1, mask2);
		__m256 combinedmask = _mm256_and_ps(det4mask, umask);
		if (_mm256_movemask_ps(combinedmask) == 0) return ZERO4;

		// Q = cross(T, e1);
		__m256 qx4 = CrossX4(tx4, ty4, tz4, e1x4, e1y4, e1z4); __m256 qy4 = CrossY4(tx4, ty4, tz4, e1x4, e1y4, e1z4); __m256 qz4 = CrossZ4(tx4, ty4, tz4, e1x4, e1y4, e1z4);

		// v = dot(D, Q) * inv_det;
		__m256 v4 = _mm256_mul_ps(Dot4(dx4, dy4, dz4, qx4, qy4, qz4), inv_det4);

		// if(v < 0.f || u + v > 1.f) return 0;
		mask1 = _mm256_cmp_ps(v4, ZERO4, _CMP_GE_OQ); //_mm256_cmpge_ps(v4, ZERO4);
		mask2 = _mm256_cmp_ps(_mm256_add_ps(u4, v4), ONEDOTZERO4, _CMP_LE_OQ); //_mm256_cmple_ps(_mm256_add_ps(u4, v4), ONEDOTZERO4);
		__m256 vmask = _mm256_and_ps(mask1, mask2);
		__m256 finalmask = _mm256_and_ps(combinedmask, vmask);

		// t = dot(e2, Q) * inv_det;
		tout4 = _mm256_mul_ps(Dot4(e2x4, e2y4, e2z4, qx4, qy4, qz4), inv_det4);

		finalmask = _mm256_and_ps(finalmask, _mm256_cmp_ps(tout4, tin4, _CMP_LT_OQ)); //_mm256_cmplt_ps(tout4, tin4));

		return finalmask;
	}

	// http://answers.unity3d.com/questions/383804/calculate-uv-coordinates-of-3d-point-on-plane-of-m.html
	inline vec3 FindInterpolation(vec3 p1, vec3 p2, vec3 p3, vec3 f)
	{
		// calculate vectors from point f to vertices p1, p2 and p3:
		vec3 f1 = p1 - f;
		vec3 f2 = p2 - f;
		vec3 f3 = p3 - f;

		// calculate the areas and factors (order of parameters doesn't matter):
		float a = 1.f / length(cross(p1 - p2, p1 - p3)); // main triangle area a
		float a1 = length(cross(f2, f3)) * a;
		float a2 = length(cross(f3, f1)) * a;
		float a3 = 1.f - a2 - a1;

		// find the uv corresponding to point f (uv1/uv2/uv3 are associated to p1/p2/p3):
		return vec3(a1, a2, a3);
	}

	// https://en.wikipedia.org/wiki/UV_mapping
	inline vec2 GetUVPoints(vec3 dir, float offset)
	{
		float u = 0.5f + (atan2f(dir.z, dir.x) + offset) / (2.0f * PI);
		float v = 0.5f - (asinf(dir.y)) / PI;
		return vec2(u, v);
	}
}