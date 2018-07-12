#pragma once

#include "Ray.h"
#define PACKET_SIZE 8

namespace Tmpl8
{
    // Normal ray packet
    struct RayPacket
    {
		/*		MORTON ORDER is used
		*
		*		0	1		4	5
		*		2	3		6	7
		*
		*		8	9		12	13
		*		10	11		14	15
		*
		*/
        Ray rays[PACKET_SIZE * PACKET_SIZE];
    };

    // SIMD ray packet
	struct RayPacket4
	{
		Ray4 rays[8];
	};
}
