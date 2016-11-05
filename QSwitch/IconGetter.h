
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
