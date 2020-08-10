#pragma once
#include <Windows.h>

void int_pxl_arr_to_byte_pxl_arr(int* int_arr, int length, BYTE* byte_arr);

char* byte_arr_to_bitmap(BYTE* pBitmapBits);

int take_web_capture(char** bmp_buffer);

int get_webcam_capture_size();
