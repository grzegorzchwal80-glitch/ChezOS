/*
File is open source and under GNU GPL v3.0 license. See LICENSE for details.
*/

#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <dwmapi.h>
#include <hidusage.h>
#include <shellapi.h>
#include <windows.h>
#include <windowsx.h>

#include <functional>
#include <stdexcept>
#include <string>

#include "UI.hpp"
#ifdef _MSC_VER
#pragma comment(lib, "dwmapi.lib")
#endif

namespace win32 {
class Window {
public:
  using EventCallback = std::function<void()>;
  using ResizeCallback = std::function<void(int, int)>;
  using KeyCallback = std::function<void(UINT, bool)>;
  using MouseMoveCallback = std::function<void(int, int)>;

  inline static HINSTANCE m_instance = nullptr;
  inline static std::wstring m_className = L"MyClass";
  inline static std::wstring m_title = L"MyWindow";
  inline static HWND m_hwnd = nullptr;
  inline static bool m_fullscreen = false;

  inline static int widthSM = GetSystemMetrics(SM_CXSCREEN);
  inline static int HeightSM = GetSystemMetrics(SM_CYSCREEN);

  inline static ResizeCallback OnResize;
  inline static EventCallback OnClose;
  inline static KeyCallback OnKey;
  inline static MouseMoveCallback OnMouseMove;

  static void EnableDPIAwareness() { SetProcessDPIAware(); }
  static void registerClass() {
    if (!m_instance)
      m_instance = GetModuleHandle(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = SetupProc;
    wc.hInstance = m_instance;
    wc.lpszClassName = m_className.c_str();
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    if (!RegisterClassExW(&wc))
      throw std::runtime_error("Class registration failed");
  }

  static void create(int width = 1280, int height = 720) {
    registerClass();

    m_hwnd =
        CreateWindowExW(0, m_className.c_str(), m_title.c_str(),
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        width, height, nullptr, nullptr, m_instance, nullptr);

    if (!m_hwnd)
      throw std::runtime_error("Window creation failed");

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
  }

  static void Show(int cmd = SW_SHOW) {
    ShowWindow(m_hwnd, cmd);
    UpdateWindow(m_hwnd);
  }

  static void Hide() { ShowWindow(m_hwnd, SW_HIDE); }

  static void SetTitle(const std::wstring &title) {
    m_title = title;
    SetWindowTextW(m_hwnd, m_title.c_str());
  }

  static void Resize(int width, int height) {
    SetWindowPos(m_hwnd, nullptr, 0, 0, width, height,
                 SWP_NOMOVE | SWP_NOZORDER);
  }

  static void Center() {
    RECT r{};
    GetWindowRect(m_hwnd, &r);
    int w = r.right - r.left;
    int h = r.bottom - r.top;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    SetWindowPos(m_hwnd, nullptr, (sw - w) / 2, (sh - h) / 2, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER);
  }

  static BOOL ToggleFullscreen() {
    static WINDOWPLACEMENT prev{};
    static LONG prevStyle{};

    if (!m_fullscreen) {
      prevStyle = GetWindowLong(m_hwnd, GWL_STYLE);

      MONITORINFO mi{};
      mi.cbSize = sizeof(MONITORINFO);

      if (GetWindowPlacement(m_hwnd, &prev) &&
          GetMonitorInfo(MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTOPRIMARY),
                         &mi)) {
        SetWindowLong(m_hwnd, GWL_STYLE, prevStyle & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(m_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
      }
      m_fullscreen = true;
    } else {
      SetWindowLong(m_hwnd, GWL_STYLE, prevStyle);
      SetWindowPlacement(m_hwnd, &prev);
      SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
      m_fullscreen = false;
    }

    return TRUE;
  }

  static void EnableRawMouse() {
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid.usUsage = HID_USAGE_GENERIC_MOUSE;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = m_hwnd;

    RegisterRawInputDevices(&rid, 1, sizeof(rid));
  }

  static void PollEvents() {
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  static int Run() {
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    return (int)msg.wParam;
  }

  static LRESULT CALLBACK SetupProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
      RECT r{};
      GetClientRect(hwnd, &r);
      UI::Window::Init(hwnd, r.right - r.left, r.bottom - r.top);
      return 0;
    }

    case WM_SIZE: {
      int w = LOWORD(lp);
      int h = HIWORD(lp);
      UI::Window::Resize(hwnd, w, h);

      if (OnResize)
        OnResize(w, h);

      return 0;
    }
    case WM_PAINT: {
      UI::Window::HandlePaint(hwnd);
      return 0;
    }
    case WM_DESTROY:
      UI::Window::Shutdown();

      if (OnClose)
        OnClose();

      PostQuitMessage(0);
      return 0;

    case WM_KEYDOWN:
    case WM_KEYUP:
      if (OnKey)
        OnKey((UINT)wp, msg == WM_KEYDOWN);
      return 0;
    case WM_CHAR:
      UI::Window::HandleChar(wp);
      return 0;
    case WM_MOUSEMOVE: {
      UI::Window::HandleMouse(msg, lp);

      if (OnMouseMove)
        OnMouseMove(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));

      return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP: {
      UI::Window::HandleMouse(msg, lp);
      return 0;
    }
    case WM_ERASEBKGND:
      return 1;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
  }
};

} // namespace win32
