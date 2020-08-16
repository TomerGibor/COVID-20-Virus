#include "http.h"
#define WIN32_LEAN_AND_MEAN
#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "IPHLPAPI.lib")
#pragma warning (disable:4996)

int winsock_setup(SOCKET* s) {
	int iResult = 0;
	WSADATA wsa_data = { 0 };
	struct addrinfo* result = NULL, * ptr = NULL, hints = { 0 };

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (iResult != NO_ERROR) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 0;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(HOST, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 0;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		*s = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (*s == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 0;
		}

		// Connect to server
		iResult = connect(*s, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(*s);
			*s = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (*s == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 0;
	}
	return 1;
}
char* create_get_http_request(HTTP_HEADER headers[], unsigned int headers_length, char* request_uri) {
	char* request = (char*)calloc(MAX_GET_REQUEST_LEN, sizeof(char));
	char* parsed_headers = (char*) calloc(MAX_HEADER_LEN * headers_length, sizeof(char));
	unsigned int parsed_headers_length = parse_request_http_headers(headers, headers_length, parsed_headers);

	/* 
	GET request structure (newline present for ease of read):
	GET /<request_uri> HTTP/1.1\r\n
	<parsed_headers>\r\n
	*/
	strncat(request, GET, sizeof(GET) - 1);
	strncat(request, request_uri, strnlen(request_uri, MAX_STR_LEN));
	strncat(request, HTTP, sizeof(HTTP) - 1);
	strncat(request, parsed_headers, parsed_headers_length);
	strncat(request, CRLF, sizeof(CRLF) - 1);

	free(parsed_headers);
	return request;
}

int send_without_listening(char* request, unsigned int length) {
	SOCKET s = INVALID_SOCKET;
	int iResult = 0;

	iResult = winsock_setup(&s);
	if (iResult == 0)
		return 0;

	// Send request
	iResult = send(s, request, length, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}

	printf("Bytes Sent: %d\n", iResult);

	// Shutdown the connection since no more data will be sent
	iResult = shutdown(s, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}

	// Close the socket
	iResult = closesocket(s);
	if (iResult == SOCKET_ERROR) {
		printf("close failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	WSACleanup();
	return 1;
}

COMMANDS_HTTP_RESPONSE send_get_command(char* request_uri) {
	COMMANDS_HTTP_RESPONSE http_response = { 0 };
	char* request = NULL;
	unsigned int headers_length = 0;
	HTTP_HEADER headers[] = {
		{"Connection", "Keep-Alive"},
		{"Charset", "utf-8"},
		build_header("Date", get_datetime()),
		build_header("Host", HOST),
		build_header("MAC-Address", get_mac_address())
	};
	headers_length = sizeof(headers) / sizeof(HTTP_HEADER);
	request = create_get_http_request(headers, headers_length, request_uri); // create GET request
	printf("SENDING REQUEST:\n%s\n", request);
	send_request_and_listen_for_response(request,
		strnlen(request, MAX_GET_REQUEST_LEN), &http_response);

	free(request);
	return http_response;
}

char* get_mac_address() {
	ULONG out_buf_len = WORKING_BUFFER_SIZE;
	unsigned int i = 0, msb = 0, lsb = 0, length = 0;
	char* mac_address = (char*)calloc(MAC_ADDRESS_LENGTH, sizeof(char));
	IP_ADAPTER_ADDRESSES* pAddresses = (IP_ADAPTER_ADDRESSES*) malloc(out_buf_len);
	ULONG res = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &out_buf_len);

	if (res == ERROR_BUFFER_OVERFLOW) {
		free(pAddresses);
		pAddresses = NULL;
		return MAC_ADDRESS_ERROR;
	}

	for (; i < (int)pAddresses->PhysicalAddressLength; i++) {
		msb = pAddresses->PhysicalAddress[i] >> 4;
		lsb = pAddresses->PhysicalAddress[i] & 0x0F;
		msb += msb < 10 ? 0x30 : 0x37;
		lsb += lsb < 10 ? 0x30 : 0x37;
		mac_address[length] = (char) msb;
		mac_address[length + 1] = (char) lsb;
		mac_address[length + 2] = '-';
		length += 3;
	}
	mac_address[length - 1] = 0;
	free(pAddresses);
	return mac_address;
}

int send_request_and_listen_for_response(char* request, unsigned int len, COMMANDS_HTTP_RESPONSE* http_response) {
	SOCKET s = INVALID_SOCKET;
	int iResult = 0;
	char temp_buffer[MAX_BUFFER_LEN] = "";
	char recvbuf[MAX_RESPONSE_LEN] = "";

	iResult = winsock_setup(&s);
	if (iResult == 0)
		return 0;

	// Send request
	iResult = send(s, request, len, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}
	printf("Bytes Sent: %d\n", iResult);

	// Shutdown the connection since no more data will be sent
	iResult = shutdown(s, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}

	// Receive until the peer closes the connection
	do {
		iResult = recv(s, temp_buffer, MAX_BUFFER_LEN, 0);
		if (iResult > 0) {
			strncat(recvbuf, temp_buffer, iResult);
			printf("Bytes received: %d\n", iResult);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

	} while (iResult > 0);

	printf("RECEIVED RESPONSE:\n%s\n", recvbuf);
	*http_response = parse_response(recvbuf, strnlen(recvbuf, MAX_RESPONSE_LEN));

	// Close the socket
	iResult = closesocket(s);
	if (iResult == SOCKET_ERROR) {
		printf("close failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	WSACleanup();
	return 1;
}

char* create_post_http_request_with_data(HTTP_HEADER headers[], unsigned int headers_length,
	char* request_uri, char* body, unsigned int body_length, unsigned int* request_length) {
	char* request = (char*) calloc(MAX_RESPONSE_LEN + body_length, sizeof(char));
	char* parsed_headers = (char*) calloc(MAX_HEADER_LEN * headers_length, sizeof(char));
	unsigned int parsed_headers_length = parse_request_http_headers(headers, headers_length, parsed_headers);
	unsigned int pos = 0;

	/*
	POST request structure (newlines present for ease of read):
	POST /<request_uri> HTTP/1.1\r\n
	<parsed_headers>\r\n
	<body>\r\n\r\n
	*/
	strncat(request, POST, sizeof(POST) - 1); // POST /
	pos += sizeof(POST) - 1;
	strncat(request, request_uri, strnlen(request_uri, MAX_STR_LEN)); // uri
	pos += strnlen(request_uri, MAX_STR_LEN);
	strncat(request, HTTP, sizeof(HTTP) - 1); // HTTP/1.1\r\n
	pos += sizeof(HTTP) - 1;
	strncat(request, parsed_headers, parsed_headers_length); // headers
	pos += parsed_headers_length;
	strncat(request, CRLF, sizeof(CRLF) - 1); // \r\n
	pos += sizeof(CRLF) - 1;
	memcpy(request + pos, body, body_length); // body
	pos += body_length;
	strncpy(request + pos, CRLF, sizeof(CRLF) - 1); // \r\n
	pos += sizeof(CRLF) - 1;
	strncpy(request + pos, CRLF, sizeof(CRLF) - 1); // \r\n
	pos += sizeof(CRLF) - 1;
	*request_length = pos;
	
	free(parsed_headers);
	return request;
}

void send_post_request_without_listening(char* data, unsigned int length, char* uri, char* content_type) {
	char* request = NULL;
	unsigned int headers_length = 0;
	HTTP_HEADER content_length_header = { "Content-Length", 0 };
	unsigned int request_length = 0;
	itoa(length, content_length_header.value, 10);
	HTTP_HEADER headers[] = {
		{"Connection", "Keep-Alive"},
		build_header("Content-Type", content_type),
		content_length_header,
		build_header("Date", get_datetime()),
		build_header("Host", HOST),
		build_header("MAC-Address", get_mac_address())
	};
	headers_length = sizeof(headers) / sizeof(HTTP_HEADER);
	request = create_post_http_request_with_data(headers, headers_length, uri,
		data, length, &request_length);
	send_without_listening(request, request_length);
	free(request);
}