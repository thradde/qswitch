

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
