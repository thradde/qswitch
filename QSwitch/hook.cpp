

#define STRICT
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <tchar.h>
#include <process.h>

#include "hook.h"


enum EMouseCorner
{
	enMcNone,
	enMcLeftTop,
	enMcRightTop,
	enMcLeftBottom,
	enMcRightBottom,
};


// --------------------------------------------------------------------------------------------------------------------------------------------
//															Globals
// --------------------------------------------------------------------------------------------------------------------------------------------
BYTE gbHotkeyVKey;				// virtual key code for app activation
BYTE gbHotkeyModifiers;			// modifiers for app activation (shift, ctrl, alt)

BYTE gbFilterHotkeyVKey;		// virtual key code for process-filtered view app activation
BYTE gbFilterHotkeyModifiers;	// modifiers for process-filtered view app activation (shift, ctrl, alt, win)

bool gbAltKeyDown;
bool gbCtrlKeyDown;
bool gbShiftKeyDown;
bool gbWinKeyDown;

int gnMouseButton;				// mouse button that activates: 0 = none, 1 = middle, 2 = xbutton1, 3 = xbutton2
int gnMouseCorner;				// if moving the mouse into the left-top corner will activate: 0 = no, 1 = yes
bool gbSuspendHooks;			// de-activate keyboard and mouse hooks, e.g. when gaming
EMouseCorner genMouseCorner;	// corner of screen that activates app when mouse is moved there

volatile bool gbHookEvent;		// true, if there was an event that did interest us.
								// This is either a mouse button event, a keyboard event or a progman event
								// After reading out "true", the consumer must set this flag = false
volatile bool gbEatSingleClick;	// same as above for mouse single click (used for checking if app was activated)
volatile bool gbHookDoubleClick;// same as above for mouse double click (used for checking for double click on desktop)
volatile bool gbFilterKey;		// true, if process-filtered view app activation

static HWND		_hWndApp;
static int		gnDoubleClickTime;	// user preferences value from registry
static __int64	gnDblClickStartTime;
static __int64	gnPerformanceFreq;

static HHOOK	_kbd_hook;
static HHOOK	_mouse_hook;

static RECT		_rcHoverArea;		// if mouse pointer hovers over this area, it will trigger the gbHookEvent


void CheckSysKeys()
{
	gbAltKeyDown = (GetKeyState(VK_MENU) & 0x80) != 0;
	gbCtrlKeyDown = (GetKeyState(VK_CONTROL) & 0x80) != 0;
	gbShiftKeyDown = (GetKeyState(VK_SHIFT) & 0x80) != 0;
	gbWinKeyDown = (GetKeyState(VK_LWIN) & 0x80) != 0 || (GetKeyState(VK_RWIN) & 0x80) != 0;
}


inline bool IsAltKeyDown()
{
	return (GetKeyState(VK_MENU) & 0x80) != 0;
}

inline bool IsCtrlKeyDown()
{
	return (GetKeyState(VK_CONTROL) & 0x80) != 0;
}

inline bool IsShiftKeyDown()
{
	return (GetKeyState(VK_SHIFT) & 0x80) != 0;
}

inline bool IsWinKeyDown()
{
	return (GetKeyState(VK_LWIN) & 0x80) || (GetKeyState(VK_RWIN) & 0x80);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															KbdHookCallback()
// --------------------------------------------------------------------------------------------------------------------------------------------
LRESULT __stdcall KbdHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!gbSuspendHooks && gbHotkeyVKey != 0 && nCode >= 0)
    {
        if (wParam == WM_KEYDOWN)
        {
			KBDLLHOOKSTRUCT *kbdStruct = ((KBDLLHOOKSTRUCT *)lParam);
			bool hot1 = (BYTE)(kbdStruct->vkCode & 0xff) == gbHotkeyVKey;
			bool hot2 = (BYTE)(kbdStruct->vkCode & 0xff) == gbFilterHotkeyVKey;
			gbFilterKey = false;

			if (hot1 || hot2)
			{
				CheckSysKeys();

				if (hot1)
				{
					if ((gbHotkeyModifiers & HotkeyAlt) && !gbAltKeyDown)
						goto bailout;

					if ((gbHotkeyModifiers & HotkeyCtrl) && !gbCtrlKeyDown)
						goto bailout;

					if ((gbHotkeyModifiers & HotkeyShift) && !gbShiftKeyDown)
						goto bailout;

					if ((gbHotkeyModifiers & HotkeyWin) && !gbWinKeyDown)
						goto bailout;

					gbHookEvent = true;
					return 1;
				}

				if (hot2)
				{
					if ((gbFilterHotkeyModifiers & HotkeyAlt) && !gbAltKeyDown)
						goto bailout;

					if ((gbFilterHotkeyModifiers & HotkeyCtrl) && !gbCtrlKeyDown)
						goto bailout;

					if ((gbFilterHotkeyModifiers & HotkeyShift) && !gbShiftKeyDown)
						goto bailout;

					if ((gbFilterHotkeyModifiers & HotkeyWin) && !gbWinKeyDown)
						goto bailout;

					gbHookEvent = true;
					gbFilterKey = true;
					return 1;
				}
			}
		}
    }
 
bailout:
    return CallNextHookEx(_kbd_hook, nCode, wParam, lParam);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															CheckDoubleClick()
// --------------------------------------------------------------------------------------------------------------------------------------------
static void CheckDoubleClick()
{
	// Can not receive WM_xyzDBLCLK through this low-level hook! So we synthesize it ourselves.
	if (gnDblClickStartTime == 0)
		QueryPerformanceCounter((LARGE_INTEGER *)&gnDblClickStartTime);
	else
	{
		__int64 now, elapsed;
		QueryPerformanceCounter((LARGE_INTEGER *)&now);
		elapsed = (now - gnDblClickStartTime) / gnPerformanceFreq;
		if (elapsed <= gnDoubleClickTime)
		{
			gbHookDoubleClick = true;
			gnDblClickStartTime = 0;
		}
		else
			QueryPerformanceCounter((LARGE_INTEGER *)&gnDblClickStartTime);
	}
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															MouseHookCallback()
// --------------------------------------------------------------------------------------------------------------------------------------------
LRESULT __stdcall MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!gbSuspendHooks && nCode >= 0)
    {
		MSLLHOOKSTRUCT *mouseStruct = ((MSLLHOOKSTRUCT *)lParam);

		POINT pt;
		pt.x = mouseStruct->pt.x;
		pt.y = mouseStruct->pt.y;

		bool IsNotForegroundWindow = GetForegroundWindow() != _hWndApp;

		if (wParam == WM_MOUSEMOVE && gnMouseCorner != 0)
		{
            if (pt.x >= _rcHoverArea.left && pt.x <= _rcHoverArea.right &&
				pt.y >= _rcHoverArea.top  && pt.y <= _rcHoverArea.bottom &&
				IsNotForegroundWindow)
			{
				gbHookEvent = true;
				gbFilterKey = false;
			}
		}
		else if ((wParam == WM_MBUTTONUP || wParam == WM_MBUTTONDOWN) && gnMouseButton == 1)
		{
			// we must eat BUTTONDOWN, otherwise the mouse input of this app hangs
			if (wParam == WM_MBUTTONUP)
			{
				gbHookEvent = true;
				gbFilterKey = false;
				CheckDoubleClick();
			}
			return 1;
		}
        else if ((wParam == WM_XBUTTONUP || wParam == WM_XBUTTONDOWN) && gnMouseButton >= 2 &&
				 (HIWORD(mouseStruct->mouseData) & (gnMouseButton - 1)))
        {
			// we must eat BUTTONDOWN, otherwise the mouse input of this app hangs
			if (wParam == WM_XBUTTONUP)
			{
				gbHookEvent = true;
				gbFilterKey = false;
				CheckDoubleClick();
			}
			return 1;
        }
    }
 
    return CallNextHookEx(_mouse_hook, nCode, wParam, lParam);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															SetHookAppWindow()
// --------------------------------------------------------------------------------------------------------------------------------------------
void SetHookAppWindow(HWND hWndApp)
{
	_hWndApp = hWndApp;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//																SetHooks()
// --------------------------------------------------------------------------------------------------------------------------------------------
static bool SetHooks()
{
	// setup variables
	_hWndApp = NULL;

	gbHotkeyVKey = VK_F8;
	gbHotkeyModifiers = HotkeyWin;

	gbFilterHotkeyVKey = VK_F7;
	gbFilterHotkeyModifiers = HotkeyWin;
	gbFilterKey = false;

	gnMouseButton = 1;			// middle mouse-button
	gnMouseCorner = 0;			// no
	gbSuspendHooks = false;

	gbHookEvent = false;
	gbEatSingleClick = false;
	gbHookDoubleClick = false;

	gnDoubleClickTime = GetDoubleClickTime();
	QueryPerformanceFrequency((LARGE_INTEGER *)&gnPerformanceFreq);
	gnPerformanceFreq /= 1000;	// we want milliseconds later
	gnDblClickStartTime = 0;

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	_rcHoverArea.left = width - 5;
	_rcHoverArea.right = width + 10;
	_rcHoverArea.top = -10;
	_rcHoverArea.bottom = 5;

	// create hooks
    if (!(_kbd_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KbdHookCallback, NULL, 0)))
		return false;

	if (!(_mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, NULL, 0)))
		return false;

	return true;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															ReleaseHooks()
// --------------------------------------------------------------------------------------------------------------------------------------------
static void ReleaseHooks()
{
    UnhookWindowsHookEx(_kbd_hook);
    UnhookWindowsHookEx(_mouse_hook);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															RunHookThread()
//
// The hooks are run in a separate thread, because they need a fast message loop.
// The SFML message loop is rather slow, because it paints a lot of stuff (and we have a Sleep() in  it).
// Using the SFML message loop, one can notice a significant slow-down, when AutoHotkey sends a group of single keystrokes.
// Using this thread, the slow-down is gone.
// There is no cleanup code for the thread, when the main app is terminated, the thread and hooks are terminated cleanly by Windows.
// --------------------------------------------------------------------------------------------------------------------------------------------
static HANDLE _hHookThread;

#ifndef _DEBUG		// declare _SetThreadName()
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;		// Must be 0x1000.
	LPCTSTR szName;		// Pointer to name (in user addr space).
	DWORD dwThreadID;	// Thread ID (-1=caller thread).
	DWORD dwFlags;		// Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void _SetThreadName(DWORD dwThreadID, LPCTSTR threadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

#define SetThreadName(id, name)		_SetThreadName(id, name)

#else
	#define SetThreadName(id, name)
#endif


unsigned __stdcall HookThread(void *param)
{
	SetThreadName(-1, _T("HookThread"));	// this is only for the debugger
	SetHooks();

#ifndef _DEBUG
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif

	ReleaseHooks();
	return 1;
}


void RunHookThread()
{
	unsigned aID;
	_hHookThread = (HANDLE)_beginthreadex(NULL, 0, HookThread, NULL, 0, &aID);
}
