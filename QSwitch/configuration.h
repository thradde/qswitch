
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


// --------------------------------------------------------------------------------------------------------------------------------------------
//															class CSettings
//
// This class is used to store global settings.
// The data herein is used for persistent storage in streams and it is also used for dialog exchange.
// --------------------------------------------------------------------------------------------------------------------------------------------
class CSettings
{
public:
	int			m_nBkgTransparency;	// 1 - 100 = 0.01 - 1.00, where 1.0 = FULLY TRANSPARENT (!)
	COLORREF	m_clrBkgColor;		// desktop solid color
	bool		m_bDisableIfMaxWin;	// if true, the hooks are disabled if the foreground window is maximized (eg it is a game)

public:
	CSettings()
		:	m_nBkgTransparency(1),				// use 15 if bkg_color is black
			m_clrBkgColor(RGB(13, 13, 13)),	// (26, 42, 77)		(35, 56, 102)		(0x0a, 0x3b, 0x76)),
			m_bDisableIfMaxWin(true)
	{
	}

	void Read(Stream &stream);
	void Write(Stream &stream) const;
	void ReadFromFile(const RString &file_name);
	void WriteToFile(const RString &file_name) const;

	bool BkgChanged(const CSettings &rhs) const
	{
		return
			m_nBkgTransparency	!= rhs.m_nBkgTransparency ||
			m_clrBkgColor	!= rhs.m_clrBkgColor;
	}

	/* bool operator !=(const CSettings &rhs) const
	{
		return
			BkgChanged(rhs);
	} */
};
