#pragma once

#include "Net/Protocol/Packet.h"

class PPlayOut08PlayerPositionAndLook : public Packet
{
public:
	// Functions
	virtual int Length() override;
	virtual void Write(ConnectionHandler* connection, class ConnectionOutput* out) override;

	// Variables
	double x;
	double y;
	double z;
	float yaw;
	float pitch;
	char flags;
};
