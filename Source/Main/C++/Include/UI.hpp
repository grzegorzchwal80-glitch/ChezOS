/*
File is open source and under GNU GPL v3.0 license. See LICENSE for details.
*/

#pragma once
#define MAX_API_KEY_LENGTH 256
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <string>
#include <vector>
#include <windows.h>
#include <windowsx.h>

namespace UI {
using UIID = unsigned long long;

struct Color {
  COLORREF value;
};

struct Base {
  UIID id;
};
struct Rect : Base {
  int x, y, w, h;
  Color color;
};
struct Circle : Base {
  int x, y, r;
  Color color;
};
struct Line : Base {
  int x1, y1, x2, y2, width;
  Color color;
};
struct Text : Base {
  int x, y;
  std::wstring str;
  Color color;
  int fontSize = 16;
};
struct Button : Base {
    int x, y, w, h;
    std::wstring text;
    Color bg, hover, active;

    // --- Interaction states ---
    bool isHovered = false;
    bool isPressed = false;
    bool wasPressed = false;

    // --- Dragging support ---
    bool draggable = false;     // <-- make this visible in Intellisense
    bool isDragging = false;
    int dragOffsetX = 0;
    int dragOffsetY = 0;
};
struct ProgressBar : Base {
  int x, y, w, h;
  float percent;
  Color fg, bg;
};
struct TextInput : Base {
  int x, y, w, h;
  std::wstring text;
  Color bg;
  Color border;
  Color activeBorder;
  bool isFocused = false;
  size_t cursorPos = 0;
};

class Window {
public:
  inline static std::vector<Rect> rects;
  inline static std::vector<Circle> circles;
  inline static std::vector<Line> lines;
  inline static std::vector<Text> texts;
  inline static std::vector<Button> buttons;
  inline static std::vector<ProgressBar> bars;
  inline static std::vector<TextInput> textInputs;
  inline static UIID focusedInput = 0;

  inline static int imageX = 0;
  inline static int imageY = 0;
  inline static int imageW = 0;
  inline static int imageH = 0;

  inline static HDC backDC = nullptr;
  inline static HBITMAP backBitmap = nullptr;
  inline static HBITMAP oldBitmap = nullptr;
  inline static int width = 0;
  inline static int height = 0;

  inline static int mouseX = 0;
  inline static int mouseY = 0;
  inline static bool mouseDown = false;
  inline static bool mousePressed = false;
  inline static bool mouseReleased = false;

  inline static UIID idCounter = 1;

  static void BeginFrame() {
    rects.clear();
    circles.clear();
    lines.clear();
    texts.clear();
    bars.clear();
  }
  static void EndFrame(HWND hwnd) {
    InvalidateRect(hwnd, nullptr, FALSE);
    UpdateWindow(hwnd);
    mousePressed = false;
    mouseReleased = false;
  }

  inline static HBITMAP gdiImage = nullptr;

  static void CreateImage(int x, int y, int w, int h,
                          const std::wstring &path) {
    imageX = x;
    imageY = y;
    imageW = w;
    imageH = h;

    if (gdiImage) {
      DeleteObject(gdiImage);
      gdiImage = nullptr;
    }

    gdiImage = (HBITMAP)LoadImageW(nullptr, path.c_str(), IMAGE_BITMAP, w, h,
                                   LR_LOADFROMFILE);
  }
  static void DrawImages(HDC hdc) {
    if (!gdiImage)
      return;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, gdiImage);

    BITMAP bm;
    GetObject(gdiImage, sizeof(bm), &bm);

    BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteDC(memDC);
  }
  static void Init(HWND hwnd, int w, int h) {
    width = w;
    height = h;
    HDC hdc = GetDC(hwnd);
    backDC = CreateCompatibleDC(hdc);
    backBitmap = CreateCompatibleBitmap(hdc, w, h);
    oldBitmap = (HBITMAP)SelectObject(backDC, backBitmap);
    ReleaseDC(hwnd, hdc);
  }

  static void Shutdown() {
    if (backDC) {
      SelectObject(backDC, oldBitmap);
      DeleteObject(backBitmap);
      DeleteDC(backDC);
      backDC = nullptr;
    }
    rects.clear();
    circles.clear();
    lines.clear();
    texts.clear();
    buttons.clear();
    bars.clear();
    textInputs.clear();
  }

  static void Resize(HWND hwnd, int w, int h) {
    if (backDC) {
      SelectObject(backDC, oldBitmap);
      DeleteObject(backBitmap);
      DeleteDC(backDC);
    }
    Init(hwnd, w, h);
  }

  static void HandleMouse(UINT msg, LPARAM lParam) {
    mouseX = GET_X_LPARAM(lParam);
    mouseY = GET_Y_LPARAM(lParam);

    switch (msg) {
    case WM_LBUTTONDOWN:
        mouseDown = true;
        mousePressed = true;
        break;

    case WM_LBUTTONUP:
        mouseDown = false;
        mouseReleased = true;
        break;
    }

    for (auto &b : buttons) {
        bool inside = mouseX >= b.x && mouseX <= b.x + b.w &&
                      mouseY >= b.y && mouseY <= b.y + b.h;

        b.isHovered = inside;

        // --- Press detection for all buttons ---
        if (mousePressed && inside)
            b.isPressed = true;

        // --- Dragging (only if draggable) ---
        if (b.draggable) {
            if (mousePressed && inside) {
                b.isDragging = true;
                b.dragOffsetX = mouseX - b.x;
                b.dragOffsetY = mouseY - b.y;
            }

            if (b.isDragging && mouseDown) {
                b.x = mouseX - b.dragOffsetX;
                b.y = mouseY - b.dragOffsetY;
            }
        }

        // --- Release logic ---
        if (mouseReleased) {
            if (b.isPressed && inside)
                b.wasPressed = true;

            b.isPressed = false;
            b.isDragging = false;
        }
    }

    // --- TEXT INPUT FOCUS ---
    if (msg == WM_LBUTTONDOWN) {
        focusedInput = 0;
        for (auto &t : textInputs) {
            bool inside = mouseX >= t.x && mouseX <= t.x + t.w &&
                          mouseY >= t.y && mouseY <= t.y + t.h;
            t.isFocused = inside;
            if (inside)
                focusedInput = t.id;
    }
}
    // --- TEXT INPUT FOCUS ---
    if (msg == WM_LBUTTONDOWN) {
        focusedInput = 0;
        for (auto &t : textInputs) {
            bool inside = mouseX >= t.x && mouseX <= t.x + t.w &&
                          mouseY >= t.y && mouseY <= t.y + t.h;
            t.isFocused = inside;
            if (inside)
                focusedInput = t.id;
        }
    }
}

  static UIID CreateButton(int x, int y, int w, int h, const std::wstring &text,
                           COLORREF bg = RGB(40, 40, 40),
                           COLORREF hover = RGB(60, 60, 60),
                           COLORREF active = RGB(90, 90, 90)) {
    UIID id = GenerateID();
    buttons.push_back({id, x, y, w, h, text, {bg}, {hover}, {active}});
    return id;
  }

  static void HandleChar(WPARAM wParam) {
    if (!focusedInput)
      return;

    for (auto &t : textInputs) {
      if (t.id != focusedInput)
        continue;

      if (wParam == VK_BACK) {
        if (!t.text.empty() && t.cursorPos > 0) {
          t.text.erase(t.cursorPos - 1, 1);
          t.cursorPos--;
        }
      } else if (wParam == VK_RETURN) {
        t.isFocused = false;
        focusedInput = 0;
      } else if (wParam >= 32) {
        t.text.insert(t.cursorPos, 1, (wchar_t)wParam);
        t.cursorPos++;
      }
    }
  }
  static bool IsButtonPressed(UIID id) {
    for (auto &b : buttons)
      if (b.id == id && b.wasPressed) {
        b.wasPressed = false;
        return true;
      }
    return false;
  }

  static bool IsButtonDown(UIID id) {
    for (auto &b : buttons)
      if (b.id == id)
        return b.isPressed;
    return false;
  }

  static bool IsButtonHovered(UIID id) {
    for (auto &b : buttons)
      if (b.id == id)
        return b.isHovered;
    return false;
  }

  static UIID DrawRectangle(int x, int y, int w, int h, COLORREF color) {
    UIID id = GenerateID();
    rects.push_back({id, x, y, w, h, {color}});
    return id;
  }

  static UIID DrawCircle(int x, int y, int r, COLORREF color) {
    UIID id = GenerateID();
    circles.push_back({id, x, y, r, {color}});
    return id;
  }

  static UIID DrawLine(int x1, int y1, int x2, int y2, COLORREF color,
                       int width = 1) {
    UIID id = GenerateID();
    lines.push_back({id, x1, y1, x2, y2, width, {color}});
    return id;
  }

  static UIID DrawText(int x, int y, const std::wstring &str, COLORREF color,
                       int fontSize = 16) {
    UIID id = GenerateID();
    texts.push_back({id, x, y, str, {color}, fontSize});
    return id;
  }

  static UIID DrawProgressBar(int x, int y, int w, int h, float percent,
                              COLORREF fg, COLORREF bg) {
    UIID id = GenerateID();
    bars.push_back({id, x, y, w, h, percent, {fg}, {bg}});
    return id;
  }

  static void KillRectangle(UIID id) { EraseByID(rects, id); }
  static void KillTextInput(UIID id) { EraseByID(textInputs, id); }
  static void KillCircle(UIID id) { EraseByID(circles, id); }
  static void KillButton(UIID id) { EraseByID(buttons, id); }
  static BOOL KillLine(UIID id) {
    EraseByID(lines, id);
    return TRUE;
  }
  static void KillText(UIID id) { EraseByID(texts, id); }
  static void KillProgressBar(UIID id) { EraseByID(bars, id); }
  static UIID CreateTextInput(int x, int y, int w, int h,
                              COLORREF bg = RGB(30, 30, 30),
                              COLORREF border = RGB(80, 80, 80),
                              COLORREF activeBorder = RGB(0, 120, 215)) {
    UIID id = GenerateID();
    textInputs.push_back({id, x, y, w, h, L"", {bg}, {border}, {activeBorder}});
    return id;
  }

  static std::wstring GetTextInputValue(UIID id) {
    for (auto &t : textInputs)
      if (t.id == id)
        return t.text;
    return L"";
  }
  static void ClearAll() {
    rects.clear();
    textInputs.clear();
    circles.clear();
    lines.clear();
    texts.clear();
    buttons.clear();
    bars.clear();
  }

  static void HandlePaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    if (!backDC) {
      OnPaint(hdc);
    } else {
      HBRUSH bg = CreateSolidBrush(RGB(0, 0, 0));
      RECT rc = {0, 0, backWidth(), backHeight()};
      FillRect(backDC, &rc, bg);
      DeleteObject(bg);

      OnPaint(backDC);

      BitBlt(hdc, 0, 0, backWidth(), backHeight(), backDC, 0, 0, SRCCOPY);
    }

    EndPaint(hwnd, &ps);
  }

private:
  static UIID GenerateID() { return idCounter++; }

  template <typename T> static void EraseByID(std::vector<T> &vec, UIID id) {
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                             [id](const T &v) { return v.id == id; }),
              vec.end());
  }

  static int backWidth() { return width; }
  static int backHeight() { return height; }

  static void OnPaint(HDC hdc) {

    // --- DRAW IMAGE FIRST ---
    if (gdiImage) {
      HDC memDC = CreateCompatibleDC(hdc);
      HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, gdiImage);

      BITMAP bm;
      GetObject(gdiImage, sizeof(bm), &bm);

      BitBlt(hdc, imageX, imageY, imageW, imageH, memDC, 0, 0, SRCCOPY);

      SelectObject(memDC, oldBmp);
      DeleteDC(memDC);
    }

    // --- RECTANGLES ---
    for (auto &r : rects) {
      HBRUSH brush = CreateSolidBrush(r.color.value);
      RECT rc = {r.x, r.y, r.x + r.w, r.y + r.h};
      FillRect(hdc, &rc, brush);
      DeleteObject(brush);
    }

    // --- CIRCLES ---
    for (auto &c : circles) {
      HBRUSH brush = CreateSolidBrush(c.color.value);
      HGDIOBJ old = SelectObject(hdc, brush);
      Ellipse(hdc, c.x - c.r, c.y - c.r, c.x + c.r, c.y + c.r);
      SelectObject(hdc, old);
      DeleteObject(brush);
    }

    // --- LINES ---
    for (auto &l : lines) {
      HPEN pen = CreatePen(PS_SOLID, l.width, l.color.value);
      HPEN old = (HPEN)SelectObject(hdc, pen);
      MoveToEx(hdc, l.x1, l.y1, nullptr);
      LineTo(hdc, l.x2, l.y2);
      SelectObject(hdc, old);
      DeleteObject(pen);
    }

    // --- TEXT ---
    for (auto &t : texts) {
      SetTextColor(hdc, t.color.value);
      SetBkMode(hdc, TRANSPARENT);
      HFONT font = CreateFontW(t.fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
                               FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
      HGDIOBJ old = SelectObject(hdc, font);
      TextOutW(hdc, t.x, t.y, t.str.c_str(), (int)t.str.length());
      SelectObject(hdc, old);
      DeleteObject(font);
    }

    // --- BUTTONS ---
    for (auto &b : buttons) {
      COLORREF drawColor = b.bg.value;
      if (b.isPressed)
        drawColor = b.active.value;
      else if (b.isHovered)
        drawColor = b.hover.value;

      HBRUSH brush = CreateSolidBrush(drawColor);
      RECT rc = {b.x, b.y, b.x + b.w, b.y + b.h};
      FillRect(hdc, &rc, brush);
      DeleteObject(brush);

      SetTextColor(hdc, RGB(255, 255, 255));
      SetBkMode(hdc, TRANSPARENT);
      HFONT font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
      HGDIOBJ old = SelectObject(hdc, font);
      SIZE size;
      GetTextExtentPoint32W(hdc, b.text.c_str(), (int)b.text.length(), &size);
      int tx = b.x + (b.w - size.cx) / 2;
      int ty = b.y + (b.h - size.cy) / 2;
      TextOutW(hdc, tx, ty, b.text.c_str(), (int)b.text.length());
      SelectObject(hdc, old);
      DeleteObject(font);
    }

    // --- TEXT INPUTS ---
    for (auto &t : textInputs) {

      COLORREF borderColor =
          t.isFocused ? t.activeBorder.value : t.border.value;

      // Background
      HBRUSH bgBrush = CreateSolidBrush(t.bg.value);
      RECT rc = {t.x, t.y, t.x + t.w, t.y + t.h};
      FillRect(hdc, &rc, bgBrush);
      DeleteObject(bgBrush);

      // Border
      HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
      HPEN oldPen = (HPEN)SelectObject(hdc, pen);
      HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
      Rectangle(hdc, t.x, t.y, t.x + t.w, t.y + t.h);
      SelectObject(hdc, oldBrush);
      SelectObject(hdc, oldPen);
      DeleteObject(pen);

      // Text
      SetTextColor(hdc, RGB(255, 255, 255));
      SetBkMode(hdc, TRANSPARENT);

      HFONT font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

      HGDIOBJ oldFont = SelectObject(hdc, font);

      TextOutW(hdc, t.x + 5, t.y + 5, t.text.c_str(), (int)t.text.length());

      // Cursor
      if (t.isFocused) {
        SIZE size;
        std::wstring sub = t.text.substr(0, t.cursorPos);
        GetTextExtentPoint32W(hdc, sub.c_str(), (int)sub.length(), &size);

        int cx = t.x + 5 + size.cx;
        int cy = t.y + 5;

        MoveToEx(hdc, cx, cy, nullptr);
        LineTo(hdc, cx, cy + 16);
      }

      SelectObject(hdc, oldFont);
      DeleteObject(font);
    }

    // --- PROGRESS BARS ---
    for (auto &b : bars) {
      HBRUSH bgBrush = CreateSolidBrush(b.bg.value);
      RECT rcBg = {b.x, b.y, b.x + b.w, b.y + b.h};
      FillRect(hdc, &rcBg, bgBrush);
      DeleteObject(bgBrush);
      HBRUSH fgBrush = CreateSolidBrush(b.fg.value);
      RECT rcFg = {b.x, b.y, b.x + (int)(b.w * b.percent), b.y + b.h};
      FillRect(hdc, &rcFg, fgBrush);
      DeleteObject(fgBrush);
    }
  }
  
};

}; // namespace UI