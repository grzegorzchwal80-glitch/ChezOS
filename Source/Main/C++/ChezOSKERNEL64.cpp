/*
File is open source and under GNU GPL v3.0 license. See LICENSE for details.
*/

#include "../../../Resources/headers/json.hpp"
#include "Include/General.hpp"
#include "Include/KernelLoad.hpp"
#include "Include/Style.hpp"
#include "Include/UI.hpp"
#include "Include/Window.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <windows.h>
#include <wincrypt.h>
#include <mmsystem.h>
#include <wingdi.h>

#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"winmm.lib")

using json = nlohmann::json;

std::filesystem::path exepath()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

static std::filesystem::path diskRoot;
static std::filesystem::path currentDir;

static bool notepadOpen = false;
static std::wstring notepadText;
static std::wstring notepadTitle;

static bool filesearcherOpen = false;
static std::vector<UI::UIID> dirButtons;
static std::vector<std::filesystem::path> dirPaths;
static int explorerX = 0;
static int explorerY = 0;
static int explorerW = 0;
static int explorerH = 0;

std::string toUTF8(const std::wstring& w)
{
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, result.data(), sizeNeeded, nullptr, nullptr);
    return result;
}

bool isInsideDisk(const std::filesystem::path& p)
{
    auto canonDisk = std::filesystem::weakly_canonical(diskRoot);
    auto canonP = std::filesystem::weakly_canonical(p);
    auto diskStr = canonDisk.wstring();
    auto pStr = canonP.wstring();
    if (pStr.size() < diskStr.size()) return false;
    if (pStr.compare(0, diskStr.size(), diskStr) != 0) return false;
    if (pStr.size() == diskStr.size()) return true;
    wchar_t c = pStr[diskStr.size()];
    return c == L'\\' || c == L'/';
}

void notepad(const std::wstring& path)
{
    notepadOpen = true;
    notepadTitle = path;
    notepadText.clear();
    std::ifstream file(toUTF8(path));
    if (!file.is_open())
    {
        notepadText = L"Failed to open file.";
        return;
    }
    std::string line;
    while (std::getline(file, line))
        notepadText += std::wstring(line.begin(), line.end()) + L"\n";
    file.close();
}

void DrawNotepad(int w, int h)
{
    if (!notepadOpen) return;
    int x = w/2 - 300;
    int y = h/2 - 250;
    UI::Window::DrawRectangle(x, y, 600, 500, RGB(20,20,20));
    UI::Window::DrawText(x+10, y+10, notepadTitle, RGB(255,255,0), 18);
    UI::UIID closeBtn = UI::Window::CreateButton(x+520, y+10, 60,25,L"Close");
    if (UI::Window::IsButtonPressed(closeBtn))
        notepadOpen = false;
    int textY = y + 50;
    std::wstring buffer;
    for (wchar_t c : notepadText)
    {
        if (c == L'\n')
        {
            UI::Window::DrawText(x+10, textY, buffer, RGB(255,255,255),16);
            buffer.clear();
            textY += 20;
        }
        else buffer.push_back(c);
    }
    if (!buffer.empty())
        UI::Window::DrawText(x+10, textY, buffer, RGB(255,255,255),16);
}

void BuildDirectoryButtons()
{
    for (auto id : dirButtons) UI::Window::KillButton(id);
    dirButtons.clear();
    dirPaths.clear();
    int yOffset = 40;
    int buttonHeight = 20;
    try
    {
        std::vector<std::filesystem::directory_entry> entries;
        for (auto& e : std::filesystem::directory_iterator(currentDir, std::filesystem::directory_options::skip_permission_denied))
            entries.push_back(e);
        std::sort(entries.begin(), entries.end(), [](auto& a, auto& b){
            if (a.is_directory() != b.is_directory()) return a.is_directory() > b.is_directory();
            return a.path().filename() < b.path().filename();
        });
        for (auto& entry : entries)
        {
            std::wstring name = entry.path().filename().wstring();
            std::wstring label = entry.is_directory() ? L"[DIR]  " + name : L"[FILE] " + name;
            UI::UIID id = UI::Window::CreateButton(explorerX+10, explorerY+yOffset, explorerW-20, buttonHeight, label);
            dirButtons.push_back(id);
            dirPaths.push_back(entry.path());
            yOffset += buttonHeight + 5;
        }
    }
    catch (...) {}
}

void UpdateDirectoryButtons()
{
    for (size_t i = 0; i < dirButtons.size(); i++)
    {
        if (UI::Window::IsButtonPressed(dirButtons[i]))
        {
            auto& p = dirPaths[i];
            if (std::filesystem::is_directory(p))
            {
                currentDir = p;
                BuildDirectoryButtons();
            }
            else notepad(p.wstring());
        }
    }
}

void DrawBackButton(int x, int y)
{
    if (currentDir == diskRoot) return;
    static UI::UIID backBtn = 0;
    if (!backBtn) backBtn = UI::Window::CreateButton(x, y, 100, 20, L"..");
    if (UI::Window::IsButtonPressed(backBtn))
    {
        auto parent = std::filesystem::weakly_canonical(currentDir.parent_path());
        auto root = std::filesystem::weakly_canonical(diskRoot);
        if (parent.string().rfind(root.string(),0) == 0)
            currentDir = parent;
        backBtn = 0;
        BuildDirectoryButtons();
    }
}

int KernelObjects::Kernel()
{
    static bool initialized = false;
    if (!initialized)
    {
        auto exeDir = exepath();
        diskRoot = exeDir / "DISK";
        if (!std::filesystem::exists(diskRoot))
            std::filesystem::create_directory(diskRoot);
        currentDir = diskRoot;
        initialized = true;
    }

    RECT rc{};
    BOOL running = TRUE;
    static UI::UIID FileSearcher = 0;

    PlaySoundA("C:\\Users\\PC\\Documents\\myProject\\Files\\ChezOS\\Resources\\boot.wav", nullptr, SND_FILENAME | SND_ASYNC);

    while (running)
    {
        win32::Window::PollEvents();
        GetClientRect(win32::Window::m_hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        UI::Window::BeginFrame();
        UI::Window::DrawRectangle(0,0,w,h,RGB(128,128,128));
        UI::Window::DrawText(10,10,currentDir.wstring(),RGB(255,255,0),16);

        if (!FileSearcher)
        {
            FileSearcher = UI::Window::CreateButton(20,20,300,300,L"File Searcher");
            UI::Style::Make(FileSearcher).Draggable(true);
        }

        if (UI::Window::IsButtonPressed(FileSearcher))
        {
            filesearcherOpen = !filesearcherOpen;
            if (filesearcherOpen)
            {
                explorerX = w/2 - 350;
                explorerY = h/2 - 300;
                explorerW = 700;
                explorerH = 600;
                BuildDirectoryButtons();
            }
            else
            {
                for (auto id : dirButtons)
                    UI::Window::KillButton(id);
                dirButtons.clear();
                dirPaths.clear();
            }
        }

        if (filesearcherOpen)
        {
            UI::Window::DrawRectangle(explorerX, explorerY, explorerW, explorerH, RGB(12,12,12));
            DrawBackButton(explorerX+10, explorerY+10);
            UpdateDirectoryButtons();
        }

        DrawNotepad(w,h);
        UI::Window::EndFrame(win32::Window::m_hwnd);
        Sleep(1);
    }

    return 0;
}