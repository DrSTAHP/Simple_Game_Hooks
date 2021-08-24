#include "hooks.h"
#include "process_window.h"

//////////////////////EndScene///////////////////////////////
static BYTE EndSceneStolenBytes[7];
static callback_endscene_t oEndScene = nullptr;
static callback_endscene_t pEndSceneCallbackHolder = nullptr;

static void APIENTRY EndScene_Hook(IDirect3DDevice9* pDevice)
{
	pEndSceneCallbackHolder(pDevice);
	oEndScene(pDevice);
}
////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////
static bool GetDevice(void** pVTableBuffer, size_t size)
{
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
	{
		pD3D->Release();
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetProcessWindow();
	d3dpp.Windowed = true;

	IDirect3DDevice9* pDevice = nullptr;
	pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);

	if (!pDevice)
	{
		pD3D->Release();
		return false;
	}

	memcpy(pVTableBuffer, *(void***)(pDevice), size);

	pDevice->Release();
	pD3D->Release();

	return true;
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
bool Hooks::HookEndScene(callback_endscene_t pCallback)
{
	void* pDeviceVTable[119];
	if (!GetDevice(pDeviceVTable, sizeof(pDeviceVTable)))
		return false;

	void* pEndScene = pDeviceVTable[42];

	//Setup trump'o-line

	int iLen = sizeof(EndSceneStolenBytes);
	if (iLen < 5)
		return false;

	//Allocate a block for stolen bytes + jmp to original function

	BYTE* pGateway = (BYTE*)VirtualAlloc(0, iLen, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	
	//Save stolen bytes in the unhook array and gateway

	memcpy(pGateway, pEndScene, iLen);
	memcpy(EndSceneStolenBytes, pEndScene, iLen);

	//Calculate the jump-back distance to the original function

	DWORD OrgToGateway = (DWORD)pEndScene - (DWORD)pGateway - 5; // -5 for jmp

	//Write the jump

	*(pGateway + iLen) = 0xE9; //jmp 
	*(DWORD*)((DWORD)pGateway + iLen + 1) = OrgToGateway;


	//Create a hook

	//Setup protection rights
	
	DWORD Protection;
	VirtualProtect(pEndScene, iLen, PAGE_EXECUTE_READWRITE, &Protection);

	//Calculate the jump from the original function to the hook

	callback_endscene_t pEndSceneHookHolder = EndScene_Hook;
	DWORD OrgToHook = ((DWORD)pEndSceneHookHolder - (DWORD)pEndScene) - 5; // -5 for jmp

	*(BYTE*)pEndScene = 0xE9; //jmp
	*(DWORD*)((DWORD)pEndScene + 1) = OrgToHook;

	//Revert the permissions

	DWORD temp;
	VirtualProtect(pEndScene, iLen, Protection, &temp);

	//Setup user callbacks

	pEndSceneCallbackHolder = pCallback;
	oEndScene = (callback_endscene_t)pGateway;
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
void Hooks::UnHookEndScene()
{
	void* pDeviceVTable[119];
	if (!GetDevice(pDeviceVTable, sizeof(pDeviceVTable)))
		return;

	void* pEndScene = pDeviceVTable[42];
	
	DWORD Protection;
	VirtualProtect(pEndScene, sizeof(EndSceneStolenBytes), PAGE_EXECUTE_READWRITE, &Protection);

	memcpy(pEndScene, EndSceneStolenBytes, sizeof(EndSceneStolenBytes));
	
	memcpy(EndSceneStolenBytes, 0, sizeof(EndSceneStolenBytes));
	oEndScene = nullptr;
	pEndSceneCallbackHolder = nullptr;
}
///////////////////////////////////////////////////////////