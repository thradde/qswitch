
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

#include "memdebug.h"

#include <SFML/Graphics.hpp>

#include <vector>
using namespace std;

#define ASSERT
#include "hook.h"
#include "RfwString.h"
#include "exceptions.h"
#include "stream.h"
#include "platform.h"
#include "generic.h"
#include "configuration.h"


#define SETTINGS_MAGIC		_T("QSwitchSettings")
#define SETTINGS_VERSION	1


// --------------------------------------------------------------------------------------------------------------------------------------------
//																Globals
// --------------------------------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------------------------------------
//															CSettings::Read()
// --------------------------------------------------------------------------------------------------------------------------------------------
void CSettings::Read(Stream &stream)
{
	m_nBkgTransparency = stream.ReadInt();
	m_clrBkgColor = stream.ReadUInt();

	m_bDisableIfMaxWin = stream.ReadBool();

	gbHotkeyVKey = stream.ReadByte();				// virtual key code for app activation
	gbHotkeyModifiers = stream.ReadByte();			// modifiers for app activation (shift, ctrl, alt)
	gbFilterHotkeyVKey = stream.ReadByte();			// virtual key code for app activation
	gbFilterHotkeyModifiers = stream.ReadByte();	// modifiers for app activation (shift, ctrl, alt)
	gnMouseButton = stream.ReadInt();
	gnMouseCorner = stream.ReadInt();
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															CSettings::Read()
// --------------------------------------------------------------------------------------------------------------------------------------------
void CSettings::Write(Stream &stream) const
{
	stream.WriteInt(m_nBkgTransparency);
	stream.WriteUInt(m_clrBkgColor);

	stream.WriteBool(m_bDisableIfMaxWin);

	stream.WriteByte(gbHotkeyVKey);				// virtual key code for app activation
	stream.WriteByte(gbHotkeyModifiers);		// modifiers for app activation (shift, ctrl, alt)
	stream.WriteByte(gbFilterHotkeyVKey);		// virtual key code for app activation
	stream.WriteByte(gbFilterHotkeyModifiers);	// modifiers for app activation (shift, ctrl, alt)
	stream.WriteInt(gnMouseButton);
	stream.WriteInt(gnMouseCorner);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															CSettings::Read()
// --------------------------------------------------------------------------------------------------------------------------------------------
void CSettings::ReadFromFile(const RString &file_name)
{
	Stream stream(file_name, _T("rb"));

	// Magic
	RString magic = stream.ReadString();
	if (magic != SETTINGS_MAGIC)
		throw Exception(Exception::EXCEPTION_DB_CORRUPT);

	// Version
	if (stream.ReadUInt() > SETTINGS_VERSION)			// this implies we can always read older versions
		throw Exception(Exception::EXCEPTION_DB_VERSION);

	// read settings
	Read(stream);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//															CSettings::Read()
// --------------------------------------------------------------------------------------------------------------------------------------------
void CSettings::WriteToFile(const RString &file_name) const
{
	_trename(file_name.c_str(), (file_name + _T(".bak")).c_str());
	Stream stream(file_name, _T("wb"));

	// Magic + Version
	stream.WriteString(SETTINGS_MAGIC);
	stream.WriteUInt(SETTINGS_VERSION);

	// write settings
	Write(stream);
}
