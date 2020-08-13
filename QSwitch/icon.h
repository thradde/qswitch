
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


#define ICON_PIXELS		32		// 32 x 32 pixels


// --------------------------------------------------------------------------------------------------------------------------------------------
//																class CSize
// --------------------------------------------------------------------------------------------------------------------------------------------
class CSize
{
public:
	int	x;
	int y;

public:
	CSize()
		: x(0)
		, y(0)
	{
	}

	CSize(int xx, int yy)
		: x(xx)
		, y(yy)
	{
	}
};


// --------------------------------------------------------------------------------------------------------------------------------------------
//																class CBitmap
// Storage format of bitmap data is always RGBA
// --------------------------------------------------------------------------------------------------------------------------------------------
class CBitmap
{
protected:
	CSize	m_Size;		// size in pixels
	BYTE	*m_pBitmap;	// bitmap data

public:
	CBitmap(const CSize &size, BYTE *bitmap)
		:	m_Size(size),
			m_pBitmap(bitmap)
	{
	}

	CBitmap()
		:	m_pBitmap(nullptr)
	{
	}

	CBitmap &operator=(const CBitmap &other)
	{
		// self-assignment guard
		if (this == &other)
			return *this;

		FreeBitmap();
		m_Size = other.m_Size;

		if (other.m_pBitmap)
		{
			size_t nbytes = 4 * m_Size.x * m_Size.y;
			m_pBitmap = new BYTE[nbytes];
			memcpy(m_pBitmap, other.m_pBitmap, nbytes);
		}
		else
			m_pBitmap = nullptr;

		return *this;
	}

	CBitmap(const CBitmap &other)
	{
		*this = other;
	}

	~CBitmap()
	{
		FreeBitmap();
	}

	void FreeBitmap()
	{
		if (m_pBitmap)
		{
			delete[] m_pBitmap;
			m_pBitmap = nullptr;
		}
	}

	const CSize &GetSize() const { return m_Size; }

	void SetSize(const CSize &size)
	{
		m_Size = size;
	}

	const BYTE *GetBitmapData() const
	{
		return m_pBitmap;
	}

	void SetBitmapData(BYTE *bitmap)
	{
		FreeBitmap();
		m_pBitmap = bitmap;
	}

	unsigned int GetDataSize() const
	{
		return m_Size.x * m_Size.y * 4;		// RGBA = 4 bytes per pixel
	}
};


// --------------------------------------------------------------------------------------------------------------------------------------------
//														class CIcon
// --------------------------------------------------------------------------------------------------------------------------------------------
class CIcon
{
public:
	RString			m_strProcess;		// process path-name
	CBitmap			m_Bitmap;			// bitmap raw data, required persistent storage for texture
	sf::Texture		m_DrawTexture;		// SFML-Texture, which is exactly scaled to the required resolution for GUI drawing
	sf::Sprite		m_Sprite;			// The drawing sprite

public:
	CIcon()
	{
	}

	CIcon(const RString &process)
		: m_strProcess(process)
	{
	}

	CIcon &operator=(const CIcon &other)
	{
		// self-assignment guard
		if (this == &other)
			return *this;

		m_strProcess = other.m_strProcess;
		m_Bitmap = other.m_Bitmap;
		m_DrawTexture = other.m_DrawTexture;

		// the sprite needs to be updated, because it points to the old texture!
		m_Sprite.setTexture(m_DrawTexture, true);
		m_Sprite.setColor(sf::Color(255, 255, 255, 255));

		return *this;
	}

	CIcon(const CIcon &other)
	{
		*this = other;
	}

	void CreateBitmap(HWND hWnd);
};
