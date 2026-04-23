// ----------------------------------------------------------------------------------------------------------------
// Copyright 2026 Hernán Di Pietro
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the “Software”), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
// to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ----------------------------------------------------------------------------------------------------------------
#include "pch.h"
#include "WindowsAuth.h"

#include "PatternCredProv_i.h"

#include <ntsecapi.h>
#include <vector>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "credui.lib")
#pragma comment(lib, "secur32.lib")

HRESULT ResolveAccountFromSid(const std::wstring& sid, std::wstring& userName, std::wstring& domainName)
{
    userName.clear();
    domainName.clear();

    PSID pSid = nullptr;
    if (!ConvertStringSidToSidW(sid.c_str(), &pSid)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD cchName = 0;
    DWORD cchDomain = 0;
    SID_NAME_USE sidType = SidTypeUnknown;
    LookupAccountSidW(nullptr, pSid, nullptr, &cchName, nullptr, &cchDomain, &sidType);
    const DWORD lookupError = GetLastError();
    if (lookupError != ERROR_INSUFFICIENT_BUFFER) {
        LocalFree(pSid);
        return HRESULT_FROM_WIN32(lookupError);
    }

    std::vector<wchar_t> nameBuffer(cchName);
    std::vector<wchar_t> domainBuffer(cchDomain);
    if (!LookupAccountSidW(nullptr, pSid, nameBuffer.data(), &cchName, domainBuffer.data(), &cchDomain, &sidType)) {
        const DWORD dwErr = GetLastError();
        LocalFree(pSid);
        return HRESULT_FROM_WIN32(dwErr);
    }

    LocalFree(pSid);
    userName.assign(nameBuffer.data());
    domainName.assign(domainBuffer.data());
    return S_OK;
}

HRESULT ValidateWindowsPassword(const std::wstring& sid, const std::wstring& password)
{
    if (password.empty()) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD);
    }

    std::wstring userName;
    std::wstring domainName;
    HRESULT hr = ResolveAccountFromSid(sid, userName, domainName);
    if (FAILED(hr)) {
        return hr;
    }

    HANDLE hToken = nullptr;
    const BOOL ok = LogonUserW(
        userName.c_str(),
        domainName.empty() ? nullptr : domainName.c_str(),
        password.c_str(),
        LOGON32_LOGON_INTERACTIVE,
        LOGON32_PROVIDER_DEFAULT,
        &hToken);

    if (!ok) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    CloseHandle(hToken);
    return S_OK;
}

HRESULT RetrieveNegotiateAuthPackage(ULONG* pulAuthPackage)
{
    static constexpr char kNegotiatePackageName[] = "Negotiate";

    if (pulAuthPackage == nullptr) {
        return E_POINTER;
    }

    HANDLE hLsa = nullptr;
    NTSTATUS status = LsaConnectUntrusted(&hLsa);
    if (status < 0) {
        return HRESULT_FROM_NT(status);
    }

    LSA_STRING packageName{};
    packageName.Buffer = const_cast<PCHAR>(kNegotiatePackageName);
    packageName.Length = static_cast<USHORT>(sizeof(kNegotiatePackageName) - 1);
    packageName.MaximumLength = packageName.Length + 1;

    ULONG authPackage = 0;
    status = LsaLookupAuthenticationPackage(hLsa, &packageName, &authPackage);
    LsaDeregisterLogonProcess(hLsa);
    if (status < 0) {
        return HRESULT_FROM_NT(status);
    }

    *pulAuthPackage = authPackage;
    return S_OK;
}

HRESULT BuildSerializationForCurrentUser(
    const std::wstring& sid,
    const std::wstring& password,
    CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
    std::wstring userName;
    std::wstring domainName;
    HRESULT hr = ResolveAccountFromSid(sid, userName, domainName);
    if (FAILED(hr)) {
        return hr;
    }

    const std::wstring qualifiedUser = domainName.empty() ? userName : (domainName + L"\\" + userName);

    DWORD cbSerialization = 0;
    CredPackAuthenticationBufferW(
        0,
        const_cast<LPWSTR>(qualifiedUser.c_str()),
        const_cast<LPWSTR>(password.c_str()),
        nullptr,
        &cbSerialization);

    const DWORD dwPackErr = GetLastError();
    if (dwPackErr != ERROR_INSUFFICIENT_BUFFER) {
        return HRESULT_FROM_WIN32(dwPackErr);
    }

    BYTE* rgbSerialization = static_cast<BYTE*>(CoTaskMemAlloc(cbSerialization));
    if (rgbSerialization == nullptr) {
        return E_OUTOFMEMORY;
    }

    if (!CredPackAuthenticationBufferW(
            0,
            const_cast<LPWSTR>(qualifiedUser.c_str()),
            const_cast<LPWSTR>(password.c_str()),
            rgbSerialization,
            &cbSerialization)) {
        const HRESULT hrPack = HRESULT_FROM_WIN32(GetLastError());
        CoTaskMemFree(rgbSerialization);
        return hrPack;
    }

    ULONG authPackage = 0;
    hr = RetrieveNegotiateAuthPackage(&authPackage);
    if (FAILED(hr)) {
        SecureZeroMemory(rgbSerialization, cbSerialization);
        CoTaskMemFree(rgbSerialization);
        return hr;
    }

    pcpcs->ulAuthenticationPackage = authPackage;
    pcpcs->clsidCredentialProvider = CLSID_PatternCredentialProvider;
    pcpcs->cbSerialization = cbSerialization;
    pcpcs->rgbSerialization = rgbSerialization;
    *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
    return S_OK;
}
