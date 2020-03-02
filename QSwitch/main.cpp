
#define STRICT
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>

#include <SFML/Graphics.hpp>
#include "RoundedRectangleShape.h"

#include <Shellapi.h>		// for DragAcceptFiles()
#include <Shlobj.h>
#include <Psapi.h>
#include <Dwmapi.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <process.h>

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
using namespace std;

#include <gl/gl.h>
#include <gl/glu.h>
//#include <gl/glut.h>

#define ASSERT
#include "SystemTraySDK.h"
#include "resource.h"
#include "hook.h"
#include "RfwString.h"
#include "exceptions.h"
#include "Mutex.h"
#include "stream.h"
#include "platform.h"
#include "generic.h"
#include "configuration.h"
#include "IconGetter.h"
#include "popup_menu.h"
//#include "bitmap_cache.h"
#include "icon.h"
//#include "icon_page.h"
//#include "undo.h"
//#include "icon_manager.h"

#include "TrayIcon.c"		// created with Gimp


// Use commctrl32 version 6 for newer visual style
#pragma comment(linker,	"\"/manifestdependency:type='win32' \
						name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
						processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(linker, "/DEFAULTLIB:Dwmapi.lib")

#define APP_NAME				_T("QSwitch")
#define APP_CLASS_NAME			_T("QSwitchClass")
#define THUMBNAIL_CLASS_NAME	_T("QuSwThumbailClass")
#define DATA_FOLDER				_T("\\QSwitch\\")
#define SETTINGS_FILE			_T("QSwitch.settings")

bool gbTerminateActivityThread;
unsigned __stdcall ActivityThread(void *param);
unsigned __stdcall ResyncThread(void *param);

/*
todo:
- Scrollen, wenn mouse-down und dann mouse-move (mit mindest-move-pixel > 2 oder > 3), so dass also Touch-Scroll möglich ist

Kostenfreie Version:
- Nach 30 Tagen bei jedem SW_SHOW ==> NAG Screen zum wegklicken
*/


// --------------------------------------------------------------------------------------------------------------------------------------------
//																	Macros
// --------------------------------------------------------------------------------------------------------------------------------------------
#define EDIT_MAX_CHARS	1024


// --------------------------------------------------------------------------------------------------------------------------------------------
//																	const
// --------------------------------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------------------------------------
//																Globals
// --------------------------------------------------------------------------------------------------------------------------------------------
static HINSTANCE ghInstance;
TMapExplorerPaths g_mapExplorerPaths;

static CSettings gSettings;		// global settings, like rows, cols, font size, etc.

static TCHAR szDlgExchange[EDIT_MAX_CHARS + 1];


// --------------------------------------------------------------------------------------------------------------------------------------------
//															PickColorDialog()
// --------------------------------------------------------------------------------------------------------------------------------------------
bool PickColorDialog(HWND hwndParent, COLORREF &color)
{
	CHOOSECOLOR cc;						// common dialog box structure 
	static COLORREF acrCustClr[16] =	// array of custom colors 
	{
		RGB(0,     5,   5),
		RGB(0,    15,  55),
		RGB(0,    25, 155),
		RGB(0,    35, 255),
		RGB(10,    0,   5),
		RGB(10,   20,  55),
		RGB(10,   40, 155),
		RGB(10,   60, 255),
		RGB(100,   5,   5),
		RGB(100,  25,  55),
		RGB(100,  50, 155),
		RGB(100, 125, 255),
		RGB(200, 120,   5),
		RGB(200, 150,  55),
		RGB(200, 200, 155),
		RGB(200, 250, 255),
	};

	// Initialize CHOOSECOLOR 
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hwndParent;
	cc.lpCustColors = (LPDWORD)acrCustClr;
	cc.rgbResult = color;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc)) 
	{
		color = cc.rgbResult;
		return true;
	}

	return false;
}
// --------------------------------------------------------------------------------------------------------------------------------------------
//															HotKey Control Subclass
// --------------------------------------------------------------------------------------------------------------------------------------------
WNDPROC wpOrigEditProc;
WNDPROC wpOrigCmbProc;
WNDPROC wpOrigDropDownProc;

// instance data of hotkey control (so we can have multiple hotkey controls in the same dialog)
class CHotkeyData
{
public:
	BYTE	HotkeyModifiers;
	BYTE	VirtKeyCode;
	BYTE	StoredHotkeyModifiers;
	BYTE	StoredVirtKeyCode;
	RString strKeyState;
	bool	bRecordedShift;

public:
	CHotkeyData()
		: HotkeyModifiers(0),
		VirtKeyCode(0),
		StoredHotkeyModifiers(0),
		StoredVirtKeyCode(0),
		bRecordedShift(false)
	{
	}
};

typedef unordered_map<HWND, CHotkeyData> THotkeyData;
typedef THotkeyData::iterator THotkeyDataIter;

THotkeyData mapHotkeyData;

#define WM_MY_SETKEYS				(WM_USER)
#define WM_MY_GETHOTKEYMODIFIERS	(WM_USER + 1)
#define WM_MY_GETVIRTKEY			(WM_USER + 2)

// What the original HOTKEY_CLASS of Windows (found docu in the internet) does:
// WM_CHAR			Retrieves the virtual key code.
// WM_CREATE		Initializes the hot key control, clears any hot key rules, and uses the system font.
// WM_ERASEBKGND	Hides the caret, calls the DefWindowProc function, and shows the caret again.
// WM_GETDLGCODE	Returns a combination of the DLGC_WANTCHARS and DLGC_WANTARROWS values.
// WM_GETFONT		Retrieves the font.
// WM_KEYDOWN		Calls the DefWindowProc function if the key is ENTER, TAB, SPACE BAR, DEL, ESC, or BACKSPACE.If the key is SHIFT, CTRL, or ALT, it checks whether the combination is valid and, if it is, sets the hot key using the combination.All other keys are set as hot keys without their validity being checked first.
// WM_KEYUP			Retrieves the virtual key code.
// WM_KILLFOCUS		Destroys the caret.
// WM_LBUTTONDOWN	Sets the focus to the window.
// WM_NCCREATE		Sets the WS_EX_CLIENTEDGE window style.
// WM_PAINT			Paints the hot key control.
// WM_SETFOCUS		Creates and shows the caret.
// WM_SETFONT		Sets the font.
// WM_SYSCHAR		Retrieves the virtual key code.
// WM_SYSKEYDOWN	Calls the DefWindowProc function if the key is ENTER, TAB, SPACE BAR, DEL, ESC, or BACKSPACE.If the key is SHIFT, CTRL, or ALT, it checks whether the combination is valid and, if it is, sets the hot key using the combination.All other keys are set as hot keys without their validity being checked first.
// WM_SYSKEYUP		Retrieves the virtual key code.

// Edit Hotkey Subclass procedure
LRESULT APIENTRY EditHotkeyProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CHotkeyData &data = mapHotkeyData[hwnd];
	BYTE LastHotkeyModifiers;

	switch (uMsg)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTCHARS + DLGC_WANTARROWS;
		// return DLGC_WANTALLKEYS;

	case WM_CREATE:
		//mapHotkeyData.insert(pair<HWND, CHotkeyData>(hwnd, CHotkeyData())); ==> done by declaration: CHotkeyData &data = mapHotkeyData[hwnd];
		break;	// use break, so DefWindowProc of original control is called

	case WM_NCDESTROY:
		mapHotkeyData.erase(hwnd);
		break;	// use break, so DefWindowProc of original control is called

	case WM_MY_SETKEYS:
		data.StoredHotkeyModifiers = (BYTE)wParam;
		data.StoredVirtKeyCode = (BYTE)lParam;
		data.strKeyState = GetKeyComboString(data.StoredHotkeyModifiers, data.StoredVirtKeyCode);
		Edit_SetText(hwnd, data.strKeyState.c_str());
		break;

	case WM_MY_GETHOTKEYMODIFIERS:
		return data.StoredHotkeyModifiers;

	case WM_MY_GETVIRTKEY:
		return data.StoredVirtKeyCode;

	case WM_KILLFOCUS:
		if (data.StoredHotkeyModifiers == 0 || data.StoredVirtKeyCode == 0)
		{
			data.StoredHotkeyModifiers = 0;
			data.StoredVirtKeyCode = 0;
			data.strKeyState = GetKeyComboString(data.StoredHotkeyModifiers, data.StoredVirtKeyCode);
			Edit_SetText(hwnd, data.strKeyState.c_str());
		}
		data.HotkeyModifiers = 0;	// this one is important if edit control was quit by shift + tab keycombo
		data.VirtKeyCode = 0;
		break;

	case WM_SETFOCUS:
		// record keys that are pressed right now
		data.bRecordedShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		break;

	case WM_CHAR:
	case WM_SYSCHAR:
		return 0;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		switch (wParam)
		{
		case VK_MENU:	data.HotkeyModifiers &= ~HotkeyAlt; break;
		case VK_CONTROL:data.HotkeyModifiers &= ~HotkeyCtrl; break;
		
		case VK_SHIFT:
			if (data.bRecordedShift)
			{
				data.bRecordedShift = false;
				return 0;
			}
			data.HotkeyModifiers &= ~HotkeyShift;
			break;

		case VK_LWIN:
		case VK_RWIN:	data.HotkeyModifiers &= ~HotkeyWin; break;

		case VK_TAB:
			break;

		default:
			data.StoredVirtKeyCode = data.VirtKeyCode;
			data.VirtKeyCode = 0;
		}

		if (data.HotkeyModifiers == 0 && data.StoredVirtKeyCode == 0)
			data.VirtKeyCode = 0;

		if (data.StoredVirtKeyCode == 0)
			data.StoredHotkeyModifiers = data.HotkeyModifiers;

		data.strKeyState = GetKeyComboString(data.StoredHotkeyModifiers, data.StoredVirtKeyCode);
		Edit_SetText(hwnd, data.strKeyState.c_str());
		return 0;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		// Calls the DefWindowProc function if the key is ENTER, TAB, SPACE BAR, DEL, ESC, or BACKSPACE.
		// If the key is SHIFT, CTRL, or ALT, it checks whether the combination is valid and, if it is, 
		// sets the hot key using the combination.
		// All other keys are set as hot keys without their validity being checked first.
		LastHotkeyModifiers = data.HotkeyModifiers;
		switch (wParam)
		{
		case VK_DELETE:
		case VK_BACK:
			data.HotkeyModifiers = 0;
			data.VirtKeyCode = 0;
			break;

		case VK_RETURN:
		case VK_TAB:
		case VK_ESCAPE:
			return CallWindowProc(wpOrigEditProc, hwnd, uMsg, wParam, lParam);

		case VK_MENU:	data.HotkeyModifiers |= HotkeyAlt; break;
		case VK_CONTROL:data.HotkeyModifiers |= HotkeyCtrl; break;
		case VK_SHIFT:	data.HotkeyModifiers |= HotkeyShift; break;
		case VK_LWIN:
		case VK_RWIN:	data.HotkeyModifiers |= HotkeyWin; break;

		default:
			data.VirtKeyCode = (BYTE)(wParam & 0xff);
			if (data.HotkeyModifiers == 0)
			{
				data.StoredHotkeyModifiers = 0;
				data.StoredVirtKeyCode = 0;
				data.VirtKeyCode = 0;
			}
		}

		if (LastHotkeyModifiers != data.HotkeyModifiers)
		{
			data.VirtKeyCode = 0;
			data.StoredVirtKeyCode = 0;
		}

		if (data.StoredVirtKeyCode == 0)
			data.StoredHotkeyModifiers = data.HotkeyModifiers;
		//data.StoredVirtKeyCode = data.VirtKeyCode;
		data.strKeyState = GetKeyComboString(data.StoredHotkeyModifiers, data.VirtKeyCode);
		Edit_SetText(hwnd, data.strKeyState.c_str());
		return 0;
	}

	return CallWindowProc(wpOrigEditProc, hwnd, uMsg, wParam, lParam);
}


// Combobox Drop Down List Subclass procedure
LRESULT APIENTRY CmbDropDownProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_XBUTTONUP:
		{
			WORD btn = HIWORD(wParam);
			if (btn <= 5)
			{
				SendMessage(hwnd, LB_SETCURSEL, btn + 1, 0);
				PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 1);
			}

			return 0;
		}
		break;

	case WM_MBUTTONUP:
		SendMessage(hwnd, LB_SETCURSEL, 1, 0);
		PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 1);
		break;
	}

	return CallWindowProc(wpOrigDropDownProc, hwnd, uMsg, wParam, lParam);
}


// Combobox Mouse Button Subclass procedure
LRESULT APIENTRY CmbMouseBtnProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_XBUTTONUP:
		{
			WORD btn = HIWORD(wParam);
			if (btn <= 5)
				SendMessage(hwnd, CB_SETCURSEL, btn + 1, 0);

			return 0;
		}
		break;

	case WM_MBUTTONUP:
		SendMessage(hwnd, CB_SETCURSEL, 1, 0);
		break;
	}

	return CallWindowProc(wpOrigCmbProc, hwnd, uMsg, wParam, lParam);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															DlgProcSettings()
// --------------------------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK DlgSettings(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndComboBox;
	static COLORREF clrTmpBkgColor;

	switch (Msg)
	{
	case WM_INITDIALOG:
		// Subclass the hotkey edit control
		wpOrigEditProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hWndDlg, ID_HOTKEY), GWLP_WNDPROC, (LONG_PTR)EditHotkeyProc);
		SetWindowLongPtr(GetDlgItem(hWndDlg, ID_HOTKEY_FILTERED), GWLP_WNDPROC, (LONG_PTR)EditHotkeyProc);
		SendMessage(GetDlgItem(hWndDlg, ID_HOTKEY), WM_MY_SETKEYS, gbHotkeyModifiers, gbHotkeyVKey);
		SendMessage(GetDlgItem(hWndDlg, ID_HOTKEY_FILTERED), WM_MY_SETKEYS, gbFilterHotkeyModifiers, gbFilterHotkeyVKey);

		// Subclass the Mouse Button Combobox
		wpOrigCmbProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hWndDlg, ID_MOUSE_BUTTON), GWLP_WNDPROC, (LONG_PTR)CmbMouseBtnProc);

		// Subclass the drop down list of the Mouse Button Combobox
		COMBOBOXINFO cbi;
		cbi.cbSize = sizeof(COMBOBOXINFO);
		GetComboBoxInfo(GetDlgItem(hWndDlg, ID_MOUSE_BUTTON), &cbi);
		wpOrigDropDownProc = (WNDPROC)SetWindowLongPtr(cbi.hwndList, GWLP_WNDPROC, (LONG_PTR)CmbDropDownProc);

		hWndComboBox = GetDlgItem(hWndDlg, ID_MOUSE_BUTTON);
		SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)_T("None"));
		SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)_T("Middle Mouse Button"));
		SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)_T("Extra Button 1"));
		SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)_T("Extra Button 2"));
		SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)_T("Extra Button 3"));
		SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)_T("Extra Button 4"));
		SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)_T("Extra Button 5"));
		SendMessage(hWndComboBox, CB_SETCURSEL, gnMouseButton, 0);

		SendMessage(GetDlgItem(hWndDlg, ID_MOUSE_CORNER), BM_SETCHECK, gnMouseCorner ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hWndDlg, ID_DISABLE_MAXIMIZED), BM_SETCHECK, gSettings.m_bDisableIfMaxWin ? BST_CHECKED : BST_UNCHECKED, 0);

		SendMessage(GetDlgItem(hWndDlg, ID_TRANSPARENCY), TBM_SETRANGE, TRUE, MAKELONG(1, 100));
		SendMessage(GetDlgItem(hWndDlg, ID_TRANSPARENCY), TBM_SETPOS, TRUE, gSettings.m_nBkgTransparency);
		_itot(gSettings.m_nBkgTransparency, szDlgExchange, 10);
		SetDlgItemText(hWndDlg, ID_TRANSP_VALUE, szDlgExchange);

		clrTmpBkgColor = gSettings.m_clrBkgColor;
		return TRUE;

	case WM_NCDESTROY:
		// Remove the subclass from the edit control.
		SetWindowLongPtr(GetDlgItem(hWndDlg, ID_HOTKEY), GWLP_WNDPROC, (LONG_PTR)wpOrigEditProc);
		SetWindowLongPtr(GetDlgItem(hWndDlg, ID_HOTKEY_FILTERED), GWLP_WNDPROC, (LONG_PTR)wpOrigEditProc);
		break;

	case WM_HSCROLL:	// trackbar notification
	{
		WORD w = (WORD)SendMessage(GetDlgItem(hWndDlg, ID_TRANSPARENCY), TBM_GETPOS, 0, 0);
		_itot(w, szDlgExchange, 10);
		SetDlgItemText(hWndDlg, ID_TRANSP_VALUE, szDlgExchange);
	}
	break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_BROWSE_COLOR:
			if (PickColorDialog(hWndDlg, clrTmpBkgColor))
				InvalidateRect(hWndDlg, NULL, TRUE);
			break;

		case IDOK:
			gbHotkeyVKey = (BYTE)SendMessage(GetDlgItem(hWndDlg, ID_HOTKEY), WM_MY_GETVIRTKEY, 0, 0);
			gbHotkeyModifiers = (BYTE)SendMessage(GetDlgItem(hWndDlg, ID_HOTKEY), WM_MY_GETHOTKEYMODIFIERS, 0, 0);
			gbFilterHotkeyVKey = (BYTE)SendMessage(GetDlgItem(hWndDlg, ID_HOTKEY_FILTERED), WM_MY_GETVIRTKEY, 0, 0);
			gbFilterHotkeyModifiers = (BYTE)SendMessage(GetDlgItem(hWndDlg, ID_HOTKEY_FILTERED), WM_MY_GETHOTKEYMODIFIERS, 0, 0);

			gnMouseButton = (int)SendMessage(GetDlgItem(hWndDlg, ID_MOUSE_BUTTON), CB_GETCURSEL, 0, 0);
			gnMouseCorner = SendMessage(GetDlgItem(hWndDlg, ID_MOUSE_CORNER), BM_GETCHECK, 0, 0) == BST_CHECKED;
			gSettings.m_bDisableIfMaxWin = SendMessage(GetDlgItem(hWndDlg, ID_DISABLE_MAXIMIZED), BM_GETCHECK, 0, 0) == BST_CHECKED;

			gSettings.m_nBkgTransparency = (int)SendMessage(GetDlgItem(hWndDlg, ID_TRANSPARENCY), TBM_GETPOS, 0, 0);

			gSettings.m_clrBkgColor = clrTmpBkgColor;
			EndDialog(hWndDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hWndDlg, ID_COLOR))
		{
			// Change text color and text-bkg-color
			//HDC hdcStatic = (HDC)wParam;
			//SetTextColor(hdcStatic, RGB(0,0,255));
			//SetBkColor(hdcStatic, RGB(250,250,0));
			return (INT_PTR)CreateSolidBrush(clrTmpBkgColor);	// control's bkg color
		}
		break;
	}

	return FALSE;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															DlgAbout()
// --------------------------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK DlgAbout(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_WEBLINK:
			ShellExecute(hWndDlg, _T("open"), _T("http://www.idealsoftware.com"), NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDOK:
			EndDialog(hWndDlg, 1);
			return TRUE;
		}
		break;
	}

	return FALSE;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														TrayNotifyWndProc()
// receives notification messages from the tray icon
// --------------------------------------------------------------------------------------------------------------------------------------------
#define			WM_ICON_NOTIFY		(WM_APP + 10)
CSystemTray		gTrayIcon;
CSfmlApp		*gpNotifyApp;

LRESULT CALLBACK TrayNotifyWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ICON_NOTIFY:
		return gTrayIcon.OnTrayNotification(wParam, lParam);

	case WM_COMMAND:
		if (wParam == ID_MENU_SHOW)
			gpNotifyApp->ShowMainWindow();
		else if (wParam == ID_MENU_SUSPEND)
		{
			if (!gbSuspendHooks)
				gbSuspendHooks = true;
			else
				gbSuspendHooks = false;
			gTrayIcon.CheckMenuItem(ID_MENU_SUSPEND, gbSuspendHooks);
		}
		else if (wParam == ID_MENU_SETTINGS)
		{
			gpNotifyApp->SettingsDialog();
		}
		else if (wParam == ID_MENU_ABOUT)
			DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUT), gpNotifyApp->GetRenderWindow().getSystemHandle(), reinterpret_cast<DLGPROC>(DlgAbout));
		else if (wParam == ID_MENU_EXIT)
			gpNotifyApp->CloseMainWindow();
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														CreateTrayNotifyWindow()
//
// Native WINAPI window, that receives notification messages from the tray icon.
// --------------------------------------------------------------------------------------------------------------------------------------------
HWND CreateTrayNotifyWindow(CSfmlApp *app)
{
	WNDCLASS    wndclass;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = TrayNotifyWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = ghInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = APP_CLASS_NAME;
	RegisterClass(&wndclass);

	HWND hwnd = CreateWindow(APP_CLASS_NAME, APP_CLASS_NAME,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		app->GetRenderWindow().getSystemHandle(), NULL, ghInstance, NULL);

	gpNotifyApp = app;

	return hwnd;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														ThumbnailWndProc()
// --------------------------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK ThumbnailWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														CreateThumbnailWindow()
// --------------------------------------------------------------------------------------------------------------------------------------------
HWND CreateThumbnailWindow(CSfmlApp *app)
{
	WNDCLASS    wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ThumbnailWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = ghInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = NULL; // CreateSolidBrush(RGB(10, 59, 180));
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = THUMBNAIL_CLASS_NAME;
	RegisterClass(&wndclass);

	HWND hwnd = CreateWindow(THUMBNAIL_CLASS_NAME, THUMBNAIL_CLASS_NAME,
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		app->GetRenderWindow().getSystemHandle(), NULL, ghInstance, NULL);

	return hwnd;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//																CopyScreen()
// --------------------------------------------------------------------------------------------------------------------------------------------
DIBitmap CopyScreen(int x, int y, int width, int height)
{
	HDC dc = ::GetDC(NULL);		// get the desktop device context

								// create RGBA device context using a DIBSection
	BITMAPV5HEADER h;
	InitializeBitmapHeader(&h, width, height);
	DIBitmap bits;
	HBITMAP dib = ::CreateDIBSection(dc, reinterpret_cast<BITMAPINFO*>(&h),
		DIB_RGB_COLORS, reinterpret_cast<void**>(&bits), NULL, 0);
	HDC dib_dc = CreateCompatibleDC(dc);
	::SelectObject(dib_dc, dib);

	// copy from the desktop device context to the bitmap device context
	BitBlt(dib_dc, 0, 0, width, height, dc, x, y, SRCCOPY);

	// Copy the image over. It is in BGRA format, change it to RGBA
	size_t num_pixels = width * height;
	DIBitmap bitmap = new uint32[num_pixels];
	register BYTE *source = (BYTE *)bits;
	register BYTE *target = (BYTE *)bitmap;
	for (register size_t i = 0; i < num_pixels; i++)
	{
		*(target + 2) = *(source + 0);
		*(target + 1) = *(source + 1);
		*target = *(source + 2);
		*(target + 3) = 0xff;
		source += 4;
		target += 4;
	}

	::DeleteDC(dib_dc);
	::DeleteObject(dib);
	::ReleaseDC(NULL, dc);

	return bitmap;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//																GetPath()
// --------------------------------------------------------------------------------------------------------------------------------------------
RString GetPath(const RString &path)
{
	int len = (int)path.length() - 1;
	int i;
	for (i = len; i >= 0 && path[i] != _T('\\'); i--)
		;	// NOP

	if (i >= 0)
		return path.left(i);

	return _T("");
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//																GetLeadingPath()
// --------------------------------------------------------------------------------------------------------------------------------------------
RString GetLeadingPath(const RString &path)
{
	int start = 0;
	if (path.left(2) == _T("\\\\"))		// UNC path
		start = 2;

	int pos = path.find(_T('\\'), start);
	if (pos == -1)
		return _T("");	// path;

	return path.left(pos + 1);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														ExtractFileName()
// --------------------------------------------------------------------------------------------------------------------------------------------
RString ExtractFileName(const RString &path)
{
	int len = (int)path.length() - 1;
	int i;
	for (i = len; i >= 0 && path[i] != _T('\\'); i--)
		;	// NOP

	RString ret;
	if (i >= 0)
	{
		ret = path.substr(i + 1);
		len = (int)ret.length() - 1;
		for (i = len; i >= 0 && ret[i] != _T('.'); i--)
			;	// NOP
		if (i > 0)
			ret = ret.left(i);
	}

	return ret;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														class CWindowDesc
// Used when enumerating open windows
// --------------------------------------------------------------------------------------------------------------------------------------------
class CWindowDesc
{
public:
	HWND		m_Hwnd;
	RString		m_strProcess;
	RString		m_strTitle;
	CIcon		m_Icon;

public:
	CWindowDesc()
		:	m_Hwnd(NULL)
	{
	}

	CWindowDesc(HWND hWnd, const RString &process, const RString &title)
		:	m_Hwnd(hWnd),
			m_strProcess(process),
			m_Icon(process)
	{
		// if the window is an Explorer window, use the viewed path as title
		auto it = g_mapExplorerPaths.find(hWnd);
		if (it != g_mapExplorerPaths.end())
			m_strTitle = it->second;
		else
			m_strTitle = title;

		m_Icon.CreateBitmap(hWnd);
	}
};

typedef vector<CWindowDesc>	TWindowDescArray;


// --------------------------------------------------------------------------------------------------------------------------------------------
//															IsAltTabWindow()
// --------------------------------------------------------------------------------------------------------------------------------------------
BOOL IsAltTabWindow(HWND hwnd)
{
	if (!IsWindowVisible(hwnd))
		return FALSE;

	// skip tool-windows
	if (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;		// continue search

	if (GetParent(hwnd))
		return FALSE;

#if 0
	// see http://blogs.msdn.com/b/oldnewthing/archive/2007/10/08/5351207.aspx
	// and http://www.dfcd.net/projects/switcher/switcher.c
	TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;
	hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
	while (hwndTry != hwndWalk)
	{
		hwndWalk = hwndTry;
		hwndTry = GetLastActivePopup(hwndWalk);
		if (IsWindowVisible(hwndTry))
			break;
	}

	if (hwndWalk != hwnd)
		return FALSE;

	// the following removes some task tray programs and "Program Manager"
	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hwnd, &ti);
	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;
#endif

	return TRUE;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															GetProcessName()
// --------------------------------------------------------------------------------------------------------------------------------------------
RString GetProcessName(HWND hWnd)
{
	DWORD proc_id;
	TCHAR proc_name[1024];

	*proc_name = _T('\0');
	GetWindowThreadProcessId(hWnd, &proc_id);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, proc_id);

	// some processes can not be opened, e.g. Microsoft Spy++, so we leave the process name empty in this case
	if (hProcess)
	{
		GetModuleFileNameEx(hProcess, NULL, proc_name, _tcschars(proc_name));
		CloseHandle(hProcess);
	}

	RString s(proc_name);
	s.MakeLower();			// sometimes there is explorer.EXE and explorer.exe running at the same time (!)
	return s;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															WindowEnumCallback()
// --------------------------------------------------------------------------------------------------------------------------------------------
BOOL CALLBACK WindowEnumCallback(HWND hWnd, LPARAM lParam)
{
	TCHAR title[256];
	
	if (!IsAltTabWindow(hWnd))// || !IsIconic(hWnd))
		return TRUE;			// skip this window and continue

	GetWindowText(hWnd, title, _tcschars(title));

	if (_tcslen(title) > 0)
	{
		// skip our own window
		if (_tcscmp(title, APP_NAME) == 0)
			return TRUE;		// continue search

		RString proc_name = GetProcessName(hWnd);
		((TWindowDescArray *)lParam)->push_back(CWindowDesc(hWnd, proc_name, title));
	}

	return TRUE;		// continue search
}


void CreateWindowList(TWindowDescArray &wdesc)
{
	EnumWindows(WindowEnumCallback, (LPARAM)&wdesc);
}


// removes all entries from array, so that only the windows of the foreground process are left
void FilterForegroundProcess(HWND hwndForeground, TWindowDescArray &wdesc)
{
	RString proc_name = GetProcessName(hwndForeground);

	for (auto it = wdesc.begin(); it != wdesc.end();)
	{
		if (it->m_strProcess != proc_name)
			it = wdesc.erase(it);
		else
			it++;
	}
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														class CWinGroup
// Used to group windows
// --------------------------------------------------------------------------------------------------------------------------------------------
class CWinGroup
{
public:
	CWindowDesc			m_GroupHeader;
	vector<CWinGroup>	m_Group;
	bool				m_bVisible;
	int					m_nSortOrder;			// for sort order, otherwise "e:\" is sorted behind "e:\download"
	bool				m_bMarkForDelete;		// for internal house-keeping

public:
	CWinGroup(const CWindowDesc &desc)
		:	m_GroupHeader(desc),
			m_bVisible(true),
			m_nSortOrder(1),
			m_bMarkForDelete(false)
	{
	}
	
	void BuildGroups(RString path);

	bool operator < (const CWinGroup &rhs) const
	{
		int cmp = 0;
		RString p1, p2;

		if (m_nSortOrder < rhs.m_nSortOrder)
			return true;
		if (m_nSortOrder > rhs.m_nSortOrder)
			return false;

		p1 = ExtractFileName(m_GroupHeader.m_strProcess);
		p2 = ExtractFileName(rhs.m_GroupHeader.m_strProcess);
		cmp = p1.CompareNoCase(p2);
		if (cmp < 0)
			return true;

		if (cmp == 0 && m_GroupHeader.m_strTitle.CompareNoCase(rhs.m_GroupHeader.m_strTitle) < 0)
			return true;

		return false;
	}
};

typedef vector<CWinGroup>	TWinGroupArray;


// --------------------------------------------------------------------------------------------------------------------------------------------
//														CWinGroup::BuildGroups()
// identische sub-pfade innerhalb der Gruppen ordnen
// --------------------------------------------------------------------------------------------------------------------------------------------
void CWinGroup::BuildGroups(RString path)
{
	if (m_Group.size() <= 1)
		return;

	int count_first_order = 0;

	size_t path_len = path.length();
	if (path_len > 0)
	{
		// remove common leading path for whole group
#if 0
		for (auto &&it : m_Group)
		{
			if (path == it.m_GroupHeader.m_strTitle.left(path_len))
			{
				if (it.m_GroupHeader.m_strTitle.substr(path_len).empty())
				{
					sort(m_Group.begin(), m_Group.end());
					return;
				}
			}
		}
#endif
		for (auto &&it : m_Group)
		{
			if (path == it.m_GroupHeader.m_strTitle.left(path_len))
			{
				if (it.m_GroupHeader.m_strTitle.length() > path_len)
					it.m_GroupHeader.m_strTitle = it.m_GroupHeader.m_strTitle.substr(path_len);
				else
				{
					// special case: the path is "e:\" and an entry is also "e:\", then we will
					// not eliminate the path, but put it in front of the sort order
					it.m_nSortOrder = 0;
					count_first_order++;
				}
			}
		}
	}

	// all entries are of same sort order, i.e. they are identical, for example "c:\" and "c:\"
	if (count_first_order == m_Group.size())
		return;

	// group identical paths together
	for (size_t i = 0; i < m_Group.size() - 1; i++)
	{
		if (m_Group[i].m_bMarkForDelete || m_Group[i].m_nSortOrder == 0)
			continue;

		RString lead1 = GetLeadingPath(m_Group[i].m_GroupHeader.m_strTitle);
		if (lead1.empty())
			continue;

		for (size_t n = i + 1; n < m_Group.size(); n++)
		{
			if (m_Group[n].m_bMarkForDelete)
				continue;

			RString lead2 = GetLeadingPath(m_Group[n].m_GroupHeader.m_strTitle);
			if (lead1 == lead2)
			{
				// we have an identical leading path
				if (m_Group[i].m_Group.empty())
					m_Group[i].m_Group.push_back(CWinGroup(m_Group[i].m_GroupHeader));
				m_Group[i].m_Group.push_back(m_Group[n]);
				m_Group[n].m_bMarkForDelete = true;

				// the title of the group header holds the leading path
				m_Group[i].m_GroupHeader.m_strTitle = lead1;
			}
		}
	}

	// remove entries marked for deletion
	for (auto it = m_Group.begin(); it != m_Group.end();)
	{
		if (it->m_bMarkForDelete)
			it = m_Group.erase(it);
		else
			it++;
	}

	// move one level deeper
	for (auto &&it : m_Group)
		it.BuildGroups(it.m_GroupHeader.m_strTitle);

	// if this node finally has only a single group, pull-up the group's subgroups and delete the group
	// eg:	d:\
	//			recording\
	//				cubase
	//				samples
	// becomes:
	//		d:\recording\
	//			cubase
	//			samples
	if (m_Group.size() == 1)
	{
		// pull-up title
		m_GroupHeader.m_strTitle += m_Group[0].m_GroupHeader.m_strTitle;

		// we must copy the group here first, because using "m_Group[0].m_Group" directly in the for-loop below
		// leads to runtime-error "vector iterator not incrementable". Maybe, because adding nodes to m_Group
		// re-allocates m_Group, and therefore m_Group[0].m_Group is no longer valid?
		TWinGroupArray group = m_Group[0].m_Group;
		for (auto &&it : group)
			m_Group.push_back(it);
		m_Group.erase(m_Group.begin());
	}

	// sortieren
	sort(m_Group.begin(), m_Group.end());

	// wenn ein sub-pfad und ein Gruppenname identisch sind, dann den Gruppennamen verbergen
	// eg:	e:\
	//			Recording		<== is an open folder
	//			Recording\		<== is a group title that needs to be hidden
	//				Mixdowns	<== is an open folder
	//				StudioOne	<== is an open folder
	for (auto &&it_group : m_Group)
	{
		if (!it_group.m_Group.empty())
		{
			// this is a group with subgroups
			// get the title and remove the "\" at the end
			RString group_title = it_group.m_GroupHeader.m_strTitle.left(it_group.m_GroupHeader.m_strTitle.length() - 1);

			// compare with any other entries within this group-level, which do not have subgroups, i.e. they are final nodes
			for (auto &&it : m_Group)
			{
				if (it.m_Group.empty() && group_title == it.m_GroupHeader.m_strTitle)
				{
					it_group.m_bVisible = false;
					break;
				}
			}
		}
	}
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															GroupWindows()
// creates grouped entries in m_WinGroups
// --------------------------------------------------------------------------------------------------------------------------------------------
void GroupWindows(const TWindowDescArray &WinDesc, TWinGroupArray &WinGroups)
{
	// first, group identical processes
	for (auto &&it : WinDesc)
	{
		RString process = it.m_strProcess;
		bool found = false;
		for (auto &&it_group : WinGroups)
		{
			if (it_group.m_GroupHeader.m_strProcess == process)
			{
				if (it_group.m_Group.empty())
					it_group.m_Group.push_back(CWinGroup(it_group.m_GroupHeader));
				it_group.m_Group.push_back(CWinGroup(it));
				found = true;
				break;
			}
		}

		if (!found)
			WinGroups.push_back(CWinGroup(it));
	}

	// jetzt identische sub-pfade innerhalb der Gruppen ordnen
	for (auto &&it_group : WinGroups)
		it_group.BuildGroups(_T(""));

	// sortieren
	sort(WinGroups.begin(), WinGroups.end());
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														class CLinearWin
//
// Used to transform CWinGroup to a linear form for easier drawing and keyboard navigation.
// --------------------------------------------------------------------------------------------------------------------------------------------
class CLinearWin
{
public:
	CWindowDesc		m_WinDesc;
	int				m_nLevel;				// indent-level
	bool			m_bIsTopLevelHeader;	// top-level header
	bool			m_bIsHeader;			// non-clickable sub-level header
	bool			m_bIsClickable;			// normal entries are clickable and top-level headers are clickable if group is empty
	RString			m_strTitle;				// display title
	int				m_nLineCount;			// number of lines for this group, including all sub-levels
	int				m_nYTop;				// top pixel position of line, initialized in Draw()
	int				m_nYBottom;				// bottom pixel position of line, initialized in Draw()

public:
	CLinearWin(const CWindowDesc &desc, int level)
		:	m_WinDesc(desc),
			m_nLevel(level),
			m_bIsTopLevelHeader(false),
			m_bIsHeader(false),
			m_bIsClickable(false),
			m_nLineCount(0),
			m_nYTop(0),
			m_nYBottom(0)
	{
	}
};

typedef vector<CLinearWin>	TLinearWinArray;


void AddGroup(TLinearWinArray &arWindows, const CWinGroup &group, int level)
{
	if (group.m_bVisible)
	{
		CLinearWin win(group.m_GroupHeader, level);

		if (level == 0)
			win.m_bIsTopLevelHeader = true;		// group top-level header
		else if (group.m_Group.size() > 1)
			win.m_bIsHeader = true;		// group sub-level header

		// draw title or process name
		if (level == 0 && group.m_Group.size() > 1)
		{
			win.m_strTitle = ExtractFileName(group.m_GroupHeader.m_strProcess);
			if (!win.m_strTitle.empty())
			{
				// upper-case first letter of process name (_toupper() for first character only does not always work)
				RString tmp = win.m_strTitle;
				tmp.MakeUpper();
				win.m_strTitle = tmp.Left(1) + win.m_strTitle.substr(1);
			}
		}
		else
			win.m_strTitle = group.m_GroupHeader.m_strTitle;

		if (!win.m_bIsHeader && group.m_Group.size() == 0)
			win.m_bIsClickable = true;

		arWindows.push_back(win);
		//CLinearWin &last = arWindows.back();
	}

	for (auto &&it : group.m_Group)
		AddGroup(arWindows, it, level + 1);
}


// convert groups to linear array
void MakeLinear(TWinGroupArray &WinGroups, TLinearWinArray &arWindows)
{
	arWindows.clear();
	for (auto &&it : WinGroups)
		AddGroup(arWindows, it, 0);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//																class CMyApp
// --------------------------------------------------------------------------------------------------------------------------------------------
class CMyApp : public CSfmlApp
{
protected:
	int				m_nDesktopWidth;
	int				m_nDesktopHeight;
	RString			m_strSettingsPath;
	int				m_nListLeft;			// x-pos of list view
	int				m_nListTop;				// y-pos of list view
	int				m_nListWidth;			// width of list in pixels
	sf::View		m_View;					// scroll view
	sf::View		m_InitialView;
	DIBitmap		m_BackBitmap;			// screen background bitmap, which is used for alpha-blending
	sf::Texture		m_BackTexture;
	sf::Sprite		m_BackSprite;

	int				m_nMouseX;
	int				m_nMouseY;

	bool			m_bFilteredView;		// true, if filtered view
	TLinearWinArray	m_Windows;
	CSfmlMemoryFont	*m_pFont;
	sf::Text		*m_pText;

	int				m_nTopLine;				// the line number, which is the topmost visible line
	int				m_nCursorLine;			// the line number, which is highlighted

	HWND			m_hwndForeground;		// the foreground window before this app is activated
	HWND			m_hwndThumbTarget;		// target thumbnail window
	bool			m_bThumbWindowVisible;	// if thumbnail creation failed, window is invisible

	HTHUMBNAIL		m_hThumbnailID;			// thumbnail ID

	int				m_nDoubleClickTime;
	__int64			m_nPerformanceFreq;
	__int64			m_nDblClickStartTime;
	bool			m_bDoubleClick;

	bool			m_bStartup;				// true, if the main window is created at startup of app
	bool			m_bFirstTime;			// true, if this app is run the very first time

	RString			m_strLicense;			// owner of license

	CTaskbar		m_Taskbar;
	int				m_nLeft;				// left position of app window
	int				m_nTop;					// top position of app window
	int				m_nWindowWidth;
	int				m_nWindowHeight;
	int				m_nFontSize;
	int				m_nLineSpacing;
	bool			m_bChangeWindowLayout;
	sf::Clock		m_Timer;

public:
	CMyApp()
		:	CSfmlApp(),
			m_BackBitmap(nullptr),
			m_nMouseX(0),
			m_nMouseY(0),
			m_bFilteredView(false),
			m_pFont(nullptr),
			m_pText(nullptr),
			m_nTopLine(0),
			m_nCursorLine(0),
			m_hwndThumbTarget(NULL),
			m_bThumbWindowVisible(false),
			m_hThumbnailID(NULL),
			m_bStartup(true),
			m_bFirstTime(false),
			m_nFontSize(14)
	{
#if 0
		//m_strLicense = _T("Registered to: Heinz Hartstein • Heinz-Hartstein@Arcor.de • Erlenweg  55 • 47877 Willich - Neersen • Deutschland");
		m_strLicense = _T("Registered to: Ralph Jansen • ralph.jansen@gmx.ch • Gerberweg 57 • 2560 Nidau • Schweiz");
#endif

		m_enWindowStyle = enWsWindowed;
		gbHotkeyVKey = 0;		// virtual key code for app activation
		gbHotkeyModifiers = 0;	// modifiers for app activation (shift, ctrl, alt)

		m_nDoubleClickTime = GetDoubleClickTime();
		QueryPerformanceFrequency((LARGE_INTEGER *)&m_nPerformanceFreq);
		m_nPerformanceFreq /= 1000;	// we want milliseconds later
		m_nDblClickStartTime = 0;
		m_bDoubleClick = false;

		RunHookThread();

		HICON hTrayIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_APP_ICON));
		gTrayIcon.Create(ghInstance, NULL, WM_ICON_NOTIFY, APP_NAME, hTrayIcon, IDR_TRAY_MENU);

		// check, if path exists. otherwise create it.
		m_strSettingsPath = gstrAppData;
		if (!TestWriteAccess(m_strSettingsPath))
			_tmkdir(m_strSettingsPath.c_str());

		m_strSettingsPath += SETTINGS_FILE;

		bool have_settings = TestWriteAccess(m_strSettingsPath);

#if 1
		if (have_settings)
		{
			try
			{
				//SetCursor(LoadCursor(ghInstance, IDC_WAIT));
				gSettings.ReadFromFile(m_strSettingsPath);
			}
			catch (const Exception &e)
			{
				MessageBox(NULL, e.GetExceptionMessage().c_str(), APP_NAME, MB_ICONERROR);
			}
		}
		else
		{
			// show settings dialog
			MessageBox(NULL,
				_T("This seems to be the first time you are running \"") APP_NAME _T("\".\n\n")
				//_T("For a tutorial video, please search in Youtube for:\n")
				//_T("\"") APP_NAME _T("\"\n\n")
				_T("To configure activation keys and the background color,\n")
				_T("please right-click onto the icon in the task bar and\n")
				_T("choose \"Settings\".")
				, APP_NAME, MB_ICONINFORMATION);

			m_bFirstTime = true;
		}
#endif
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::~CMyApp()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	~CMyApp()
	{
		// ReleaseHooks();
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::DesktopResolutionChanged()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	bool DesktopResolutionChanged()
	{
		sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
		return m_nDesktopWidth != desktop.width || m_nDesktopHeight != desktop.height;
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//														CMyApp::ComputeLayout()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void FinalizeLayout()
	{
		if (m_Window.getSystemHandle())
		{

			// position window
			MoveWindow(m_Window.getSystemHandle(), m_nLeft, m_nTop, m_nWindowWidth, m_nWindowHeight, true);

			float w = m_nWindowWidth * 0.275f;
			float h = w;	// *3 / 4;
			MoveWindow(m_hwndThumbTarget, m_nLeft + 20, m_nTop + 20, (int)w, (int)h, TRUE);
		}

		// setup initial view, used for drawing the background and other static parts (page indicators, activity indicators)
		float w = (float)m_nWindowWidth;	// m_Window.getSize().x;
		float h = (float)m_nWindowHeight;	// m_Window.getSize().y;
		m_InitialView.setSize(w, h);
		m_InitialView.setCenter(w / 2.f, h / 2.f);
		m_Window.setView(m_InitialView);

		// setup View
		float win_w = (float)m_nWindowWidth;
		float win_h = (float)m_nWindowHeight;
		w = win_w * .68f;
		h = win_h * .94f;
		m_View.setSize(w, h);
		m_View.setCenter(w / 2.f, h / 2.f);
		m_View.setViewport(sf::FloatRect(0.3f, 0.025f, .68f, 0.94f));

		m_nListLeft = (int)(win_w * 0.3f);
		m_nListTop = (int)(win_h * 0.025f);
		m_nListWidth = (int)w; // win_w - m_nListLeft;

		m_nTopLine = 0;
	}


	void ComputeLayout()
	{
		// setup global config
		sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

		m_nDesktopWidth = desktop.width;
		m_nDesktopHeight = desktop.height;

		m_nWindowWidth = desktop.width;
		m_nWindowHeight = desktop.height;
		m_nTop = 0;
		m_nLeft = 0;

		// make window fit in conjuction with taskbar
		// if taskbar is not covering full height, it is docked at top or bottom of screen
		// NOTE: if the taskbar is hidden, the height of the bar is still as if unhidden, so we can not 
		//       use the width or height members of the CTaskbar class.
		if (m_nWindowHeight != m_Taskbar.m_nHeight)
		{
			// taskbar is top or bottom attached
			if (m_Taskbar.m_rcPosition.top <= 0)
			{
				m_nTop = m_Taskbar.m_rcPosition.bottom;
				m_nWindowHeight -= m_Taskbar.m_rcPosition.bottom;
			}
			else
				m_nWindowHeight -= m_nWindowHeight - m_Taskbar.m_rcPosition.top;
		}

		// if taskbar is not covering full width, it is docked at left or right of screen
		if (m_nWindowWidth != m_Taskbar.m_nWidth)
		{
			// taskbar is left or right attached
			if (m_Taskbar.m_rcPosition.left <= 0)
			{
				m_nLeft = m_Taskbar.m_rcPosition.right;
				m_nWindowWidth -= m_Taskbar.m_rcPosition.right;
			}
			else
				m_nWindowWidth -= m_nWindowWidth - m_Taskbar.m_rcPosition.left;
		}

		m_nWindowWidth = (int)((float)m_nWindowWidth * 0.9f);
		m_nWindowHeight = (int)((float)m_nWindowHeight * 0.9f);
		m_nLeft += (int)((float)m_nWindowWidth * 0.05f);
		m_nTop += (int)((float)m_nWindowHeight * 0.05f);

		if (!m_pFont)
		{
			m_pFont = new CSfmlMemoryFont();
			m_pFont->Create(m_Window, _T("Segoe UI"));		// will fallback to Arial, if not present
		}

		if (!m_pText)
		{
			// Create a sf::Text object which uses our font
			m_pText = new sf::Text();
			m_pText->setFont(m_pFont->GetSfmlFont());
			m_pText->setColor(sf::Color::White);
			m_pText->setStyle(sf::Text::Regular);		// sf::Text::Bold         sf::Text::Regular
		}

		// use desktop.height to scale font size
		double factor = sqrt((double)desktop.height / 1080.0);
		m_nFontSize = (int)(14.0 * factor);
		m_pText->setCharacterSize(m_nFontSize);
		m_nLineSpacing = (int)(m_pText->getFont()->getLineSpacing((int)(m_pText->getCharacterSize()))) - 1;	// -1 looks better

		FinalizeLayout();
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::CreateMainWindow()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void CreateMainWindow() override
	{
		// Compute Layout, Create Font
		ComputeLayout();

		// setup main window
		unsigned int style = sf::Style::None;
		sf::ContextSettings context;
		context.antialiasingLevel = 4;
		m_Window.create(sf::VideoMode(m_nWindowWidth, m_nWindowHeight), APP_NAME, style, context);
		m_Window.setIcon(app_icon.width, app_icon.height, app_icon.pixel_data); 

		HWND hWnd = m_Window.getSystemHandle();

		// install hooks
		SetHookAppWindow(hWnd);
		SetHookInstance(ghInstance);

		if (m_bStartup && !m_bFirstTime)
		{
//			ShowWindow(hWnd, SW_HIDE);
			m_bFirstTime = false;
		}

		/* does not work, because we have a pseudo-rounded window (by using a bitmap)
		   and using real rounded corners does not work with OpenGL
		// use DWM to create a drop-shadow, even if our window has no window style
		DWORD attr = DWMNCRP_ENABLED;
		DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &attr, sizeof(attr));
		MARGINS margins = { 25, 25, 25, 25 };
		DwmExtendFrameIntoClientArea(hWnd, &margins); */

#if 0
		SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
//		SetLayeredWindowAttributes(hWnd, 0, 210, LWA_ALPHA);		// LWA_COLORKEY seems not to work for OpenGL Windows
//		SetLayeredWindowAttributes(hWnd, m_BkgColor.toInteger(), 210, LWA_COLORKEY);
		SetLayeredWindowAttributes(hWnd, 0, 0, LWA_COLORKEY);
#endif

		/*
		HRGN rgn = CreateRoundRectRgn(	0, 0,
										width, height,
										20, 20);
		 SetWindowRgn(hWnd, rgn, TRUE);
		 */

		// Send all menu messages to MainWindow
		gTrayIcon.SetTargetWnd(CreateTrayNotifyWindow(this));

		// Compute Layout, Create Font
		//m_bDrawActivityOnly = true;

		// HINT: removing the call to FramerateLimit, enabling vsync, adjusting the timing using Sleep() and setting 
		// "Threaded Optimization = Off" in the nVidia driver panel finally did the trick.
		// (whatever Threaded Optimization means... found it using google)
		//	m_Window.setFramerateLimit(60);
//m_Window.setVerticalSyncEnabled(true);
m_Window.setFramerateLimit(0);
m_Window.setVerticalSyncEnabled(true);

		m_bStartup = false;
		//CheckNagScreen();		==> done in ShowMainWindow()

		m_hwndThumbTarget = CreateThumbnailWindow(this);

		FinalizeLayout();

		//ShowMainWindow();		// for debugging
		ShowWindow(m_Window.getSystemHandle(), SW_HIDE);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::CheckNagScreen()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void CheckNagScreen()
	{
#if 0
		SYSTEMTIME time;
		
		GetSystemTime(&time);

		// don't make it too easy, scramble a bit (so year constant can not be found in hex editor)
		time.wYear -= 2000;

		// run until 2019/01/01
		if (time.wYear >= 19 && time.wMonth >= 1)
		{
			MessageBox(m_Window.getSystemHandle(),
				_T("The beta period for \"" APP_NAME "\" has expired.\n\n")
				_T("Please check at www.tenware.net, if a newer version is available.\n\n"),
				APP_NAME, MB_ICONERROR);
			exit(1);
		}
#endif
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::CreateSnapshot()
	//
	// filter = true: removes all entries from array, so that only the windows of the foreground process are left
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void CreateSnapshot(bool filter)
	{
		GetAllExplorerPaths(g_mapExplorerPaths);

		TWindowDescArray WinDescs;
		CreateWindowList(WinDescs);

		if (filter)
			FilterForegroundProcess(m_hwndForeground, WinDescs);

		TWinGroupArray WinGroups;
		GroupWindows(WinDescs, WinGroups);
		MakeLinear(WinGroups, m_Windows);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::ShowMainWindow()
	//
	// filter = true: removes all entries from array, so that only the windows of the foreground process are left
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void ShowMainWindow() override
	{
		ShowMainWindow(false);
	}

	void ShowMainWindow(bool filter)
	{
		// due to a weird bug with 4K, where the resolution seems to be reported wrongly sometimes,
		// we compute the layout always
		//if (m_Taskbar.ChangedPosition())
		{
			// the taskbar changed its position, we need to re-layout
			ComputeLayout();
		}

		m_bFilteredView = filter;

		/*if (IsWindowVisible(m_Window.getSystemHandle()))
		{
			SetFocus(m_Window.getSystemHandle());
			return;
		}*/

		if (!IsWindowVisible(m_Window.getSystemHandle()))
		{
			m_hwndForeground = GetForegroundWindow();
#if 1
			if (m_BackBitmap)
				delete[] m_BackBitmap;
			m_BackBitmap = CopyScreen(m_nLeft, m_nTop, m_nWindowWidth, m_nWindowHeight);
#endif
			m_BackTexture.create(m_nWindowWidth, m_nWindowHeight);
			m_BackTexture.update((sf::Uint8 *)m_BackBitmap);
			m_BackTexture.setSmooth(true);

			m_BackSprite.setTexture(m_BackTexture, true);
			m_BackSprite.setColor(sf::Color(255, 255, 255, 255));
		}

		CreateSnapshot(filter);

		// select the foreground window
		int i = 0;
		bool found = false;
		for (auto &&it : m_Windows)
		{
			if (it.m_bIsClickable && it.m_WinDesc.m_Hwnd == m_hwndForeground)
			{
				found = true;
				break;
			}

			i++;
		}

		// reset view to initial 0, 0 position:
		m_View.setCenter(m_View.getSize().x / 2, m_View.getSize().y / 2);
		m_nTopLine = 0;

		if (!found)
		{
			// select first clickable line
			i = -1;
		}

		MakeCursorVisible(i, true);

		if (!IsWindowVisible(m_Window.getSystemHandle()))
		{
			// show window
			if (m_bThumbWindowVisible)
				ShowWindow(m_hwndThumbTarget, SW_SHOW);
			ShowWindow(m_Window.getSystemHandle(), SW_SHOW);
			SetFocus(m_Window.getSystemHandle());
			SetForegroundWindowInternal(m_Window.getSystemHandle());
			gbEatSingleClick = false;
			CheckNagScreen();
		}
	}
	

	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::OnLostFocus()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void OnLostFocus() override
	{
		ShowWindow(m_hwndThumbTarget, SW_HIDE);
		ShowWindow(m_Window.getSystemHandle(), SW_HIDE);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::SaveSettings()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void SaveSettings()
	{
		try
		{
			gSettings.WriteToFile(m_strSettingsPath);
		}
		catch (const Exception &e)
		{
			MessageBox(m_Window.getSystemHandle(), e.GetExceptionMessage().c_str(), APP_NAME, MB_ICONERROR);
		}
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::SettingsDialog()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void SettingsDialog()
	{
		CSettings old(gSettings);
		if (DialogBox(ghInstance, MAKEINTRESOURCE(IDD_SETTINGS), m_Window.getSystemHandle(), reinterpret_cast<DLGPROC>(DlgSettings)) != 1)
			return;

		SaveSettings();
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::OnClose()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	bool OnClose() override
	{
		//SaveSettings();		// save always, so current page number is saved
		return true;
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::ControlKeyPressed()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	bool ControlKeyPressed() const
	{
		return sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::IsArrowKey()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	bool IsArrowKey(sf::Keyboard::Key code) const
	{
		return code == sf::Keyboard::Left || code == sf::Keyboard::Right || code == sf::Keyboard::Up || code == sf::Keyboard::Down;
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::OnMouseDown()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void OnMouseDown(const sf::Event &event) override
	{
		int mouse_x = event.mouseButton.x;	// +m_IconManager.GetCurrentPageNum() * gConfig.m_nWindowWidth;
		int mouse_y = event.mouseButton.y;

		if (event.mouseButton.button == sf::Mouse::Left)
		{
			if (m_nDblClickStartTime == 0)
				QueryPerformanceCounter((LARGE_INTEGER *)&m_nDblClickStartTime);
			else
			{
				__int64 now, elapsed;
				QueryPerformanceCounter((LARGE_INTEGER *)&now);
				elapsed = (now - m_nDblClickStartTime) / m_nPerformanceFreq;
				if (elapsed <= m_nDoubleClickTime)
				{
					// we have a double-click here
					m_bDoubleClick = true;
					m_nDblClickStartTime = 0;
				}
				else
					QueryPerformanceCounter((LARGE_INTEGER *)&m_nDblClickStartTime);
			}
		}
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::OnMouseUp()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void OnMouseUp(const sf::Event &event) override
	{
		int mouse_x = event.mouseButton.x;
		int mouse_y = event.mouseButton.y;

		mouse_y -= m_nListTop;						// subtract list-view top
		mouse_y += m_Windows[m_nTopLine].m_nYTop;	// add scroll position

		if (event.mouseButton.button == sf::Mouse::Left)
		{
			bool hit_line = false;

			if (mouse_x >= m_nListLeft)
			{
				// find clicked line
				size_t i = 0;
				for (auto &&it : m_Windows)
				{
					// compute mouse-hover bottom coordinate of current line
					int bottom = it.m_nYBottom + 1;
					if (i + 1 < m_Windows.size())
						bottom = m_Windows[i + 1].m_nYTop;

					if (mouse_y >= it.m_nYTop && mouse_y < bottom)
					{
						// skip non-clickable lines
						while (i < m_Windows.size() && !m_Windows[i].m_bIsClickable)
							i++;

						if (i < m_Windows.size())
						{
							hit_line = true;
							m_nCursorLine = (int)i;
							if (m_bDoubleClick)
							{
								ShowWindow(m_Window.getSystemHandle(), SW_HIDE);
								ShowWindow(m_hwndThumbTarget, SW_HIDE);
								SetForegroundWindowInternal(m_Windows[i].m_WinDesc.m_Hwnd);
							}
							else
								RegisterThumbnail(m_Windows[i].m_WinDesc.m_Hwnd);
						}
						m_bDoubleClick = false;
						break;
					}
					i++;
				}
			}

			if (!hit_line)
			{
				ShowWindow(m_hwndThumbTarget, SW_HIDE);
				ShowWindow(m_Window.getSystemHandle(), SW_HIDE);
			}
		}
		else if (event.mouseButton.button == sf::Mouse::Right)
		{
		}
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::OnMouseMove()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void OnMouseMove(const sf::Event &event) override
	{
		m_nMouseX = event.mouseMove.x;	// +m_IconManager.GetCurrentPageNum() * gConfig.m_nWindowWidth;
		m_nMouseY = event.mouseMove.y;
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::OnMouseWheel()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void OnMouseWheel(const sf::Event &event) override
	{
		if (event.mouseWheel.delta < 0)
			ScrollDown();
		else
			ScrollUp();
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::ScrollUp()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void ScrollUp()
	{
		if (m_nTopLine > 0)
		{
			int cur_y = m_Windows[m_nTopLine].m_nYTop;
			m_nTopLine--;
			m_View.move(0, (float)m_Windows[m_nTopLine].m_nYTop - cur_y);
		}
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::ScrollDown()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void ScrollDown()
	{
		int cur_y = m_Windows[m_nTopLine].m_nYTop;
		int bottom = m_Windows.back().m_nYBottom;
		if (bottom - cur_y > (int)m_View.getSize().y)
		{
			m_nTopLine++;
			m_View.move(0, (float)m_Windows[m_nTopLine].m_nYTop - cur_y);
		}
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::MakeCursorVisible()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void MakeCursorVisible(int line, bool compute_lines = false)
	{
		if (line >= (int)m_Windows.size())
			return;

		// -1 = select first clickable line
		if (line == -1)
		{
			line = 0;
			for (auto &&it : m_Windows)
			{
				if (it.m_bIsClickable)
					break;

				line++;
			}
		}
		else if (line == -2)	// -2 = last entry
			line = (int)m_Windows.size() - 1;

		if (compute_lines)
			Draw();
		
		m_nCursorLine = line;
		int cursor_top = m_Windows[m_nCursorLine].m_nYTop;
		int cursor_bottom = m_Windows[m_nCursorLine].m_nYBottom;

		int view_top = m_Windows[m_nTopLine].m_nYTop;
		while (cursor_top < view_top)
		{
			ScrollUp();		// m_View.move(0, (float)(cursor_top - view_top));
			view_top = m_Windows[m_nTopLine].m_nYTop;
		}

		int view_bottom = view_top + (int)m_View.getSize().y;
		while (cursor_bottom > view_bottom)
		{
			ScrollDown();	// m_View.move(0, (float)(cursor_bottom - view_bottom));
			view_top = m_Windows[m_nTopLine].m_nYTop;
			view_bottom = view_top + (int)m_View.getSize().y;
		}

		RegisterThumbnail(m_Windows[m_nCursorLine].m_WinDesc.m_Hwnd);
	}


	bool OnlyPrevHeaders()
	{
		int line = m_nCursorLine;
		while (line > 0)
		{
			line--;
			if (m_Windows[line].m_bIsClickable)
				return false;
		}

		while (m_nTopLine > 0)
			ScrollUp();

		return true;
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::MoveCursorUp()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void MoveCursorUp()
	{
		if (OnlyPrevHeaders())
			return;

		int line = m_nCursorLine - 1;
		while (line > 0 && !m_Windows[line].m_bIsClickable)
			line--;

		if (line >= 0 && m_Windows[line].m_bIsClickable)
			MakeCursorVisible(line);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::MoveCursorDown()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void MoveCursorDown()
	{
		int line = m_nCursorLine + 1;
		while (line < (int)m_Windows.size() - 1 && !m_Windows[line].m_bIsClickable)
			line++;

		if (line < (int)m_Windows.size() && m_Windows[line].m_bIsClickable)
			MakeCursorVisible(line);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::MoveCursorPrevGroup()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void MoveCursorPrevGroup()
	{
		if (OnlyPrevHeaders())
			return;

		int line = m_nCursorLine - 1;
		while (line > 0 && !m_Windows[line].m_bIsClickable)
			line--;

		while (line > 0 && m_Windows[line].m_bIsClickable && m_Windows[line].m_nLevel != 0)
			line--;

		if (m_Windows[line].m_nLevel != 0 || !m_Windows[line].m_bIsClickable)
		{
			while (line < (int)m_Windows.size() - 1 && !m_Windows[line].m_bIsClickable)
				line++;
		}

		if (line >= 0 && line < (int)m_Windows.size() && m_Windows[line].m_bIsClickable)
			MakeCursorVisible(line);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::MoveCursorNextGroup()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void MoveCursorNextGroup()
	{
		int line = m_nCursorLine + 1;
		while (line < (int)m_Windows.size() - 1 && m_Windows[line].m_bIsClickable && m_Windows[line].m_nLevel != 0)
			line++;

		while (line < (int)m_Windows.size() - 1 && !m_Windows[line].m_bIsClickable)
			line++;

		if (line < (int)m_Windows.size() && m_Windows[line].m_bIsClickable)
			MakeCursorVisible(line);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::OnKeyPressed()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void OnKeyPressed(const sf::Event &event) override
	{
		if (event.key.code == sf::Keyboard::Escape)
		{
			// hide window
			ShowWindow(m_hwndThumbTarget, SW_HIDE);
			ShowWindow(m_Window.getSystemHandle(), SW_HIDE);
		}
		else if (event.key.code == sf::Keyboard::Delete)
		{
		}
		else if (event.key.code == sf::Keyboard::Menu)
		{
		}
		else if (event.key.code == sf::Keyboard::Home)
		{
			MakeCursorVisible(-1);
		}
		else if (event.key.code == sf::Keyboard::End)
		{
			MakeCursorVisible(-2);
		}
		else if (event.key.code == sf::Keyboard::PageUp)
		{
		}
		else if (event.key.code == sf::Keyboard::PageDown)
		{
		}
		else if (IsArrowKey(event.key.code))
		{
			if (event.key.code == sf::Keyboard::Left)
				MoveCursorPrevGroup();
			else if (event.key.code == sf::Keyboard::Right)
				MoveCursorNextGroup();
			else if (event.key.code == sf::Keyboard::Up)
				MoveCursorUp();
			else if (event.key.code == sf::Keyboard::Down)
				MoveCursorDown();
		}
		else if (event.key.code == sf::Keyboard::Return)
		{
			ShowWindow(m_Window.getSystemHandle(), SW_HIDE);
			ShowWindow(m_hwndThumbTarget, SW_HIDE);
			SetForegroundWindowInternal(m_Windows[m_nCursorLine].m_WinDesc.m_Hwnd);
		}
		else if (event.key.control)	// ctrl-key is down
		{
		}
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::Work()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void Work() override
	{
		if (gbHookDoubleClick || gbHookEvent)
		{
			HWND fg_win = GetForegroundWindow();

			if (gSettings.m_bDisableIfMaxWin && fg_win != m_Window.getSystemHandle() && IsFullscreenAndMaximized(fg_win))
			{
				gbHookDoubleClick = false;
				gbHookEvent = false;
				return;
			}

			if (gbHookDoubleClick)
			{
				// a double-click will create a filtered view, where only
				// windows of the current foreground process are displayed
				gbHookDoubleClick = false;
				gbHookEvent = false;
				ShowMainWindow(true);
				m_bFilteredView = false;
			}
			else if (gbHookEvent)
			{
				// we have any of the user-defined activation actions here, which was caught by our hook
				gbHookEvent = false;
				if (fg_win != m_Window.getSystemHandle())
				{
					// show
					ShowMainWindow(gbFilterKey);
				}
				else
				{
					if (gbFilterKey != m_bFilteredView)
					{
						// switch from filtered view to unfiltered view or vice versa
						ShowMainWindow(gbFilterKey);
					}
					else
					{
						// hide
						ShowWindow(m_hwndThumbTarget, SW_HIDE);
						ShowWindow(m_Window.getSystemHandle(), SW_HIDE);
					}
				}
			}
		}

		if (DesktopResolutionChanged())
			ComputeLayout();
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//														CMyApp::RegisterThumbnail()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void RegisterThumbnail(HWND hwndSource)
	{
//return;
		HRESULT hr;

		// Register the thumbnail
		if (m_hThumbnailID)
			DwmUnregisterThumbnail(m_hThumbnailID);

		hr = DwmRegisterThumbnail(m_hwndThumbTarget, hwndSource, &m_hThumbnailID);

		if (SUCCEEDED(hr))
		{
			// Specify the destination rectangle size
			RECT dest;
			GetClientRect(m_hwndThumbTarget, &dest);

			// Set the thumbnail properties for use
			DWM_THUMBNAIL_PROPERTIES dskThumbProps;
			dskThumbProps.dwFlags = DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION;
			dskThumbProps.fSourceClientAreaOnly = FALSE;
			dskThumbProps.fVisible = TRUE;
			dskThumbProps.opacity = 255;
			dskThumbProps.rcDestination = dest;

			// Display the thumbnail
			hr = DwmUpdateThumbnailProperties(m_hThumbnailID, &dskThumbProps);
			if (SUCCEEDED(hr))
			{
				m_bThumbWindowVisible = true;
				ShowWindow(m_hwndThumbTarget, SW_SHOW);
				return;
			}
		}

		// thumbnail can not be created, so hide the thumbnail window
		m_bThumbWindowVisible = false;
		ShowWindow(m_hwndThumbTarget, SW_HIDE);
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------
	//															CMyApp::Draw()
	// --------------------------------------------------------------------------------------------------------------------------------------------
	void Draw() override
	{
		sf::Color bkg_color;
		BYTE r = GetRValue(gSettings.m_clrBkgColor);
		BYTE g = GetGValue(gSettings.m_clrBkgColor);
		BYTE b = GetBValue(gSettings.m_clrBkgColor);
		BYTE a = (100 - gSettings.m_nBkgTransparency) * 255 / 100;
		bkg_color = sf::Color(r, g, b, a);
		m_Window.clear(bkg_color);

		// draw screen background
		m_Window.draw(m_BackSprite);

		// draw window background
		sf::RoundedRectangleShape bkg(sf::Vector2f((float)m_nWindowWidth, (float)m_nWindowHeight), 15, 10);
		bkg.setFillColor(bkg_color);
		m_Window.draw(bkg);

		m_Window.setView(m_View);

		float text_x_offset;
		float text_y_offset;
		const float text_line_spacing = floor(m_pText->getFont()->getLineSpacing((int)(m_pText->getCharacterSize()))) + 1;	// round down looks better
		const float header_line_spacing = ICON_PIXELS + 4;
		const float next_header_spacing = 4;
		float line_spacing;
		const float x_base = 0;
		float y = 1; // 20;
		bool is_first_line = true;
		int prev_level = 0;

		int line = 0;
		int mouse_y = m_nMouseY - m_nListTop;		// subtract list-view top
		mouse_y += m_Windows[m_nTopLine].m_nYTop;	// add scroll position

		for (auto &&it : m_Windows)
		{
			// compute mouse-hover bottom coordinate of current line
			int bottom = it.m_nYBottom + 1;
			if ((size_t)(line + 1) < m_Windows.size())
				bottom = m_Windows[line + 1].m_nYTop;

			if (m_nMouseX >= m_nListLeft && mouse_y >= it.m_nYTop && mouse_y < bottom)
			{
				sf::RoundedRectangleShape rect(sf::Vector2f((float)m_nListWidth - 2.f, (float)(it.m_nYBottom - it.m_nYTop) - 1), 9.f, 10);
				rect.setPosition(1, (float)it.m_nYTop);

				if (it.m_bIsClickable)
				{
					// soft highlight line
					rect.setFillColor(sf::Color(0x00, 0xa0, 0xff));
				}
				else
				{
					// non-clickable line, draw dark green frame
					rect.setFillColor(sf::Color(0x1d, 0x29, 0x51));
				}

				rect.setOutlineColor(sf::Color(0x80, 0x80, 0x80));
				rect.setOutlineThickness(1);
				m_Window.draw(rect);
			}
			else if (line == m_nCursorLine)
			{
				// highlight line
				sf::RoundedRectangleShape rect(sf::Vector2f((float)m_nListWidth - 2.f, (float)(it.m_nYBottom - it.m_nYTop) - 1), 9.f, 10);
				rect.setPosition(1, (float)it.m_nYTop);
				rect.setFillColor(sf::Color(0x00, 0x60, 0xff));
				rect.setOutlineColor(sf::Color::White);
				rect.setOutlineThickness(1);
				m_Window.draw(rect);
			}

			text_x_offset = 0;
			text_y_offset = 1;
			line_spacing = text_line_spacing;

			float x = x_base + 10;	// make room for rounded corners on left side of highlighting box
			if (it.m_nLevel > 0)
			{
				x += ICON_PIXELS + 8.f;
				if (it.m_nLevel > 1)
					x += (it.m_nLevel - 1) * 30.f;

				if (prev_level > it.m_nLevel)
					y += 8;
				else if (prev_level == it.m_nLevel && it.m_bIsHeader)
					y += 4;
			}
			else if (prev_level > it.m_nLevel)
				y += 4;

			prev_level = it.m_nLevel;

			if (it.m_bIsTopLevelHeader)
			{
				// draw big icon
				if (!is_first_line)
					y += next_header_spacing;
				it.m_WinDesc.m_Icon.m_Sprite.setPosition(x, y + (next_header_spacing / 2));
				text_x_offset = ICON_PIXELS + 8.f;
				m_Window.draw(it.m_WinDesc.m_Icon.m_Sprite);
				line_spacing = header_line_spacing;
			}
			else if (!it.m_bIsHeader)
			{
				// draw small icon
				line_spacing = (float)(m_nLineSpacing) + next_header_spacing;
				if (line_spacing < 16)
					line_spacing = 16;
				it.m_WinDesc.m_Icon.m_Sprite.setPosition(x, y + (next_header_spacing / 2));
				text_x_offset = line_spacing + 4.f;
				float factor = (float)(m_nLineSpacing) / (float)ICON_PIXELS;
				it.m_WinDesc.m_Icon.m_Sprite.setScale(factor, factor);
				m_Window.draw(it.m_WinDesc.m_Icon.m_Sprite);
				it.m_WinDesc.m_Icon.m_Sprite.setScale(1.f, 1.f);
			}

			it.m_nYTop = (int)y;
			it.m_nYBottom = it.m_nYTop + (int)line_spacing + 1;

			// draw title or process name
			m_pText->setString(it.m_strTitle.c_str());

			sf::Color text_color;
			if (it.m_bIsClickable)
			{
				text_color = sf::Color(235, 235, 235);	// sf::Color(220, 220, 220);
				m_pText->setStyle(sf::Text::Regular);	// sf::Text::Italic | sf::Text::Bold);
				//m_pText->setCharacterSize(14);
			}
			else
			{
				// text_color = sf::Color(28, 134, 238);
				text_color = sf::Color(0xff, 0x80, 0);
				m_pText->setStyle(sf::Text::Regular);	// (sf::Text::Regular);
				//m_pText->setCharacterSize(14);
			}

			text_y_offset += (line_spacing - text_line_spacing) / 2;

			m_pText->setPosition(x + text_x_offset + 1, y + 1 + text_y_offset);
			m_pText->setColor(sf::Color(0x40, 0x40, 0x40));
			m_Window.draw(*m_pText);

			m_pText->setPosition(x + text_x_offset, y + text_y_offset);
			m_pText->setColor(text_color);
			m_Window.draw(*m_pText);

			y += line_spacing + 1;
			is_first_line = false;
			line++;
		}

		// restore view (background may not be drawn with a view applied)
		m_Window.setView(m_InitialView);

		// draw license string
		if (!m_strLicense.empty())
		{
			float x = 20.f;
			float y = (float)(m_nWindowHeight - 40);

			m_pText->setString(m_strLicense.c_str());
			m_pText->setPosition(x + 1, y + 1);
			m_pText->setColor(sf::Color(0x40, 0x40, 0x40));
			m_Window.draw(*m_pText);

			m_pText->setPosition(x, y);
			m_pText->setColor(sf::Color(255, 255, 255));
			m_Window.draw(*m_pText);
		}
	}
};


// --------------------------------------------------------------------------------------------------------------------------------------------
//															WindowSearcher()
// --------------------------------------------------------------------------------------------------------------------------------------------
BOOL CALLBACK WindowSearcher(HWND hWnd, LPARAM lParam)
{
	TCHAR classname[32];
	GetClassName(hWnd, classname, _tcschars(classname));
	if (_tcscmp(classname, APP_CLASS_NAME) == 0)
	{
		HWND *target = (HWND *)lParam;
		*target = hWnd;
		return FALSE;	// stop search
	}

	return TRUE;		// continue search
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//																main
// --------------------------------------------------------------------------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/* test for breaking titles at word boundaries
	const wchar_t c = L'\u042f'; // cyrillic capital letter ya

	int is = _istalpha(c);
	is = _istalpha(L'0');
	*/

	// Create a mutex with a unique name, so Inno Setup can terminate the program before update / reinstall / uninstall
	CreateMutex(NULL, FALSE, _T("IS$$Mutex$$") APP_NAME);

	// if an instance of our app is running, activate the instance and bail out
	HWND hwndChild = NULL;
	EnumWindows(WindowSearcher, (LPARAM)&hwndChild);	// FindWindow(APP_CLASS_NAME, NULL) does not work, because it is not a top-level window
	if (hwndChild)
	{
		// get the SFML owner window
		HWND hwndParent = GetWindow(hwndChild, GW_OWNER);
		if (hwndParent)
		{
			SetForegroundWindowInternal(hwndParent);
			ShowWindow(hwndParent, SW_SHOW);
			gbEatSingleClick = false;
		}
		return 0;
	}

	// set gstrAppData and gstrCommonAppData
	if (!GetAppPaths())
	{
		MessageBox(NULL,
			_T("Can not retrieve application data directory!\n\n")
			_T("SHGetFolderPath() failed.\n\n")
			_T("Please fix your system."), 
			APP_NAME, MB_ICONERROR);
		return 1;
	}

	gstrAppData += DATA_FOLDER;
	gstrCommonAppData += DATA_FOLDER;
	gstrWindir += _T('\\');
	ghInstance = hInstance;

	// init COM
	CoInitializeEx(NULL, COINIT_MULTITHREADED  | COINIT_DISABLE_OLE1DDE); // COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Run App
	CMyApp app;
	app.Run();

	// cleanup
	// Uninitialize the COM Library
	CoUninitialize();

	return 0;
}
