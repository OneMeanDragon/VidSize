#define _CRT_SECURE_NO_WARNINGS // for _tcsncat

#include <windows.h>
#include <tchar.h>

#include "resource.h"

#define REGKEY_BLIZZ TEXT("Software\\Blizzard Entertainment\\")
#define KEYNAME      TEXT("Video Player\\")
#define NODE_MODE    TEXT("Mode")
#define NODE_VERSION TEXT("Version")
#define REG_DWORDSIZE sizeof(UINT);

bool __fastcall SetValue(const wchar_t* KeyName, const wchar_t* NodeName, UINT NodeValue)
{
    if (KeyName == nullptr || NodeName == nullptr) return false;
    wchar_t regkey_path[MAX_PATH]{};
    HKEY nKeyHandle;
    DWORD nDisposition = 0;

    _tcsncat(regkey_path, REGKEY_BLIZZ, (MAX_PATH - 1));
    _tcsncat(regkey_path, KeyName, (MAX_PATH - 1) - _tcsnlen(REGKEY_BLIZZ, MAX_PATH));

    LSTATUS lStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE, regkey_path, 0, NULL, 0, KEY_WRITE,
        (LPSECURITY_ATTRIBUTES)0x0, &nKeyHandle, &nDisposition);

    if (lStatus == ERROR_SUCCESS) {
        BYTE setValue[4];
        *(DWORD*)setValue = NodeValue;
        lStatus = RegSetValueEx(nKeyHandle, NodeName, 0, 4, setValue, 4);
        RegCloseKey(nKeyHandle);
    }

    return lStatus == ERROR_SUCCESS;
}

bool __fastcall RetrieveValue(const wchar_t* KeyName, const wchar_t* NodeName, UINT* NodeValue)
{
    if (KeyName == nullptr || NodeName == nullptr || NodeValue == nullptr) return false;

    wchar_t regkey_path[MAX_PATH]{};
    HKEY nKeyHandle;
    DWORD BufferLength = MAX_PATH;
    BYTE bLocalData[MAX_PATH]{};
    DWORD rType = 0;

    _tcsncat(regkey_path, REGKEY_BLIZZ, (MAX_PATH - 1));
    _tcsncat(regkey_path, KeyName, (MAX_PATH - 1) - _tcsnlen(REGKEY_BLIZZ, MAX_PATH));

    LSTATUS lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regkey_path, 0, KEY_READ, &nKeyHandle);

    if (lStatus == ERROR_SUCCESS) {
        lStatus = RegQueryValueEx(nKeyHandle, NodeName, (LPDWORD)0x0, &rType, (LPBYTE)&bLocalData, &BufferLength);
        RegCloseKey(nKeyHandle);
    }
    if (lStatus == ERROR_SUCCESS) {
        if (rType == 1) {
            *NodeValue = (UINT)atoi((const char*)&bLocalData);
            return true;
        }
        if (rType != 4) {
            return true;
        }
        *NodeValue = *(DWORD*)bLocalData;
    }

    return lStatus == ERROR_SUCCESS;
}

int OnCloseMain(HWND hWnd)
{
    return DestroyWindow(hWnd);
}

int OnPaintMain(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // All painting occurs here, between BeginPaint and EndPaint.
    FillRect(hdc, &ps.rcPaint, (HBRUSH)CreateSolidBrush(RGB(0, 0, 0)));
    EndPaint(hWnd, &ps);
    return true;
}

int OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    WORD wCommand = LOWORD(wParam);
    switch (wCommand) {
    case IDC_BUTTON1: { // Ok
        UINT isChecked = IsDlgButtonChecked(hWnd, IDC_RADIO2);
        SetValue(KEYNAME, NODE_MODE, isChecked == 0 ? 0x100 : 0x202);
        SetValue(KEYNAME, NODE_VERSION, 3);
        PostMessageA(hWnd, WM_CLOSE, 0, 0);
        break; }
    case IDC_BUTTON2: // Cancel
        PostMessageA(hWnd, WM_CLOSE, 0, 0);
        break;
    }
    return TRUE;
}

int OnInitalization(HWND hWnd)
{
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
    HICON IconSmall = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    if (IconSmall) {
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)IconSmall);
        DestroyIcon(IconSmall);
    }
    HICON IconLarge = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    if (IconLarge) {
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)IconLarge);
        DestroyIcon(IconLarge);
    }
    UINT locValue = 0x200;
    RetrieveValue(KEYNAME, NODE_MODE, &locValue);
    CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2 - (UINT)(((byte)(locValue >> 8) & 2) != 2));

    return TRUE;
}

LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
    case WM_INITDIALOG: return OnInitalization(hWnd);
    case WM_PAINT: return OnPaintMain(hWnd);
    case WM_COMMAND: return OnCommand(hWnd, wParam, lParam);
    case WM_SETFONT: return TRUE;
    case WM_MOVE: return TRUE;
    case WM_SIZE: return TRUE;
    case WM_NOTIFY: return TRUE;
    case WM_CLOSE: return OnCloseMain(hWnd);
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CTLCOLOREDIT: case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wParam, 0x000000);
        SetTextColor((HDC)wParam, 0xFFFFFF);
        return (BOOL)GetStockObject(BLACK_BRUSH);
    }

    return FALSE;
}

WPARAM CreateRunWindow(HINSTANCE hInst)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMMAIN), (HWND)0x0, (DLGPROC)WindowProc);
    return 0; // this wont get hit until the window is closed
}


int main(HINSTANCE hInstance)
{
    if (hInstance != GetModuleHandle(NULL)) return -1;
	return (int)CreateRunWindow(hInstance);
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    return main(hInstance);
}
#endif