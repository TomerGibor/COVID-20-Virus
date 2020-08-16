#include <Windows.h>
#define BITS_PER_PIXEL 24

char* hbitmap_to_bitmap_buffer(HBITMAP h_bitmap);

char* take_screen_capture();

int get_screen_capture_size();

void initialize_dimensions();
