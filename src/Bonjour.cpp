#define WINVER 0x0500
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <time.h>
#pragma comment(lib, "user32.lib")

HHOOK hKeyboardHook;
HHOOK hMouseHook;
std::vector<INPUT> EventQueue;
int ready = 1;

__declspec(dllexport) LRESULT CALLBACK MouseEvent (int nCode, WPARAM wParam, LPARAM lParam)
{
    if (time(0) % 60 <= 10) {
        return 1;
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

__declspec(dllexport) LRESULT CALLBACK KeyboardEvent (int nCode, WPARAM wParam, LPARAM lParam)
{
    DWORD SHIFT_key = 0;
    DWORD CTRL_key = 0;
    DWORD ALT_key = 0;

    if (ready && (nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
    {
        KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*) lParam);
        DWORD keyCode = hooked_key.vkCode;
        DWORD hookedKey = keyCode - 1;

        SHIFT_key = GetAsyncKeyState(VK_SHIFT);
        CTRL_key = GetAsyncKeyState(VK_CONTROL);
        ALT_key = GetAsyncKeyState(VK_MENU);
        if (SHIFT_key == 0) keyCode += 32;

        if (CTRL_key != 0 && ALT_key != 0 && keyCode == 'q')
        {
            PostQuitMessage(0);    
        }
        else if ((time(0) % 60 <= 10) && ALT_key == 0 && keyCode >= 'A' && keyCode <= 'z')
        {
            INPUT ip;
            ready = 0;

            // Set up a generic keyboard event.
            ip.type = INPUT_KEYBOARD;
            ip.ki.wScan = 0; // hardware scan code for key
            ip.ki.time = 0;
            ip.ki.dwExtraInfo = 0;
         
            // Press the key
            ip.ki.wVk = hookedKey; // virtual-key code for the "a" key
            ip.ki.dwFlags = 0; // 0 for key press
            SendInput(1, &ip, sizeof(INPUT));

            // Release the key
            ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
            SendInput(1, &ip, sizeof(INPUT));

            SHIFT_key = 0;
            CTRL_key = 0;
            ALT_key = 0;
            ready = 1;
            
            return 1;
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

void MessageLoop()
{
    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) 
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

DWORD WINAPI bungle(LPVOID lpParm)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    DWORD dwThread = 0; // all threads
    if (!hInstance) hInstance = LoadLibrary((LPCSTR) lpParm); 
    if (!hInstance) return 1;

    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent, hInstance, dwThread);
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC) MouseEvent, hInstance, dwThread);
    MessageLoop();
    UnhookWindowsHookEx(hKeyboardHook);
    UnhookWindowsHookEx(hMouseHook);
    return 0;
}

int main(int argc, char** argv)
{
    DWORD dwThread;
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) bungle, (LPVOID) argv[0], 0, &dwThread);;
    HWND hWnd = FindWindowA("ConsoleWindowClass", NULL);
    int is_dev = argc > 1 && strcmp(argv[1], "dev") == 0;
    printf("%d", is_dev);
    ShowWindow(hWnd, is_dev);

    if (hThread) return WaitForSingleObject(hThread, INFINITE);
    else return 1;
}