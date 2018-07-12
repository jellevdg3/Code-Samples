#include "Packet.h"

#include <vector>
#include <string>
#include <iostream>

// Register
struct PacketElement
{
	Packet* (*function)();
	std::string name;
};

struct PacketRegister
{
	std::vector<PacketElement*> allPackets;
	PacketElement* reg[ConnectionStatus::COUNT][ConnectionMode::COUNT][256]; // status, mode, id

	PacketRegister()
	{
		// Set the packetregister to 0
		std::memset(reg, 0, sizeof(PacketElement*) * ConnectionStatus::COUNT * ConnectionMode::COUNT * 256);
	}
};

PacketRegister* packetRegister;

// Constructor
Packet::Packet()
{
	shouldApply = false;
	id = -1;
}

Packet::~Packet()
{
}

// IO functions
int Packet::GetVarIntLength(int d)
{
	int count = 0;
	do
	{
		((d & 0x7f) | ((d > 0x7f) ? 0x80 : 0x00));
		d = d >> 7;
		count++;
	} while (d > 0);
	return count;
}

int Packet::GetStringLength(std::string d)
{
	return GetVarIntLength(d.length()) + d.length();
}

// Functions
void Packet::Register(int status, int mode, int id, Packet* (*function)(), char* name)
{
	if (packetRegister == 0)
		packetRegister = new PacketRegister();

	if (packetRegister->reg[status][mode][id] != 0)
	{
		std::cout << "Packet " << name << " could not be registered. Slot already in use: [" << status << "," << mode << "," << id << "]." << std::endl;
		return;
	}

	PacketElement* e = new PacketElement();
	e->name = name;
	e->function = function;

	packetRegister->allPackets.push_back(e);
	packetRegister->reg[status][mode][id] = e;

	std::cout << "Registered Packet " << name << " to slot: [" << status << "," << mode << "," << id << "]." << std::endl;
}

Packet* Packet::Get(int status, int mode, int id)
{
	if (packetRegister->reg[status][mode][id])
	{
		return packetRegister->reg[status][mode][id]->function();
	}
	return 0;
}

void Packet::WriteHeader(ConnectionHandler* connection, class ConnectionOutput* out)
{
	int length = Length() + GetVarIntLength(id);
	if (out->compressionMode)
	{
		out->WriteVarInt(length + 1);
		out->WriteUChar(0);
	}
	else
	{
		out->WriteVarInt(length);
	}

	out->WriteVarInt(id);
}

void Packet::WriteContent(ConnectionHandler* connection, class ConnectionOutput* out)
{
	Write(connection, out);
}
