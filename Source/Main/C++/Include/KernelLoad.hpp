#pragma once

#include "../../../../Resources/headers/json.hpp"
#include "Window.hpp"
#include <atomic>
#include <fstream>
#include <random>
#include <string>
#include <windows.h>


using json = nlohmann::json;

extern UI::UIID width;
extern UI::UIID height;

class MakeWindow {
public:
  void MakeEPIKwindow() {
    win32::Window::create(width, height);
    win32::Window::SetTitle(L"ChezOS");
    win32::Window::Center();
  }
};

class KernelObjects {
public:
  static int Kernel();
  static void APICALL();
  // I use these variables, and for every variable i make in ChezOSKERNEL64.cpp,
  // i put it here
  inline static json j{};
  inline static RECT rc{};
  inline static bool showPopupBrowser = false;
  inline static UI::UIID ButtonCloseWindowBrowser = 0;
  inline static UI::UIID input = 0;
  inline static UI::UIID sendBtn = 0;
  inline static std::string aiReply = "";
  inline static std::atomic<bool> requestInProgress{false};

  inline static int popupW = 500;
  inline static int popupH = 600;

  inline static HMODULE hDLL = nullptr;
  inline static char *g_apiKey_ptr = nullptr;

  inline static int (*GetDuckAnswer)(const char *, char *, int) = nullptr;

  inline static int windowWidth = 0;
  inline static int windowHeight = 0;

  inline static UI::UIID ChezOSAI = 0;

  inline static BOOL running = TRUE;

  inline static bool showPopupAPI = false;
  inline static bool showPopupAI = false;

  inline static UI::UIID apiInput = 0;
  inline static UI::UIID apiSendBtn = 0;
  inline static UI::UIID apiCloseBtn = 0;

  inline static UI::UIID aiInput = 0;
  inline static UI::UIID aiSendBtn = 0;
  inline static UI::UIID aiCloseBtn = 0;

  inline static int w = 0;
  inline static int h = 0;
  inline static int px = 0;
  inline static int py = 0;

  inline static std::wstring wApiKey = L"";
  inline static std::string apiKeyStr = "";

  inline static unsigned char salt[16]{};
  inline static std::random_device rd{};
  inline static char encoded[256]{};

  inline static int result = 0;

  inline static std::ofstream file{};

  inline static std::wstring text = L"";
  inline static std::string queryStr = "";

  inline static std::wstring wReply = L"";

  // I SEE THE LIGHT (if you use clangd you can relate to the inline)
  static int Load() { return Kernel(); }
};