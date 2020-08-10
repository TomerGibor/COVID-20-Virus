#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "escapi.h"
#include <string.h>
#include "webcam.h"
#include "capture_utils.h"
#pragma warning(disable:4996)
#define WIDTH 1920
#define HEIGHT 1080
#define PIXEL_SIZE 4
#define BITS_PER_PIXEL 24

void int_pxl_arr_to_byte_pxl_arr(int* int_arr, int length, BYTE* byte_arr) {
	int c = 0;
	for (int i = 0; i < length; i++)
	{
		byte_arr[c] = (BYTE) (int_arr[i] & 0xff);
		byte_arr[c + 1] = (BYTE) (int_arr[i] >> 8 & 0xff);
		byte_arr[c + 2] = (BYTE) (int_arr[i] >> 16 & 0xff);
		c += 3;
	}
}

char* byte_arr_to_bitmap(BYTE* bmp_data) {
	unsigned long headers_size = get_headers_size();
	unsigned long pixel_data_size = get_image_size(WIDTH, HEIGHT, BITS_PER_PIXEL);
	BITMAPINFOHEADER bmp_info_header = { 0 };
	BITMAPFILEHEADER bmp_file_header = { 0 };
	char* bitmap_file = NULL;
	int pos = 0;

	bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bmp_info_header.biBitCount = BITS_PER_PIXEL;
	bmp_info_header.biClrImportant = 0;
	bmp_info_header.biClrUsed = 0;
	bmp_info_header.biCompression = BI_RGB;
	bmp_info_header.biHeight = -HEIGHT;
	bmp_info_header.biWidth = WIDTH;
	bmp_info_header.biPlanes = 1;
	bmp_info_header.biSizeImage = pixel_data_size;

	bmp_file_header.bfType = 0x4D42; // "BM"
	bmp_file_header.bfOffBits = headers_size;
	bmp_file_header.bfSize = headers_size + pixel_data_size;
	
	bitmap_file = (char*) calloc(sizeof(bmp_file_header) + sizeof(bmp_info_header) + bmp_info_header.biSizeImage, sizeof(char));
	
	memcpy(bitmap_file, &bmp_file_header, sizeof(bmp_file_header));
	pos += sizeof(bmp_file_header);
	memcpy(bitmap_file + pos, &bmp_info_header, sizeof(bmp_info_header));
	pos += sizeof(bmp_info_header);
	memcpy(bitmap_file + pos, bmp_data, bmp_info_header.biSizeImage);
	pos += bmp_info_header.biSizeImage;
	return bitmap_file;
}

int take_web_capture(char** bmp_buffer) {
	struct SimpleCapParams capture = { 0 };
	int devices = 0;
	int* target_buffer = (int*)  malloc(WIDTH * HEIGHT * PIXEL_SIZE * sizeof(int));
	BYTE* pixel_buffer = (BYTE*) malloc(WIDTH * HEIGHT * PIXEL_SIZE * sizeof(BYTE));

	devices = setupESCAPI();
	if (!devices){
		perror("No devices found!\n");
		return 0;
	}
	capture.mWidth = WIDTH;
	capture.mHeight = HEIGHT;
	capture.mTargetBuf = target_buffer;
	if (initCapture(0, &capture) == 0){
		perror("Capture failed - device may already be in use.\n");
		return 0;
	}

	printf("Got %d devices\n", devices);

	doCapture(0);
	while (isCaptureDone(0) == 0);

	int_pxl_arr_to_byte_pxl_arr(capture.mTargetBuf, capture.mWidth * capture.mHeight, pixel_buffer);
	deinitCapture(0);

	*bmp_buffer = byte_arr_to_bitmap(pixel_buffer);
	return 1;
}

int get_webcam_capture_size() {
	return get_headers_size() + get_image_size(WIDTH, HEIGHT, BITS_PER_PIXEL);
}