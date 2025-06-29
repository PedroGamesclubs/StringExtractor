#include "StringFileExtractor.h"

#include <windows.h>
#include <wincrypt.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma comment(lib, "Crypt32.lib")

StringFileExtractor::StringFileExtractor() {}

std::wstring StringFileExtractor::extract(const std::wstring& filePath) {
    std::wstringstream ss;

    std::wstring md5 = computeMD5(filePath);
    std::wstring sizeOfImage = getSizeOfImageString(filePath);
    std::wstring timestampStr = getTimestampString(filePath);

    ss << L"MD5: " << md5 << L"\r\n";
    ss << L"PcaSvc: " << sizeOfImage << L"\r\n";
    ss << L"DPS: !" << timestampStr << L"\r\n";

    return ss.str();
}

std::wstring StringFileExtractor::computeMD5(const std::wstring& filePath) {
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return L"Erro ao abrir arquivo";

    HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMapping) {
        CloseHandle(hFile);
        return L"Erro ao mapear arquivo";
    }

    BYTE* mappedFile = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!mappedFile) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Erro ao mapear view";
    }

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Erro CryptAcquireContext";
    }
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Erro CryptCreateHash";
    }

    DWORD fileSize = GetFileSize(hFile, nullptr);
    if (!CryptHashData(hHash, mappedFile, fileSize, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Erro CryptHashData";
    }

    BYTE hash[16];
    DWORD hashLen = 16;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Erro CryptGetHashParam";
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    UnmapViewOfFile(mappedFile);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    std::wstringstream md5ss;
    md5ss << std::hex << std::setfill(L'0');
    for (DWORD i = 0; i < hashLen; i++) {
        md5ss << std::setw(2) << (int)hash[i];
    }
    return md5ss.str();
}

std::wstring StringFileExtractor::getSizeOfImageString(const std::wstring& filePath) {
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return L"Erro ao abrir arquivo";

    HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMapping) {
        CloseHandle(hFile);
        return L"Erro ao criar mapeamento";
    }

    BYTE* mappedFile = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!mappedFile) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Erro ao mapear arquivo";
    }

    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)mappedFile;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Arquivo não é PE válido";
    }

    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(mappedFile + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Arquivo não é PE válido";
    }

    DWORD sizeOfImage = nt->OptionalHeader.SizeOfImage;

    UnmapViewOfFile(mappedFile);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    std::wstringstream ss;
    ss << L"0x" << std::hex << sizeOfImage;
    return ss.str();
}

std::wstring StringFileExtractor::getTimestampString(const std::wstring& filePath) {
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return L"Erro ao abrir arquivo";

    HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMapping) {
        CloseHandle(hFile);
        return L"Erro ao criar mapeamento";
    }

    BYTE* mappedFile = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!mappedFile) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Erro ao mapear arquivo";
    }

    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)mappedFile;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Arquivo não é PE válido";
    }

    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(mappedFile + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        UnmapViewOfFile(mappedFile);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return L"Arquivo não é PE válido";
    }

    DWORD timestamp = nt->FileHeader.TimeDateStamp;

    UnmapViewOfFile(mappedFile);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    std::time_t t = static_cast<std::time_t>(timestamp);
    std::tm gmt = {};
    gmtime_s(&gmt, &t);

    wchar_t buffer[32];
    std::wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%Y/%m/%d:%H:%M:%S", &gmt);

    return std::wstring(buffer);
}
