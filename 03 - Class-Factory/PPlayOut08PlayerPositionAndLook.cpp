#include "PPlayOut08PlayerPositionAndLook.h"

RegisterPacket(ConnectionStatus::PLAY, ConnectionMode::OUT, 8, PPlayOut08PlayerPositionAndLook);

int PPlayOut08PlayerPositionAndLook::Length()
{
	return 0
		+ DOUBLE_SIZE
		+ DOUBLE_SIZE
		+ DOUBLE_SIZE
		+ FLOAT_SIZE
		+ FLOAT_SIZE
		+ BYTE_SIZE
		;
}

void PPlayOut08PlayerPositionAndLook::Write(class ConnectionHandler* connection, class ConnectionOutput* out)
{
	out->WriteDouble(x);
	out->WriteDouble(y);
	out->WriteDouble(z);
	out->WriteFloat(yaw);
	out->WriteFloat(pitch);
	out->WriteChar(flags);
}
