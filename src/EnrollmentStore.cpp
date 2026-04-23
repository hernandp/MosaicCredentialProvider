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
#include "EnrollmentStore.h"
#include "ProductConfig.h"
#include "dprintf.h"

namespace
{
    HRESULT GetUserEnrollmentKey(const std::wstring& sid, std::wstring& strOut)
    {
        strOut = std::wstring(g_wszProductKeyPath).append(L"\\Enrollment\\").append(sid);
        return S_OK;
    }

    HRESULT CreateDefaultEnrollmentInfo(const std::wstring& sid)
    {
        std::wstring enrollmentKey;
        HRESULT hr = GetUserEnrollmentKey(sid, enrollmentKey);
        if (FAILED(hr)) {
            return hr;
        }

        PSECURITY_DESCRIPTOR pSD = nullptr;
        LPCWSTR sddl =
            L"D:"
            L"(A;OICI;KA;;;SY)"
            L"(A;OICI;KA;;;BA)";

        DWORD err = ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl,
            SDDL_REVISION_1,
            &pSD,
            nullptr);

        if (err == 0) {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        ATL::CRegKey cpConfig;
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

        LSTATUS ls = cpConfig.Create(HKEY_LOCAL_MACHINE, enrollmentKey.c_str(), 0, 0, KEY_WRITE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY, &sa);
        if (ls != ERROR_SUCCESS) {
            dprintfW(L"Failed to create configuration registry key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
            if (pSD != nullptr) {
                LocalFree(pSD);
            }
            return HRESULT_FROM_WIN32(ls);
        }

        if (pSD != nullptr) {
            LocalFree(pSD);
        }

        ls = cpConfig.SetDWORDValue(L"Enabled", 0);
        if (ls != ERROR_SUCCESS) {
            dprintfW(L"Failed to set configuration registry value, LSTATUS = 0x%08x\n", ls);
            return HRESULT_FROM_WIN32(ls);
        }

        dprintfW(L"Enrollment registry entries created successfully\n");
        return S_OK;
    }
}

HRESULT GetEnrollmentInfo(std::wstring sid, UserEnrollmentInfo& outInfo)
{
    outInfo = UserEnrollmentInfo{};

    std::wstring enrollmentKey;
    HRESULT hr = GetUserEnrollmentKey(sid, enrollmentKey);
    if (FAILED(hr)) {
        return hr;
    }

    ATL::CRegKey cpConfig;
    LSTATUS ls = cpConfig.Open(HKEY_LOCAL_MACHINE, enrollmentKey.c_str());
    if (ls != ERROR_SUCCESS) {
        if (ls == ERROR_FILE_NOT_FOUND) {
            dprintfW(L"User enrollment key %s not found, assuming not enrolled\n", enrollmentKey.c_str());
            return S_FALSE;
        }

        dprintfW(L"Failed to open configuration registry key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
        return HRESULT_FROM_WIN32(ls);
    }

    ls = cpConfig.QueryDWORDValue(L"Enabled", outInfo.dwEnabled);
    if (ls != ERROR_SUCCESS) {
        dprintfW(L"Failed to query Enabled in enrollment key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
        return HRESULT_FROM_WIN32(ls);
    }

    ULONG cbData = 0;
    ls = cpConfig.QueryBinaryValue(L"ProtectedBlob", nullptr, &cbData);
    if (ls == ERROR_SUCCESS && cbData > 0) {
        outInfo.protectedBlob.resize(cbData);
        ls = cpConfig.QueryBinaryValue(L"ProtectedBlob", outInfo.protectedBlob.data(), &cbData);
        if (ls != ERROR_SUCCESS) {
            dprintfW(L"Failed to query ProtectedBlob in enrollment key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
            return HRESULT_FROM_WIN32(ls);
        }
    }
    else if (ls != ERROR_FILE_NOT_FOUND && ls != ERROR_SUCCESS) {
        dprintfW(L"Failed to query ProtectedBlob size in enrollment key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
        return HRESULT_FROM_WIN32(ls);
    }

    return S_OK;
}

HRESULT SaveEnrollmentInfo(std::wstring sid, const std::vector<BYTE>& protectedBlob)
{
    if (protectedBlob.empty()) {
        return E_INVALIDARG;
    }

    HRESULT hr = CreateDefaultEnrollmentInfo(sid);
    if (FAILED(hr)) {
        return hr;
    }

    std::wstring enrollmentKey;
    hr = GetUserEnrollmentKey(sid, enrollmentKey);
    if (FAILED(hr)) {
        return hr;
    }

    ATL::CRegKey cpConfig;
    LSTATUS ls = cpConfig.Open(HKEY_LOCAL_MACHINE, enrollmentKey.c_str(), KEY_WRITE | KEY_SET_VALUE);
    if (ls != ERROR_SUCCESS) {
        dprintfW(L"Failed to open configuration registry key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
        return HRESULT_FROM_WIN32(ls);
    }

    ls = cpConfig.SetBinaryValue(L"ProtectedBlob", protectedBlob.data(), static_cast<ULONG>(protectedBlob.size()));
    if (ls != ERROR_SUCCESS) {
        dprintfW(L"Failed to set ProtectedBlob value in key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
        return HRESULT_FROM_WIN32(ls);
    }

    ls = cpConfig.SetDWORDValue(L"Enabled", 1);
    if (ls != ERROR_SUCCESS) {
        dprintfW(L"Failed to set Enabled value in key (%s), LSTATUS = 0x%08x\n", enrollmentKey.c_str(), ls);
        return HRESULT_FROM_WIN32(ls);
    }

    dprintfW(L"Enrollment registry entries committed successfully for SID %s\n", sid.c_str());
    return S_OK;
}
