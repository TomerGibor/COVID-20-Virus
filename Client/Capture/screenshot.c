#include <Windows.h>
#include <string.h>
#include "screenshot.h"
#include "capture_utils.h"
#pragma comment(lib, "Gdi32.lib")

#define BITS_PER_PIXEL 24

unsigned int WIDTH = 1920; // default width
unsigned int HEIGHT = 1080; // default height

char* hbitmap_to_bitmap_buffer(HBITMAP h_bitmap) {
    HDC hDC = NULL;
    int i_bits = 0;
    DWORD image_size = get_image_size(WIDTH, HEIGHT, BITS_PER_PIXEL);
    BITMAP bitmap = { 0 };
    BITMAPFILEHEADER bmp_f_hdr = { 0 };
    BITMAPINFOHEADER bmp_info_hdr = { 0 };
    LPBITMAPINFOHEADER lp_bmp_info_hdr = NULL;
    HANDLE h_dib = NULL, h_pal = NULL, h_old_pal = NULL;
    char* bitmap_buffer = NULL;

    hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
    i_bits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
    DeleteDC(hDC);
    
    GetObject(h_bitmap, sizeof(bitmap), (LPSTR)&bitmap);
    bmp_info_hdr.biSize = sizeof(BITMAPINFOHEADER);
    bmp_info_hdr.biWidth = bitmap.bmWidth;
    bmp_info_hdr.biHeight = -bitmap.bmHeight;
    bmp_info_hdr.biPlanes = 1;
    bmp_info_hdr.biBitCount = BITS_PER_PIXEL;
    bmp_info_hdr.biCompression = BI_RGB;
    bmp_info_hdr.biSizeImage = 0;
    bmp_info_hdr.biXPelsPerMeter = 0;
    bmp_info_hdr.biYPelsPerMeter = 0;
    bmp_info_hdr.biClrImportant = 0;
    bmp_info_hdr.biClrUsed = 256;

    h_dib = GlobalAlloc(GHND, image_size + sizeof(BITMAPINFOHEADER));
    lp_bmp_info_hdr = (LPBITMAPINFOHEADER)GlobalLock(h_dib);
    *lp_bmp_info_hdr = bmp_info_hdr;

    h_pal = GetStockObject(DEFAULT_PALETTE);
    if (h_pal){
        hDC = GetDC(NULL);
        h_old_pal = SelectPalette(hDC, (HPALETTE)h_pal, FALSE);
        RealizePalette(hDC);
    }

    GetDIBits(hDC, h_bitmap, 0, (UINT)bitmap.bmHeight, (LPSTR)lp_bmp_info_hdr + sizeof(BITMAPINFOHEADER),
        (BITMAPINFO*)lp_bmp_info_hdr, DIB_RGB_COLORS);

    if (h_old_pal){
        SelectPalette(hDC, (HPALETTE)h_old_pal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
    }

    bmp_f_hdr.bfType = 0x4D42; // "BM"
    bmp_f_hdr.bfSize = get_headers_size() + image_size;
    bmp_f_hdr.bfOffBits = get_headers_size();
    
    bitmap_buffer = (char*)malloc(get_headers_size() + image_size);
    memcpy(bitmap_buffer, &bmp_f_hdr, sizeof(BITMAPFILEHEADER));
    memcpy(bitmap_buffer + sizeof(BITMAPFILEHEADER), lp_bmp_info_hdr, sizeof(BITMAPINFOHEADER) + image_size);

    GlobalUnlock(h_dib);
    GlobalFree(h_dib);

    return bitmap_buffer;
}

char* take_screen_capture(){
    initialize_dimensions();
    HDC hdcSource = GetDC(NULL);
    HDC hdcMemory = CreateCompatibleDC(hdcSource);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, WIDTH, HEIGHT);
    HBITMAP hBitmapOld = (HBITMAP) SelectObject(hdcMemory, hBitmap);

    BitBlt(hdcMemory, 0, 0, WIDTH, HEIGHT, hdcSource, 0, 0, SRCCOPY);
    hBitmap = (HBITMAP) SelectObject(hdcMemory, hBitmapOld);

    DeleteDC(hdcSource);
    DeleteDC(hdcMemory);
    
    return hbitmap_to_bitmap_buffer(hBitmap);
}

int get_screen_capture_size() {
    return get_headers_size() + get_image_size(WIDTH, HEIGHT, BITS_PER_PIXEL);
}


void initialize_dimensions() {
    RECT desktop = { 0 };
    const HWND h_desktop = GetDesktopWindow();
    GetWindowRect(h_desktop, &desktop);
    WIDTH = desktop.right;
    HEIGHT = desktop.bottom;
}


