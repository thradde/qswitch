
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

typedef unsigned int uint32;
typedef uint32 *DIBitmap;

struct MyIconInfo
{
	int     nWidth;
	int     nHeight;
	int     nBitsPerPixel;
};


bool MyGetIconInfo(HICON hIcon, MyIconInfo &info);

void InitializeBitmapHeader(BITMAPV5HEADER* header, int width, int height);
DIBitmap BGRA2RGBA(DIBitmap org_bitmap, const SIZE &size);
DIBitmap MakeBmpTopDown(DIBitmap org_bitmap, const SIZE &size);
DIBitmap GetIconBitmap(LPCTSTR filePath, int &pixels, bool thumbnail);
DIBitmap CreateDIBitmapFromHICON(HICON icon, int size_x, int size_y);
