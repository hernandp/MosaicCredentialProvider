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
// MosaicCpCoClass.h : Declaration of the CMosaicCredentialProvider

#pragma once
#include "resource.h"       // main symbols
#include "PatternCredProv_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif
#include <vector>
#include <string>

using namespace ATL;

// CMosaicCredentialProvider

class ATL_NO_VTABLE CMosaicCredentialProvider :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CMosaicCredentialProvider, &CLSID_PatternCredentialProvider>,
    public ICredentialProvider,    
    public ICredentialProviderSetUserArray
{
public:
    CMosaicCredentialProvider()
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_MOSAICCREDENTIALPROVIDER)

    BEGIN_COM_MAP(CMosaicCredentialProvider)
        COM_INTERFACE_ENTRY(ICredentialProvider)
        COM_INTERFACE_ENTRY(ICredentialProviderSetUserArray)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

public:

    // Inherited via ICredentialProvider
    HRESULT __stdcall SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags) override;

    HRESULT __stdcall SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs) override;

    HRESULT __stdcall Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext) override;

    HRESULT __stdcall UnAdvise(void) override;

    HRESULT __stdcall GetFieldDescriptorCount(DWORD* pdwCount) override;

    HRESULT __stdcall GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd) override;

    HRESULT __stdcall GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault) override;

    HRESULT __stdcall GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc) override;

    // Inherited via ICredentialProviderSetUserArray
    HRESULT __stdcall SetUserArray(
        ICredentialProviderUserArray *users) override;

private:
    std::vector<std::wstring>                               m_userSids;
    std::vector<CComPtr<ICredentialProviderCredential2>>    m_pCredentials;
};

OBJECT_ENTRY_AUTO(__uuidof(PatternCredentialProvider), CMosaicCredentialProvider)
