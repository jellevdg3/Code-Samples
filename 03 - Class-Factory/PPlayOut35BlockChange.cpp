#include "PPlayOut35BlockChange.h"

RegisterPacket(ConnectionStatus::PLAY, ConnectionMode::OUT, 35, PPlayOut35BlockChange);

int PPlayOut35BlockChange::Length()
{
	return 0
		+ LONG_SIZE
		+ GetVarIntLength(blockid)
		;
}

void PPlayOut35BlockChange::Write(class ConnectionHandler* connection, class ConnectionOutput* out)
{
	out->WriteLong(position);
	out->WriteVarInt(blockid);
}
