#pragma once

#include <string>
#include <windows.h>

#define LAUNCH_STATUS_PIPE "\\\\.\\pipe\\smf\\launchstatus"

namespace IPC {
	const char* SpawnAndWaitForPipe(std::string pipeName);
	HANDLE OpenPipe(std::string pipeName);
	const char* ReadBytes(HANDLE hPipe);
	std::string ReadMessage(HANDLE hPipe);
	void WriteBytes(HANDLE hPipe, const char* message);
	void WriteMessage(HANDLE hPipe, std::string message);
	void ClosePipe(HANDLE hPipe);
};
