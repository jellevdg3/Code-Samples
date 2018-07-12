#pragma once

#include "Net/Protocol/Packet.h"

class PPlayOut35BlockChange : public Packet
{
public:
	// Functions
	virtual int Length() override;
	virtual void Write(ConnectionHandler* connection, class ConnectionOutput* out) override;

	// Variables
	unsigned long long position;
	varint blockid;
};
