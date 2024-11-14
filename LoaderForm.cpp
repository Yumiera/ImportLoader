#include "pch.h"
#include "LoaderForm.h"
#include <windows.h>
#include <shlwapi.h>  // 包含 PathCombineA 和 PathRemoveFileSpecA
#include <stdio.h>    // 包含 printf

// 手动定义 _countof 宏
#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof((array)[0]))
#endif

// 获取当前DLL的路径
void GetCurrentDllPath(char* buffer, size_t bufferSize)
{
    HMODULE hModule = GetModuleHandleA("ImportLoader.dll");  // 替换为你的DLL名称
    if (hModule != NULL)
    {
        GetModuleFileNameA(hModule, buffer, (DWORD)bufferSize);
    }
}

// 获取当前DLL所在的目录
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

// 窗体回调函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hButton;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        // 创建按钮控件
        hButton = CreateWindowA(
            "BUTTON",                   // 预定义的按钮类
            "Inject",                   // 按钮上的文本
            WS_VISIBLE | WS_CHILD,       // 样式
            10, 10, 100, 50,             // 位置和大小
            hwnd,                        // 父窗口
            (HMENU)1,                    // 控件ID
            GetModuleHandleA(NULL),      // 实例句柄
            NULL                         // 额外数据
        );
    }
    break;

    case WM_COMMAND:
    {
        // 处理按钮点击事件
        if (LOWORD(wParam) == 1) // 控件ID为1的按钮被点击
        {
            char dllDir[MAX_PATH] = { 0 };
            GetCurrentDllDirectory(dllDir, _countof(dllDir));

            // 假设目标DLL在同一目录下
            char targetDllPath[MAX_PATH] = { 0 };
            PathCombineA(targetDllPath, dllDir, "TargetDll.dll");

            // 输出路径以进行调试
            MessageBoxA(hwnd, targetDllPath, "Target DLL Path", MB_OK);

            // 检查文件是否存在
            if (!PathFileExistsA(targetDllPath))
            {
                MessageBoxA(hwnd, "TargetDll.dll does not exist.", "Error", MB_OK);
                return 0;
            }

            HINSTANCE hinstLib = LoadLibraryA(targetDllPath);
            if (hinstLib == NULL)
            {
                // 获取加载失败的错误码
                DWORD dwError = GetLastError();
                char errorMessage[256];
                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    errorMessage, 256, NULL);

                // 显示错误信息
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

// 创建并显示窗体的函数
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

// 异步显示窗体的函数
extern "C" __declspec(dllexport) void ShowFormAsync()
{
    CreateThread(
        NULL,                   // 默认安全属性
        0,                      // 默认堆栈大小
        (LPTHREAD_START_ROUTINE)ShowForm,  // 线程函数
        NULL,                   // 线程函数参数
        0,                      // 默认创建标志
        NULL                    // 线程ID
    );
}