#pragma once
#include <Windows.h>

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

//////////////////////EndScene///////////////////////////////
typedef void(APIENTRY* callback_endscene_t)(IDirect3DDevice9*);
////////////////////////////////////////////////////////////

namespace Hooks {
	bool HookEndScene(callback_endscene_t);
	void UnHookEndScene();
}