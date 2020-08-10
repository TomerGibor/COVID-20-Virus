#pragma once
#define MAX_STR_LEN 64
#define MAX_HEADER_LEN (2 * MAX_STR_LEN + 4)
#define MAX_REQUEST_LEN 1024
#define MAX_RESPONSE_LEN 1024
#define MAX_HEADERS 16
#define MAX_COMMANDS 10
#define MAX_COMMAND_LEN 64

typedef struct _HTTP_HEADER {
	char name[MAX_STR_LEN];
	char value[MAX_STR_LEN];
} HTTP_HEADER;

typedef struct _HTTP_RESPONSE {
	HTTP_HEADER headers[MAX_HEADERS];
	char commands[MAX_COMMANDS][MAX_COMMAND_LEN];
} HTTP_RESPONSE;

char* create_get_http_request(HTTP_HEADER headers[], unsigned int headers_length, char* request_uri);

int send_without_listening(char* request, unsigned int length);

HTTP_RESPONSE send_get_command(char* request_uri);

char* get_mac_address();

int send_request_and_listen_for_response(char* request, unsigned int length, HTTP_RESPONSE* http_response);

char* create_post_http_request_with_data(HTTP_HEADER headers[], unsigned int headers_length,
	char* response_uri, char* body, unsigned int body_length, unsigned int* response_length);

void send_post_request_without_listening(char* data, unsigned int length, char* uri, char* content_type);
