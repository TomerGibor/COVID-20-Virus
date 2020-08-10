#include <stdio.h>
#include "logger.h"
#include "keylogger_utils.h"

SPECIAL_KEY* get_special_key_by_key_code(int key_code, SPECIAL_KEY specials[], int len) {
	SPECIAL_KEY* special_key = NULL;
	int i = 0;

	for (; i < len; i++)
	{
		if (specials[i].key_code == key_code) {
			special_key = &(specials[i]);
			return special_key;
		}
	}
	return special_key;
}

BOOL key_code_in_symbols(int key_code, SYMBOL symbols[], int len) {
	int i = 0;

	for (; i < len; i++)
	{
		if (symbols[i].key_code == key_code)
			return TRUE;
	}
	return FALSE;
}