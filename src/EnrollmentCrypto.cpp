// ----------------------------------------------------------------------------------------------------------------
// Copyright 2026 Hernan Di Pietro
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
// to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ----------------------------------------------------------------------------------------------------------------
#include "pch.h"
#include "EnrollmentCrypto.h"

#include <wincrypt.h>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

namespace
{
    constexpr wchar_t kEnrollmentBlobMagic[] = L"MCP1";
    constexpr wchar_t kEnrollmentBlobDescription[] = L"MosaicEnrollment";

    HRESULT ComputeSha256(const BYTE* data, ULONG cbData, std::vector<BYTE>& hash)
    {
        hash.clear();

        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        DWORD cbObject = 0;
        DWORD cbResult = 0;
        DWORD cbHash = 0;
        std::vector<BYTE> hashObject;

        NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        if (!BCRYPT_SUCCESS(status)) {
            return HRESULT_FROM_NT(status);
        }

        status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&cbObject), sizeof(cbObject), &cbResult, 0);
        if (!BCRYPT_SUCCESS(status)) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return HRESULT_FROM_NT(status);
        }

        status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&cbHash), sizeof(cbHash), &cbResult, 0);
        if (!BCRYPT_SUCCESS(status)) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return HRESULT_FROM_NT(status);
        }

        hashObject.resize(cbObject);
        hash.resize(cbHash);

        status = BCryptCreateHash(hAlg, &hHash, hashObject.data(), cbObject, nullptr, 0, 0);
        if (!BCRYPT_SUCCESS(status)) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return HRESULT_FROM_NT(status);
        }

        status = BCryptHashData(hHash, const_cast<PUCHAR>(data), cbData, 0);
        if (!BCRYPT_SUCCESS(status)) {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return HRESULT_FROM_NT(status);
        }

        status = BCryptFinishHash(hHash, hash.data(), cbHash, 0);
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        if (!BCRYPT_SUCCESS(status)) {
            return HRESULT_FROM_NT(status);
        }

        return S_OK;
    }

    void XorWithKey(std::vector<BYTE>& data, const std::vector<BYTE>& key)
    {
        if (key.empty()) {
            return;
        }

        for (size_t i = 0; i < data.size(); ++i) {
            data[i] ^= key[i % key.size()];
        }
    }
}

HRESULT CreateProtectedEnrollmentBlob(
    const std::wstring& sid,
    const std::wstring& password,
    const std::wstring& normalizedPattern,
    std::vector<BYTE>& protectedBlob)
{
    protectedBlob.clear();
    if (password.empty() || normalizedPattern.empty()) {
        return E_INVALIDARG;
    }

    std::wstring keyMaterial = normalizedPattern;
    keyMaterial.push_back(L'|');
    keyMaterial.append(sid);

    std::vector<BYTE> patternKey;
    HRESULT hr = ComputeSha256(
        reinterpret_cast<const BYTE*>(keyMaterial.data()),
        static_cast<ULONG>(keyMaterial.size() * sizeof(wchar_t)),
        patternKey);
    if (FAILED(hr)) {
        return hr;
    }

    const DWORD cbPassword = static_cast<DWORD>(password.size() * sizeof(wchar_t));
    const DWORD cbMagic = sizeof(kEnrollmentBlobMagic);
    std::vector<BYTE> payload(sizeof(DWORD) * 2 + cbMagic + cbPassword);
    BYTE* cursor = payload.data();

    memcpy(cursor, &cbMagic, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(cursor, &cbPassword, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(cursor, kEnrollmentBlobMagic, cbMagic);
    cursor += cbMagic;
    memcpy(cursor, password.data(), cbPassword);

    XorWithKey(payload, patternKey);

    DATA_BLOB plainBlob{};
    plainBlob.pbData = payload.data();
    plainBlob.cbData = static_cast<DWORD>(payload.size());

    DATA_BLOB protectedData{};
    if (!CryptProtectData(&plainBlob, kEnrollmentBlobDescription, nullptr, nullptr, nullptr, 0, &protectedData)) {
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    protectedBlob.assign(protectedData.pbData, protectedData.pbData + protectedData.cbData);
    LocalFree(protectedData.pbData);
    SecureZeroMemory(payload.data(), payload.size());
    return S_OK;
}

HRESULT RecoverPasswordFromProtectedBlob(
    const std::wstring& sid,
    const std::wstring& normalizedPattern,
    const std::vector<BYTE>& protectedBlob,
    std::wstring& passwordOut)
{
    passwordOut.clear();
    if (normalizedPattern.empty() || protectedBlob.empty()) {
        return E_INVALIDARG;
    }

    std::wstring keyMaterial = normalizedPattern;
    keyMaterial.push_back(L'|');
    keyMaterial.append(sid);

    std::vector<BYTE> patternKey;
    HRESULT hr = ComputeSha256(
        reinterpret_cast<const BYTE*>(keyMaterial.data()),
        static_cast<ULONG>(keyMaterial.size() * sizeof(wchar_t)),
        patternKey);
    if (FAILED(hr)) {
        return hr;
    }

    DATA_BLOB encryptedBlob{};
    encryptedBlob.pbData = const_cast<BYTE*>(protectedBlob.data());
    encryptedBlob.cbData = static_cast<DWORD>(protectedBlob.size());

    DATA_BLOB plainBlob{};
    if (!CryptUnprotectData(&encryptedBlob, nullptr, nullptr, nullptr, nullptr, 0, &plainBlob)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::vector<BYTE> payload(plainBlob.pbData, plainBlob.pbData + plainBlob.cbData);
    LocalFree(plainBlob.pbData);
    XorWithKey(payload, patternKey);

    if (payload.size() < (sizeof(DWORD) * 2)) {
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    const BYTE* cursor = payload.data();
    DWORD cbMagic = 0;
    DWORD cbPassword = 0;
    memcpy(&cbMagic, cursor, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(&cbPassword, cursor, sizeof(DWORD));
    cursor += sizeof(DWORD);

    const size_t headerSize = sizeof(DWORD) * 2;
    if (cbMagic != sizeof(kEnrollmentBlobMagic) || payload.size() < headerSize + cbMagic + cbPassword) {
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    if (memcmp(cursor, kEnrollmentBlobMagic, cbMagic) != 0) {
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD);
    }
    cursor += cbMagic;

    if ((cbPassword % sizeof(wchar_t)) != 0) {
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    passwordOut.assign(reinterpret_cast<const wchar_t*>(cursor), cbPassword / sizeof(wchar_t));
    SecureZeroMemory(payload.data(), payload.size());
    return S_OK;
}
