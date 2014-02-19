#define WINVER 0x0500
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <time.h>
#pragma comment(lib, "user32.lib")

HHOOK hKeyboardHook;
std::vector<INPUT> EventQueue;
int ready = 1;

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

        if (CTRL_key != 0)
        {
            if (keyCode == 'y')
            {
                MessageBox(NULL, "CTRL-y was pressed\nLaunch your app here", "H O T K E Y", MB_OK); 
                CTRL_key = 0;
            }
            else if (keyCode == 'q')
            {
                MessageBox(NULL, "Shutting down", "H O T K E Y", MB_OK); 
                PostQuitMessage(0);    
            }
        }
        else if ((time(0) % 60 <= 10) && ALT_key == 0 && keyCode >= 'A' && keyCode <= 'z')
        {
            printf("%d ha ha, I intercepted your [%c] and changed it to [%c] %d\n", time(0), keyCode, keyCode+1, keyCode+1);

            INPUT ip;
            ready = 0;

            // Set up a generic keyboard event.
            ip.type = INPUT_KEYBOARD;
            ip.ki.wScan = 0; // hardware scan code for key
            ip.ki.time = 0;
            ip.ki.dwExtraInfo = 0;
         
            // Press the "A" key
            ip.ki.wVk = hookedKey; // virtual-key code for the "a" key
            ip.ki.dwFlags = 0; // 0 for key press
            SendInput(1, &ip, sizeof(INPUT));

            // Release the "A" key
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

DWORD WINAPI my_HotKey(LPVOID lpParm)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!hInstance) hInstance = LoadLibrary((LPCSTR) lpParm); 
    if (!hInstance) return 1;

    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent, hInstance, 0);
    MessageLoop();
    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}

int main(int argc, char** argv)
{
    HANDLE hThread;
    DWORD dwThread;

    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) my_HotKey, (LPVOID) argv[0], 0, &dwThread);

    if (!(argc > 1 && argv[1] == "dev"))
    {
        ShowWindow(FindWindowA("ConsoleWindowClass", NULL), FALSE);
    }

    if (hThread) return WaitForSingleObject(hThread, INFINITE);
    else return 1;
}