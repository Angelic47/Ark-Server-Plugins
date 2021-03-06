#include <windows.h>
#include <API/ARK/Ark.h>

#pragma comment(lib, "ArkApi.lib")

DECLARE_HOOK(RCONClientConnection_SendMessageW, void, RCONClientConnection*, int, int, FString*);

void Hook_RCONClientConnection_SendMessageW(RCONClientConnection* _this, int id, int type, FString* out_going_message)
{
	std::string msg = ArkApi::Tools::Utf8Encode(**out_going_message);

	const size_t len = msg.length();

	DWORD64 packet = reinterpret_cast<DWORD64>(FMemory::Malloc(len + 14));

	*(int *)packet = len + 10;
	*(int *)(packet + 4) = id;
	*(int *)(packet + 8) = type;

	memcpy((void *)(packet + 12), msg.data(), len);

	*(char *)(len + packet + 12) = 0;
	*(char *)(len + packet + 13) = 0;

	auto socket = static_cast<FSocketBSD*>(_this->SocketField()());

	int bytes_sent = 0;
	if (!socket->Send((char*)packet, len + 14, &bytes_sent))
		_this->IsClosedField() = true;

	FMemory::Free(reinterpret_cast<void*>(packet));
}

void Load()
{
	ArkApi::GetHooks().SetHook("RCONClientConnection.SendMessageW", &Hook_RCONClientConnection_SendMessageW,
	                           &RCONClientConnection_SendMessageW_original);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
