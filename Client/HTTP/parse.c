#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#pragma warning(disable:4996)

char* parse_request_http_header(HTTP_HEADER header) {
	unsigned int name_len = strnlen(header.name, MAX_STR_LEN);
	unsigned int value_len = strnlen(header.value, MAX_STR_LEN);
	unsigned int len = name_len + sizeof(COLON) - 1 + value_len + sizeof(CRLF) - 1;
	char* parsed_http_header = (char*) calloc(len, sizeof(char));

	//HTTP header structure: "<header_name>: <header_value>\r\n"
	strncat(parsed_http_header, header.name, name_len);
	strncat(parsed_http_header, COLON, sizeof(COLON) - 1);
	strncat(parsed_http_header, header.value, value_len);
	strncat(parsed_http_header, CRLF, sizeof(CRLF) - 1);

	return parsed_http_header;
}

int parse_request_http_headers(HTTP_HEADER headers[], unsigned int headers_length, char* parsed_headers_ptr) {
	unsigned int i = 0, length = 0;
	char* parsed_header = NULL;

	for (; i < headers_length; i++)
	{
		parsed_header = parse_request_http_header(headers[i]);
		strncat(parsed_headers_ptr, parsed_header, strnlen(parsed_header, MAX_HEADER_LEN));
		length += strnlen(parsed_header, MAX_HEADER_LEN);
	}

	return length;
}

HTTP_HEADER* parse_response_header_from_text(char* arg) {
	char* token = NULL;
	HTTP_HEADER* header = (HTTP_HEADER*)calloc(1, sizeof(HTTP_HEADER*));
	char* text = strdup(arg);

	token = strtok(text, COLON); // name of HTTP header
	strncpy(header->name, token, strnlen(token, MAX_STR_LEN));
	header->name[strnlen(token, MAX_STR_LEN)] = 0;
	token = strtok(NULL, COLON); // value of HTTP header
	strncpy(header->value, token, strnlen(token, MAX_STR_LEN));
	header->value[strnlen(token, MAX_STR_LEN)] = 0;

	return header;
}

void parse_commands_from_json(char* json_text, char commands[MAX_COMMANDS][MAX_COMMAND_LEN], unsigned int* length) {
	/*
	JSON structure in response:
	{"commands": ["command1", "command2", "command3", ...]}
	*/
	char* token = NULL;
	char* token2 = NULL;
	char* command = NULL;
	char* json = strdup(json_text);
	char* temp_command = (char*)calloc(MAX_COMMAND_LEN, sizeof(char));
	unsigned int command_len = 0;
	unsigned int i = 0;

	token = strtok(json, ":"); // skip "commands"
	token = strtok(NULL, ":"); // token is now the list of commands
	token2 = strtok(token, ",");
	*length = 0;
	while (token2) { // loop until token2 is NULL
		command = strdup(token2); // duplicate token2 so it can be looped over
		while (*command < 'A' || (*command > 'Z' && *command < 'a') || *command > 'z')
			command++; // get rid of spaces and quotes before command
		command_len = 0;
		while (*command != '"') { // loop until reached end of command
			temp_command[command_len] = *command; // build command letter-by-letter
			command_len++;
			command++;
		}
		strncpy(commands[*length], temp_command, command_len); //copy command into the array of commands
		for (i = 0; i < command_len; i++)
			temp_command[i] = 0; // erase old command
		token2 = strtok(NULL, ",");
		(*length)++;
	}
	free(temp_command);
	free(json);
}

char* next_token(char* text, const char* delimiter, unsigned int delimiter_length) {
	char* token = NULL;
	char* start = NULL;
	char* end = NULL;
	unsigned int token_length = 0;

	start = strstr(text, delimiter);
	if (start == NULL) // delimiter not found in string
		return NULL;
	end = strstr(start + 1, delimiter);
	if (end == NULL) // start is the last section of the string
		return strdup(start + delimiter_length);
	token_length = end - (start + delimiter_length);
	token = (char*) malloc((token_length + 1) * sizeof(char));
	strncpy(token, start + delimiter_length, token_length);
	token[token_length] = 0;
	return token;
}

COMMANDS_HTTP_RESPONSE parse_response(char* response, unsigned int response_length) {
	char* token = NULL;
	COMMANDS_HTTP_RESPONSE http_response = { 0 };
	char commands[MAX_COMMANDS][MAX_COMMAND_LEN] = { 0 };
	unsigned int length = 0, i = 0, headers_len = 0, pos = 0;

	if (response[response_length] != 0) { // check that response ends with \0 (for safety)
		printf("INVALID RESPONSE: No \\0 at the end of response string\n");
		return http_response; // return empty response
	}

	response = strstr(response, CRLF); // discard response line header
	if (response == NULL) {
		printf("INVALID RESPONSE: No \\r\\n in response\n");
		return http_response; // return empty response
	}
	token = next_token(response + pos, CRLF, sizeof(CRLF) - 1);

	while (token != NULL){ // loop until token is NULL
		if (strcmp(token, "") == 0) { 
			pos += sizeof(CRLF) - 1;
			token = next_token(response + pos, CRLF, sizeof(CRLF) - 1);
			continue;
		}
		if (*token == '{') { // reached body containing json with commands
			parse_commands_from_json(token, commands, &length);
			for (i = 0; i < length; i++) {
				strncpy(http_response.commands[i], commands[i], strnlen(commands[i], MAX_COMMAND_LEN));
			}
		}
		else { // token is HTTP HEADER
			http_response.headers[headers_len] = *parse_response_header_from_text(token);
			headers_len++;
		}
		pos += strnlen(token, MAX_TOKEN_LEN) + sizeof(CRLF) - 1;
		token = next_token(response + pos, CRLF, sizeof(CRLF) - 1);
	}
	return http_response;
}

HTTP_HEADER build_header(char* name, char* value) {
	HTTP_HEADER http_header = { 0 };
	strncpy(http_header.name, name, strnlen(name, MAX_STR_LEN));
	strncpy(http_header.value, value, strnlen(value, MAX_STR_LEN));
	return http_header;
}

char* get_datetime() {
	time_t timer = time(NULL);
	struct tm* timeinfo = gmtime(&timer);
	char* curr_gmt_time = (char*) calloc(DATETIME_LEN, sizeof(char));
	strftime(curr_gmt_time, DATETIME_LEN, DATETIME_FORMAT, timeinfo);
	return curr_gmt_time;
}


