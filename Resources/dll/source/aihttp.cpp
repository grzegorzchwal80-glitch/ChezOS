#include "../../../Source/Main/C++/Include/UI.hpp"
#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

extern "C" {
__declspec(dllexport) UI::Button *sendBtn = nullptr;
__declspec(dllexport) char g_queryBuffer[1024] = {};
__declspec(dllexport) char g_responseBuffer[8192] = {};
__declspec(dllexport) char g_jsonBody[4096] = {};
__declspec(dllexport) int g_maxTokens = 800;
__declspec(dllexport) double g_temperature = 0.7;
__declspec(dllexport) char g_apiKey[256] = "or-YOUR-KEY";
__declspec(dllexport) wchar_t g_host[128] = L"openrouter.ai";
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

std::string ExtractAIContent(const std::string &json) {
  std::string key = "\"content\":\"";
  size_t start = json.find(key);
  if (start == std::string::npos)
    return "Failed to find response";
  start += key.length();
  size_t end = json.find("\"", start);
  if (end == std::string::npos)
    return "Invalid AI response.";
  std::string raw = json.substr(start, end - start);
  std::string clean;
  for (size_t i = 0; i < raw.size(); ++i) {
    if (raw[i] == '\\' && i + 1 < raw.size()) {
      char next = raw[i + 1];
      if (next == 'n')
        clean += '\n';
      else if (next == '"')
        clean += '"';
      else if (next == '\\')
        clean += '\\';
      else
        clean += next;
      ++i;
    } else
      clean += raw[i];
  }
  return clean;
}

std::string GetOpenRouterAnswerInternal(const char *query) {
  strncpy_s(g_queryBuffer, query, sizeof(g_queryBuffer) - 1);
  HINTERNET hSession =
      WinHttpOpen(L"ChezOS-AI/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession)
    return "Failed to open session.";
  HINTERNET hConnect =
      WinHttpConnect(hSession, g_host, INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (!hConnect) {
    WinHttpCloseHandle(hSession);
    return "Failed to connect.";
  }
  HINTERNET hRequest = WinHttpOpenRequest(
      hConnect, L"POST", L"/api/v1/chat/completions", nullptr,
      WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
  if (!hRequest) {
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return "Failed to create request.";
  }

  snprintf(g_jsonBody, sizeof(g_jsonBody),
           "{"
           "\"model\":\"openai/gpt-4.1\","
           "\"messages\":[{\"role\":\"system\",\"content\":\"You are a helpful "
           "AI assistant.\"},{\"role\":\"user\",\"content\":\"%s\"}],"
           "\"max_tokens\":%d,"
           "\"temperature\":%f"
           "}",
           g_queryBuffer, g_maxTokens, g_temperature);

  std::wstring headers =
      L"Content-Type: application/json\r\nAuthorization: Bearer ";
  std::wstring wApiKey(g_apiKey, g_apiKey + strlen(g_apiKey));
  headers += wApiKey + L"\r\n";
  BOOL sent = WinHttpSendRequest(hRequest, headers.c_str(), -1L,
                                 (LPVOID)g_jsonBody, (DWORD)strlen(g_jsonBody),
                                 (DWORD)strlen(g_jsonBody), 0);
  if (!sent || !WinHttpReceiveResponse(hRequest, nullptr)) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return "HTTP request failed.";
  }

  std::string response;
  DWORD size = 0;
  do {
    if (!WinHttpQueryDataAvailable(hRequest, &size) || size == 0)
      break;
    std::vector<char> buffer(size + 1);
    DWORD downloaded = 0;
    if (!WinHttpReadData(hRequest, buffer.data(), size, &downloaded))
      break;
    response.append(buffer.data(), downloaded);
  } while (size > 0);

  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);

  if (response.empty())
    return "AI response empty.";
  std::string result = ExtractAIContent(response);
  strncpy_s(g_responseBuffer, result.c_str(), sizeof(g_responseBuffer) - 1);
  return result;
}

extern "C" __declspec(dllexport) int
GetDuckDuckGoAnswer(const char *query, char *outBuffer, int bufferSize) {
  if (!query || !outBuffer || bufferSize <= 0)
    return -1;
  std::string result = GetOpenRouterAnswerInternal(query);
  if ((int)result.size() + 1 > bufferSize)
    return -2;
  strcpy_s(outBuffer, bufferSize, result.c_str());
  return (int)result.size();
}
// idk if this will work, if any of you want to help, do it.