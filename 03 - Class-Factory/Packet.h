#pragma once

#include "Net/Connection/ConnectionHandler.h"

// Default structures
typedef int varint;

typedef union
{
	struct
	{
		int x : 26;
		int y : 12;
		int z : 26;
	};
	long long pos;
} Position;

// Packet
class Packet
{
public:
	const static int BYTE_SIZE = 1;
	const static int SHORT_SIZE = 2;
	const static int INT_SIZE = 4;
	const static int FLOAT_SIZE = 4;
	const static int LONG_SIZE = 8;
	const static int DOUBLE_SIZE = 8;

	// Constructor
	Packet();
	virtual ~Packet();

	// IO functions
	int	GetVarIntLength(int d);
	int GetStringLength(std::string d);
	
	// Functions
	static void Register(int status, int mode, int id, Packet* (*function)(), char* name);
	static Packet* Get(int status, int mode, int id);

	virtual void WriteHeader(ConnectionHandler* connection, class ConnectionOutput* out);
	virtual void WriteContent(ConnectionHandler* connection, class ConnectionOutput* out);

	virtual int Length() { return 0; }
	virtual int CompressedLength() { return 0; }

	virtual void Read(ConnectionHandler* connection, class ConnectionInput* in) {}
	virtual void Write(ConnectionHandler* connection, class ConnectionOutput* out) {}

	virtual void Apply(ConnectionHandler* connection) {}

	// Variables
	int id;
	bool shouldApply;
};

// Marco to register a class. usage: RegisterPacket(ConnectionStatus, ConnectionMode, PacketID, Packet Class).
#define RegisterPacket(_status, _mode, _id, _p) namespace _p ## _N \
{ \
	Packet* _p ## Get() \
	{ \
		Packet* p = (Packet*)(new _p()); \
		p->id = _id; \
		return p; \
	}; \
	\
	struct StaticCode \
	{ \
		StaticCode() \
		{ \
			Packet::Register(_status, _mode, _id, &_p ## Get, #_p); \
		} \
	}; \
	static StaticCode executeStaticCode; \
};
