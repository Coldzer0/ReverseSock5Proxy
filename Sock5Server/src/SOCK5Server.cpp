/**
 * Reverse TCP Sock5 Proxy
 * Copyright(c) 2022 - Coldzer0 <Coldzer0 [at] protonmail.ch> @Coldzer0x0
 */

#ifdef _DIST
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "UI/GUI_backend.h"
#include <Windows.h>

extern "C" {
[[maybe_unused]] __declspec(dllexport) uint32_t NvOptimusEnablement = 1;
[[maybe_unused]] __declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  InitServerGui();
  return 0;
}
