#include <Windows.h>
#include "capture_utils.h"

int get_image_size(unsigned int width, unsigned int height, unsigned int bits_per_pixel) {
	return height * ((width * (bits_per_pixel / 8)));
}

int get_headers_size() {
	return sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
}
