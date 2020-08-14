
// Copyright (c) 2020 IDEAL Software GmbH, Neuss, Germany.
// www.idealsoftware.com
// Author: Thorsten Radde
//
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU 
// General Public License as published by the Free Software Foundation; either version 2 of the License, 
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU General Public 
// License for more details.
//
// You should have received a copy of the GNU General Public License along with this program; if not, 
// write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.
//

#define STRICT
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include "shobjidl.h"
#include "shlguid.h"

#include "memdebug.h"

#include <SFML/Graphics.hpp>

#include <vector>
#include <set>
using namespace std;

#define ASSERT
#include "RfwString.h"
#include "exceptions.h"
#include "Mutex.h"
#include "stream.h"
#include "platform.h"
#include "generic.h"
#include "configuration.h"
#include "IconGetter.h"
#include "my_resampler.h"
#include "icon.h"


// --------------------------------------------------------------------------------------------------------------------------------------------
//																Globals
// --------------------------------------------------------------------------------------------------------------------------------------------
static CResampler gResampler;


// --------------------------------------------------------------------------------------------------------------------------------------------
//														MySendMessage()
//
// If a window hangs, this calling app will hang in SendMessage().
// To avoid this, we use SendMessageTimeout().
// --------------------------------------------------------------------------------------------------------------------------------------------
LRESULT MySendMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret;
	//PDWORD_PTR
	DWORD_PTR result;

	ret = SendMessageTimeout(
		hwnd,
		msg,
		wParam,
		lParam,
		SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT,
		100,
		&result);

	if (!ret)
		return 0;

	return result;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//														GetAppIcon()
// --------------------------------------------------------------------------------------------------------------------------------------------
HICON GetAppIcon(HWND hwnd)
{
	HICON hIcon;

	hIcon = (HICON)MySendMessage(hwnd, WM_GETICON, ICON_BIG, 0);

	if (hIcon == NULL)
		hIcon = (HICON)MySendMessage(hwnd, WM_GETICON, ICON_SMALL2, 0);

	if (hIcon == NULL)
		hIcon = (HICON)MySendMessage(hwnd, WM_GETICON, ICON_SMALL, 0);

	if (hIcon == NULL)
		hIcon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);

	if (hIcon == NULL)
		hIcon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);

	// nothing found, use default Windows App-Icon
	//if (hIcon == NULL)
	//	hIcon = ::LoadIcon(0, IDI_APPLICATION);

	return hIcon;
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//													CIcon2::CreateBitmap()
// --------------------------------------------------------------------------------------------------------------------------------------------
void CIcon::CreateBitmap(HWND hWnd)
{
	DIBitmap bmp = NULL;

	HICON hIcon = GetAppIcon(hWnd);
	int pixels = ICON_PIXELS;

	if (hIcon)
	{
		bmp = CreateDIBitmapFromHICON(hIcon, ICON_PIXELS, ICON_PIXELS);
	}
	else if (!m_strProcess.empty())
	{
		// use shell-api
		bmp = GetIconBitmap(m_strProcess, pixels, false);
	}

	if (!bmp)
	{
		// use placeholder icon (Windows standard app icon)
		hIcon = LoadIcon(NULL, IDI_APPLICATION);
		bmp = CreateDIBitmapFromHICON(hIcon, ICON_PIXELS, ICON_PIXELS);
	}

	unsigned char *resampled;
	if (pixels == ICON_PIXELS)
		resampled = (unsigned char *)bmp;
	else
	{
		resampled = gResampler.Resample((unsigned char *)bmp, pixels, pixels, 4, ICON_PIXELS, ICON_PIXELS);
		if (!resampled)
		{
			// todo: mark icon as undrawable ==> use a default bitmap!
			// this can happen! at least theoretically, but then we are out of memory
			return;
		}

		delete[] bmp;
	}

	m_Bitmap.SetSize(CSize(ICON_PIXELS, ICON_PIXELS));
	m_Bitmap.SetBitmapData(resampled);
	m_DrawTexture.create(ICON_PIXELS, ICON_PIXELS);
	m_DrawTexture.update((sf::Uint8 *)resampled);
	m_DrawTexture.setSmooth(true);

	m_Sprite.setTexture(m_DrawTexture, true);
	m_Sprite.setColor(sf::Color(255, 255, 255, 255));
}
