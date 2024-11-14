#include "pch.h"
#include "LoaderForm.h"
#include <windows.h>
#include <shlwapi.h>  // ���� PathCombineA �� PathRemoveFileSpecA
#include <stdio.h>    // ���� printf

// �ֶ����� _countof ��
#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof((array)[0]))
#endif

// ��ȡ��ǰDLL��·��
void GetCurrentDllPath(char* buffer, size_t bufferSize)
{
    HMODULE hModule = GetModuleHandleA("ImportLoader.dll");  // �滻Ϊ���DLL����
    if (hModule != NULL)
    {
        GetModuleFileNameA(hModule, buffer, (DWORD)bufferSize);
    }
}

// ��ȡ��ǰDLL���ڵ�Ŀ¼
void GetCurrentDllDirectory(char* buffer, size_t bufferSize)
{
    char fullPath[MAX_PATH] = { 0 };
    GetCurrentDllPath(fullPath, _countof(fullPath));

    if (*fullPath != '\0')
    {
        PathRemoveFileSpecA(fullPath);
        strncpy_s(buffer, bufferSize, fullPath, _countof(fullPath));
    }
}

// ����ص�����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hButton;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        // ������ť�ؼ�
        hButton = CreateWindowA(
            "BUTTON",                   // Ԥ����İ�ť��
            "Inject",                   // ��ť�ϵ��ı�
            WS_VISIBLE | WS_CHILD,       // ��ʽ
            10, 10, 100, 50,             // λ�úʹ�С
            hwnd,                        // ������
            (HMENU)1,                    // �ؼ�ID
            GetModuleHandleA(NULL),      // ʵ�����
            NULL                         // ��������
        );
    }
    break;

    case WM_COMMAND:
    {
        // ����ť����¼�
        if (LOWORD(wParam) == 1) // �ؼ�IDΪ1�İ�ť�����
        {
            char dllDir[MAX_PATH] = { 0 };
            GetCurrentDllDirectory(dllDir, _countof(dllDir));

            // ����Ŀ��DLL��ͬһĿ¼��
            char targetDllPath[MAX_PATH] = { 0 };
            PathCombineA(targetDllPath, dllDir, "TargetDll.dll");

            // ���·���Խ��е���
            MessageBoxA(hwnd, targetDllPath, "Target DLL Path", MB_OK);

            // ����ļ��Ƿ����
            if (!PathFileExistsA(targetDllPath))
            {
                MessageBoxA(hwnd, "TargetDll.dll does not exist.", "Error", MB_OK);
                return 0;
            }

            HINSTANCE hinstLib = LoadLibraryA(targetDllPath);
            if (hinstLib == NULL)
            {
                // ��ȡ����ʧ�ܵĴ�����
                DWORD dwError = GetLastError();
                char errorMessage[256];
                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    errorMessage, 256, NULL);

                // ��ʾ������Ϣ
                char errorText[512];
                sprintf_s(errorText, _countof(errorText), "Cannot load library TargetDll.dll. Error: %s", errorMessage);
                MessageBoxA(hwnd, errorText, "Error", MB_OK);
            }
        }
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
    return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// ��������ʾ����ĺ���
void ShowForm()
{
    const char* CLASS_NAME = "YMLLoader";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "YMLLoader",     // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        GetModuleHandleA(NULL),  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return;
    }

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

// �첽��ʾ����ĺ���
extern "C" __declspec(dllexport) void ShowFormAsync()
{
    CreateThread(
        NULL,                   // Ĭ�ϰ�ȫ����
        0,                      // Ĭ�϶�ջ��С
        (LPTHREAD_START_ROUTINE)ShowForm,  // �̺߳���
        NULL,                   // �̺߳�������
        0,                      // Ĭ�ϴ�����־
        NULL                    // �߳�ID
    );
}