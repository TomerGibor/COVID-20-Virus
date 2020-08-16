#pragma once
#include "http.h"
#include "../constants.h"

char* parse_request_http_header(HTTP_HEADER header);

int parse_request_http_headers(HTTP_HEADER headers[], unsigned int headers_length, char* parsed_http_headers_ptr);

HTTP_HEADER* parse_response_header_from_text(char* text);

void parse_commands_from_json(char* json_text, char commands[MAX_COMMANDS][MAX_COMMAND_LEN],
	unsigned int* length);

COMMANDS_HTTP_RESPONSE parse_response(char* response, unsigned int response_length);

char* next_token(char* text, const char* delimiter, unsigned int delimiter_length);

HTTP_HEADER build_header(char* name, char* value);

char* get_datetime();

