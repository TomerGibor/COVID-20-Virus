#pragma once
#include "logger.h"

SPECIAL_KEY* get_special_key_by_key_code(int key_code, SPECIAL_KEY specials[], int len);

BOOL key_code_in_symbols(int key_code, SYMBOL symbols[], int len);