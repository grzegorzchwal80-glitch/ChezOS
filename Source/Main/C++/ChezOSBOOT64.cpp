/*
File is open source and under GNU GPL v3.0 license. See LICENSE for details.
*/

#include "Include/General.hpp"
#include "Include/KernelLoad.hpp"
#include "Include/Style.hpp"
#include "Include/UI.hpp"
#include "Include/Window.hpp"
#include <Windows.h>
UI::UIID width = win32::Window::widthSM;
UI::UIID height = win32::Window::HeightSM;
int main() {
  MakeWindow MakeWindow;
  MakeWindow.MakeEPIKwindow();

  if (!win32::Window::ToggleFullscreen()) {
    if (MessageBoxA(NULL, "Error!",
                    "Caught error, where fullscreen failed. Try again?",
                    MB_ICONERROR | MB_YESNO) == IDYES)
      win32::Window::ToggleFullscreen();
  }

  bool running = true;
  BOOL Wait = true;

  while (running) {
    win32::Window::PollEvents();

    if (!IsWindow(win32::Window::m_hwnd))
      running = false;

    UI::Window::BeginFrame();
    UI::Window::CreateImage(
        width / 2.5, height / 2.5, 300, 250,
        L"C:\\Users\\PC\\Documents\\myProject\\Files\\ChezOS\\ChezOS.logo.bmp");
    UI::UIID BOOTTEXT =
        UI::Window::DrawTextW(width / 2.2, height / 1.5, L"ChezOS", 0x00FFFFFF);
    UI::Style::SetTextSize(BOOTTEXT, 32);
    UI::Window::EndFrame(win32::Window::m_hwnd);

    if (Wait) {
      Sleep(3000);
      Wait = false;
      UI::Window::KillText(BOOTTEXT);
      KernelObjects::Load();
      break;
    }

    Sleep(1);
  }

  return 0;
}