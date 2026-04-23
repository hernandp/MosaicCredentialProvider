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
// MosaicCpCoClass.cpp : Implementation of CMosaicCredentialProvider

#include "pch.h"
#include "MosaicCpCoClass.h"
#include "MosaicCpCredentialCoClass.h"
#include "ProductConfig.h"
#include "dprintf.h"
#include "FieldDescriptors.h"
#pragma comment(lib, "Credui.lib")

// ICredentialProvider methods


HRESULT __stdcall CMosaicCredentialProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
{
    // Check if we are enabled.

    CRegKey cpConfig;
    DWORD dwEnabled{ 0 };
    LSTATUS ls = cpConfig.Open(HKEY_LOCAL_MACHINE, g_wszProductKeyPath, KEY_READ);
    if (ls != ERROR_SUCCESS) {
        dprintfW(L"Failed to open configuration registry key, LSTATUS = 0x%08x\n", ls);
        return E_NOTIMPL;
    }

    ls = cpConfig.QueryDWORDValue(L"Enabled", dwEnabled);
    if (dwEnabled == 0) {
        dprintfW(L"Disabled via registry or key absent. ls=%d, dwEnabled=%d", ls, dwEnabled);
        return E_NOTIMPL;
    }

    dprintfW(L"SetUsageScenario called with cpus=%d, dwFlags=0x%08x\n", cpus, dwFlags);

    return (cpus == CPUS_UNLOCK_WORKSTATION || cpus == CPUS_LOGON) ? S_OK : E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProvider::SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
    dprintfW(L"SetSerialization called with pcpcs=%p\n", pcpcs);
    UNREFERENCED_PARAMETER(pcpcs);
    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProvider::Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext)
{
    dprintfW(L"Advise called with pcpe=%p, upAdviseContext=0x%p\n", pcpe, reinterpret_cast<void*>(upAdviseContext));
    
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProvider::UnAdvise(void)
{
    dprintfW(L"UnAdvise called\n");
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProvider::GetFieldDescriptorCount(DWORD* pdwCount)
{
    dprintfW(L"GetFieldDescriptorCount called\n");

    *pdwCount = CP_NUM_FIELDS;
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    dprintfW(L"GetFieldDescriptorAt called with dwIndex=%d\n", dwIndex);

    if (ppcpfd == nullptr) {
        return E_POINTER;
    }

    if (dwIndex >= CP_NUM_FIELDS) {
        return E_INVALIDARG;
    }
    
    // pcpfd and the labels will be freed by the caller (LogonUI) via CoTaskMemFree.

    auto pcpfd = reinterpret_cast<CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*>(CoTaskMemAlloc(sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR)));
    if (!pcpfd) {
        dprintfW(L"CoTaskMemAlloc failed with error: %d\n", GetLastError());
        return E_OUTOFMEMORY;
    }

    auto fieldDesc = g_fieldDescriptors[dwIndex];

    if (SUCCEEDED(SHStrDupW(fieldDesc.pszLabel, &pcpfd->pszLabel)))
    {
        pcpfd->cpft = fieldDesc.cpft;
        pcpfd->guidFieldType = fieldDesc.guidFieldType;
        pcpfd->dwFieldID = fieldDesc.dwFieldID;
    }
    else {
        dprintfW(L"SHStrDupW failed with error: %d\n", GetLastError());
        CoTaskMemFree(pcpfd);
        return E_OUTOFMEMORY;
    }
    
    (*ppcpfd) = pcpfd;
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
{
    dprintfW(L"GetCredentialCount called\n");
    *pdwCount = static_cast<DWORD>(m_userSids.size());
    *pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
    *pbAutoLogonWithDefault = FALSE;
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    HRESULT hr{ S_OK };
    dprintfW(L"GetCredentialAt called with dwIndex=%d\n", dwIndex);

    if (!ppcpc)
        return E_POINTER;

    if (dwIndex >= static_cast<DWORD> ( m_userSids.size())) {
        return E_INVALIDARG;
    }

    if (!m_pCredentials[dwIndex]) {
        // lazy init 
        CComObject<CMosaicCredentialProviderCredential>* pCredential = nullptr;
        if (SUCCEEDED(hr = CComObject<CMosaicCredentialProviderCredential>::CreateInstance(&pCredential))) {
            pCredential->AddRef(); // AddRef because CComObject<T>::CreateInstance returns refcount 0!
            if (!m_userSids.empty()) {
                pCredential->SetUserData(m_userSids[dwIndex]);
            }
            m_pCredentials[dwIndex].Attach(pCredential);
            dprintfW(L"Created credential for index %d\n", dwIndex);
        }
        else
        {
            dprintfW(L"Failed to create credential instance with error: %d\n", GetLastError());
            return hr;
        }
    }
    
    (*ppcpc) = m_pCredentials[dwIndex];
    (*ppcpc)->AddRef();
    return hr;
}

HRESULT __stdcall CMosaicCredentialProvider::SetUserArray(ICredentialProviderUserArray *users) 
{
    dprintfW(L"SetUserArray called with users=%p\n", users);

    if (users == nullptr) {
        return E_POINTER;
    }

    m_userSids.clear();
    m_pCredentials.clear();

    DWORD dwUserCount{ 0 };
    HRESULT hr = users->GetCount(&dwUserCount);
    if (FAILED(hr)) {
        dprintfW(L"SetUserArray: GetCount failed with error: %d\n", GetLastError());
    }

    for (DWORD i = 0; i < dwUserCount; ++i) {
        CComPtr<ICredentialProviderUser> pUser;
        hr = users->GetAt(i, &pUser);
        if (FAILED(hr)) {
            dprintfW(L"SetUserArray: GetAt(%d) failed with error: %d\n", i, GetLastError());
            continue;
        }
        
        LPWSTR pszSid = nullptr;
        hr = pUser->GetSid(&pszSid);
        if (FAILED(hr)) {
            dprintfW(L"SetUserArray: GetSid(%d) failed with error: %d\n", i, GetLastError());
            continue;
        }
        dprintfW(L"User %d SID: %s\n", i, pszSid);
        
        m_userSids.emplace_back(std::wstring(pszSid));
        m_pCredentials.emplace_back(nullptr); // placeholder for lazy init of credentials in GetCredentialAt
        CoTaskMemFree(pszSid);
    }

    return hr;
}

