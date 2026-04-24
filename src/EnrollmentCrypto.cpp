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
    constexpr DWORD kPayloadVersionPbkdf2 = 2;
    constexpr DWORD kPbkdf2SaltLength = 32;
    constexpr DWORD kPbkdf2Iterations = 200000;

    HRESULT DerivePatternKeyPbkdf2(
        const std::wstring& sid,
        const std::wstring& normalizedPattern,
        const BYTE* salt,
        DWORD cbSalt,
        DWORD iterations,
        std::vector<BYTE>& key)
    {
        if (salt == nullptr || cbSalt == 0 || iterations == 0 || key.empty()) {
            return E_INVALIDARG;
        }

        std::wstring secretMaterial = normalizedPattern;
        secretMaterial.push_back(L'|');
        secretMaterial.append(sid);

        BCRYPT_ALG_HANDLE hAlg = nullptr;
        NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
        if (!BCRYPT_SUCCESS(status)) {
            return HRESULT_FROM_NT(status);
        }

        status = BCryptDeriveKeyPBKDF2(
            hAlg,
            reinterpret_cast<PUCHAR>(secretMaterial.data()),
            static_cast<ULONG>(secretMaterial.size() * sizeof(wchar_t)),
            const_cast<PUCHAR>(salt),
            cbSalt,
            iterations,
            key.data(),
            static_cast<ULONG>(key.size()),
            0);
        SecureZeroMemory(secretMaterial.data(), secretMaterial.size() * sizeof(wchar_t));
        BCryptCloseAlgorithmProvider(hAlg, 0);
        if (!BCRYPT_SUCCESS(status)) {
            return HRESULT_FROM_NT(status);
        }

        return S_OK;
    }

    HRESULT ExtractPasswordFromPayload(
        const BYTE* payloadData,
        size_t payloadSize,
        std::wstring& passwordOut)
    {
        if (payloadData == nullptr || payloadSize < (sizeof(DWORD) * 2)) {
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        const BYTE* cursor = payloadData;
        DWORD cbMagic = 0;
        DWORD cbPassword = 0;
        memcpy(&cbMagic, cursor, sizeof(DWORD));
        cursor += sizeof(DWORD);
        memcpy(&cbPassword, cursor, sizeof(DWORD));
        cursor += sizeof(DWORD);

        const size_t headerSize = sizeof(DWORD) * 2;
        if (cbMagic != sizeof(kEnrollmentBlobMagic) || payloadSize < headerSize + cbMagic + cbPassword) {
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (memcmp(cursor, kEnrollmentBlobMagic, cbMagic) != 0) {
            return HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD);
        }
        cursor += cbMagic;

        if ((cbPassword % sizeof(wchar_t)) != 0) {
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        passwordOut.assign(reinterpret_cast<const wchar_t*>(cursor), cbPassword / sizeof(wchar_t));
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

    std::vector<BYTE> salt(kPbkdf2SaltLength);
    NTSTATUS status = BCryptGenRandom(nullptr, salt.data(), static_cast<ULONG>(salt.size()), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) {
        return HRESULT_FROM_NT(status);
    }

    std::vector<BYTE> patternKey(32);
    HRESULT hr = DerivePatternKeyPbkdf2(sid, normalizedPattern, salt.data(), static_cast<DWORD>(salt.size()), kPbkdf2Iterations, patternKey);
    if (FAILED(hr)) {
        SecureZeroMemory(salt.data(), salt.size());
        return hr;
    }

    const DWORD cbPassword = static_cast<DWORD>(password.size() * sizeof(wchar_t));
    const DWORD cbMagic = sizeof(kEnrollmentBlobMagic);
    std::vector<BYTE> secretData(sizeof(DWORD) * 2 + cbMagic + cbPassword);
    BYTE* secretCursor = secretData.data();

    memcpy(secretCursor, &cbMagic, sizeof(DWORD));
    secretCursor += sizeof(DWORD);
    memcpy(secretCursor, &cbPassword, sizeof(DWORD));
    secretCursor += sizeof(DWORD);
    memcpy(secretCursor, kEnrollmentBlobMagic, cbMagic);
    secretCursor += cbMagic;
    memcpy(secretCursor, password.data(), cbPassword);

    XorWithKey(secretData, patternKey);

    const DWORD cbSecretData = static_cast<DWORD>(secretData.size());
    std::vector<BYTE> payload(sizeof(DWORD) * 4 + salt.size() + secretData.size());
    BYTE* cursor = payload.data();
    memcpy(cursor, &kPayloadVersionPbkdf2, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(cursor, &kPbkdf2Iterations, sizeof(DWORD));
    cursor += sizeof(DWORD);
    const DWORD cbSalt = static_cast<DWORD>(salt.size());
    memcpy(cursor, &cbSalt, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(cursor, &cbSecretData, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(cursor, salt.data(), salt.size());
    cursor += salt.size();
    memcpy(cursor, secretData.data(), secretData.size());

    DATA_BLOB plainBlob{};
    plainBlob.pbData = payload.data();
    plainBlob.cbData = static_cast<DWORD>(payload.size());

    DATA_BLOB protectedData{};
    if (!CryptProtectData(&plainBlob, kEnrollmentBlobDescription, nullptr, nullptr, nullptr, 0, &protectedData)) {
        SecureZeroMemory(secretData.data(), secretData.size());
        SecureZeroMemory(salt.data(), salt.size());
        SecureZeroMemory(patternKey.data(), patternKey.size());
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    protectedBlob.assign(protectedData.pbData, protectedData.pbData + protectedData.cbData);
    LocalFree(protectedData.pbData);
    SecureZeroMemory(secretData.data(), secretData.size());
    SecureZeroMemory(salt.data(), salt.size());
    SecureZeroMemory(patternKey.data(), patternKey.size());
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

    DATA_BLOB encryptedBlob{};
    encryptedBlob.pbData = const_cast<BYTE*>(protectedBlob.data());
    encryptedBlob.cbData = static_cast<DWORD>(protectedBlob.size());

    DATA_BLOB plainBlob{};
    if (!CryptUnprotectData(&encryptedBlob, nullptr, nullptr, nullptr, nullptr, 0, &plainBlob)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::vector<BYTE> payload(plainBlob.pbData, plainBlob.pbData + plainBlob.cbData);
    LocalFree(plainBlob.pbData);

    if (payload.size() < (sizeof(DWORD) * 4)) {
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    const BYTE* cursor = payload.data();
    DWORD version = 0;
    DWORD iterations = 0;
    DWORD cbSalt = 0;
    DWORD cbSecretData = 0;

    memcpy(&version, cursor, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(&iterations, cursor, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(&cbSalt, cursor, sizeof(DWORD));
    cursor += sizeof(DWORD);
    memcpy(&cbSecretData, cursor, sizeof(DWORD));
    cursor += sizeof(DWORD);

    const size_t headerSize = sizeof(DWORD) * 4;
    if (version != kPayloadVersionPbkdf2 ||
        iterations < 100000 ||
        (cbSalt != 16 && cbSalt != 32) ||
        cbSecretData == 0 ||
        payload.size() < headerSize + cbSalt + cbSecretData) {
        SecureZeroMemory(payload.data(), payload.size());
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    const BYTE* salt = cursor;
    const BYTE* encryptedSecretData = cursor + cbSalt;
    std::vector<BYTE> secretData(encryptedSecretData, encryptedSecretData + cbSecretData);
    std::vector<BYTE> patternKey(32);

    HRESULT hr = DerivePatternKeyPbkdf2(sid, normalizedPattern, salt, cbSalt, iterations, patternKey);
    if (SUCCEEDED(hr)) {
        XorWithKey(secretData, patternKey);
        hr = ExtractPasswordFromPayload(secretData.data(), secretData.size(), passwordOut);
    }

    SecureZeroMemory(patternKey.data(), patternKey.size());
    SecureZeroMemory(secretData.data(), secretData.size());
    SecureZeroMemory(payload.data(), payload.size());
    return hr;
}
