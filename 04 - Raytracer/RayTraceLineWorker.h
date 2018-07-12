#include "Raytracer.h"

#include "BVH.h"
#include "Primitive.h"

// RayTracer Line Worker; Iterate an entire line of pixels on the screen
static void RayTraceLineWorker(void* rayTracer)
{
	Raytracer* rt = (Tmpl8::Raytracer*)rayTracer;
	bool done = false;

	RayPacket packet;
	RayPacket4 packet4;

	RayCounter counter;
	counter = 0;

	while (rt->running)
	{
		if (done && (rt->rayLineTasks == -1 || rt->rayLineTasks >= rt->rayTotalTasks))
		{
			Sleep(1);
			continue;
		}

		// Start lock to obtain next task
		rt->threads->taskObtainLock.lock();
		if (rt->rayLineTasks == -1 || rt->rayLineTasks >= rt->rayTotalTasks)
		{
			if (!done)
			{
				rt->completedThreads++;
				done = true;
			}
			rt->threads->taskObtainLock.unlock();
			continue;
		}
		if (done)
		{
			done = false;
			rt->completedThreads--;
			rt->currentCount += counter;
			counter = 0;
		}

		// End lock
		int y = rt->rayLineTasks;
		rt->rayLineTasks += 1;
		rt->threads->taskObtainLock.unlock();

		// Do we use SIMD?
		if (rt->useSimd)
		{
			rt->TracePacket(counter, y, packet4);
		}
		else
		{
			rt->TracePacket(counter, y, packet);
		}
	}
}