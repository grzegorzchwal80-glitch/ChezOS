/*
File is open source and under GNU GPL v3.0 license. See LICENSE for details.
*/

#pragma once
#include "UI.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <windows.h>


namespace UI {
namespace Style {
  
using ID = UIID;
using ColorRef = COLORREF;

struct Vec2 {
  int x, y;
};
struct Padding {
  int left, top, right, bottom;
};
struct Margin {
  int left, top, right, bottom;
};
struct Shadow {
  int offsetX = 0, offsetY = 0, blur = 0;
  COLORREF color = RGB(0, 0, 0);
};
struct Border {
  int thickness = 1;
  COLORREF color = RGB(255, 255, 255);
};
struct Gradient {
  COLORREF start;
  COLORREF end;
};

inline Rect *FindRect(ID id) {
  for (auto &r : Window::rects)
    if (r.id == id)
      return &r;
  return nullptr;
}
inline Button* FindButton(ID id) {
  for (auto &b : Window::buttons)
    if (b.id == id)
      return &b;
  return nullptr;
}
inline Circle *FindCircle(ID id) {
  for (auto &c : Window::circles)
    if (c.id == id)
      return &c;
  return nullptr;
}
inline Line *FindLine(ID id) {
  for (auto &l : Window::lines)
    if (l.id == id)
      return &l;
  return nullptr;
}
inline Text *FindText(ID id) {
  for (auto &t : Window::texts)
    if (t.id == id)
      return &t;
  return nullptr;
}
inline ProgressBar *FindBar(ID id) {
  for (auto &b : Window::bars)
    if (b.id == id)
      return &b;
  return nullptr;
}

inline void SetRectangleColor(ID id, ColorRef c) {
  if (auto *r = FindRect(id))
    r->color.value = c;
}
inline void SetButtonDraggable(ID id, bool v) {
  if (auto* b = FindButton(id))
    b->draggable = v;
}
inline void SetCircleColor(ID id, ColorRef c) {
  if (auto *c2 = FindCircle(id))
    c2->color.value = c;
}
inline void SetLineColor(ID id, ColorRef c) {
  if (auto *l = FindLine(id))
    l->color.value = c;
}
inline void SetTextColor(ID id, ColorRef c) {
  if (auto *t = FindText(id))
    t->color.value = c;
}
inline void SetProgressBarFG(ID id, ColorRef c) {
  if (auto *b = FindBar(id))
    b->fg.value = c;
}
inline void SetProgressBarBG(ID id, ColorRef c) {
  if (auto *b = FindBar(id))
    b->bg.value = c;
}

inline void SetRectangleSize(ID id, int w, int h) {
  if (auto *r = FindRect(id)) {
    r->w = w;
    r->h = h;
  }
}
inline void SetCircleRadius(ID id, int radius) {
  if (auto *c = FindCircle(id))
    c->r = radius;
}
inline void SetLineWidth(ID id, int width) {
  if (auto *l = FindLine(id))
    l->width = width;
}
inline void SetTextSize(ID id, int size) {
  if (auto *t = FindText(id))
    t->fontSize = size;
}
inline void SetTextPosition(ID id, int x, int y) {
  if (auto *t = FindText(id)) {
    t->x = x;
    t->y = y;
  }
}
inline void SetProgress(ID id, float percent) {
  if (auto *b = FindBar(id))
    b->percent = std::clamp(percent, 0.0f, 1.0f);
}

inline void MoveRectangle(ID id, int x, int y) {
  if (auto *r = FindRect(id)) {
    r->x = x;
    r->y = y;
  }
}
inline void MoveCircle(ID id, int x, int y) {
  if (auto *c = FindCircle(id)) {
    c->x = x;
    c->y = y;
  }
}
inline void MoveText(ID id, int x, int y) {
  if (auto *t = FindText(id)) {
    t->x = x;
    t->y = y;
  }
}

inline void SetTextString(ID id, const std::wstring &str) {
  if (auto *t = FindText(id))
    t->str = str;
}
inline void AppendText(ID id, const std::wstring &str) {
  if (auto *t = FindText(id))
    t->str += str;
}
inline void ClearText(ID id) {
  if (auto *t = FindText(id))
    t->str.clear();
}

inline void FadeToColor(ID id, ColorRef target, float factor) {
  auto lerp = [](int a, int b, float f) {
    return static_cast<int>(a + (b - a) * f);
  };
  if (auto *r = FindRect(id)) {
    COLORREF c = r->color.value;
    r->color.value = RGB(lerp(GetRValue(c), GetRValue(target), factor),
                         lerp(GetGValue(c), GetGValue(target), factor),
                         lerp(GetBValue(c), GetBValue(target), factor));
  }
}

inline void GrowCircle(ID id, int delta) {
  if (auto *c = FindCircle(id))
    c->r += delta;
}
inline void PulseCircle(ID id, float time) {
  if (auto *c = FindCircle(id))
    c->r += static_cast<int>(std::sin(time) * 2.0f);
}

inline bool IsPointInsideRect(ID id, int mx, int my) {
  if (auto *r = FindRect(id))
    return mx >= r->x && mx <= r->x + r->w && my >= r->y && my <= r->y + r->h;
  return false;
}
inline bool IsPointInsideCircle(ID id, int mx, int my) {
  if (auto *c = FindCircle(id)) {
    int dx = mx - c->x, dy = my - c->y;
    return dx * dx + dy * dy <= c->r * c->r;
  }
  return false;
}

struct Theme {
  COLORREF primary = RGB(40, 40, 40), secondary = RGB(70, 70, 70),
           accent = RGB(0, 120, 215), text = RGB(255, 255, 255);
};
inline Theme CurrentTheme;
inline void ApplyThemeToRect(ID id) {
  if (auto *r = FindRect(id))
    r->color.value = CurrentTheme.primary;
}
inline void ApplyThemeToText(ID id) {
  if (auto *t = FindText(id))
    t->color.value = CurrentTheme.text;
}
inline void SetTheme(const Theme &t) { CurrentTheme = t; }

inline void ColorAllRectangles(ColorRef c) {
  for (auto &r : Window::rects)
    r.color.value = c;
}
inline void ColorAllCircles(ColorRef c) {
  for (auto &c2 : Window::circles)
    c2.color.value = c;
}
inline void ClearAllText() {
  for (auto &t : Window::texts)
    t.str.clear();
}

class Builder {
  ID m_id;

public:
  Builder(ID id) : m_id(id) {}
  Builder &Color(ColorRef c) {
    SetRectangleColor(m_id, c);
    SetCircleColor(m_id, c);
    SetTextColor(m_id, c);
    return *this;
  }
  Builder& Draggable(bool v) {
  SetButtonDraggable(m_id, v);
  return *this;
}
  Builder &Size(int w, int h) {
    SetRectangleSize(m_id, w, h);
    return *this;
  }
  Builder &Radius(int r) {
    SetCircleRadius(m_id, r);
    return *this;
  }
  Builder &TextSize(int s) {
    SetTextSize(m_id, s);
    return *this;
  }
  Builder &Move(int x, int y) {
    MoveRectangle(m_id, x, y);
    MoveCircle(m_id, x, y);
    MoveText(m_id, x, y);
    return *this;
  }
  Builder &TextPos(int x, int y) {
    SetTextPosition(m_id, x, y);
    return *this;
  }
};

inline Builder Make(ID id) { return Builder(id); }
} // namespace Style
} // namespace UI
