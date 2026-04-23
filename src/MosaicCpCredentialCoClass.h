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
// MosaicCpCredentialCoClass.h : Declaration of the CMosaicCredentialProviderCredential

#pragma once
#include "resource.h"       // main symbols
#include "CredentialState.h"
#include "PatternCredProv_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;
#include <string>

// CMosaicCredentialProviderCredential

class ATL_NO_VTABLE CMosaicCredentialProviderCredential :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ICredentialProviderCredential2
{
public:
    CMosaicCredentialProviderCredential() : m_hwndParent(nullptr), 
        m_pEvents(nullptr), 
        m_userSid(L""),
        m_cpState(CredentialProviderState::CPSTATE_INITIAL)
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_MOSAICCREDENTIALPROVIDER);

    BEGIN_COM_MAP(CMosaicCredentialProviderCredential)
        COM_INTERFACE_ENTRY(ICredentialProviderCredential2)
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
    void SetUserData(const std::wstring& userSid);

    // Inherited via ICredentialProviderCredential2
    HRESULT __stdcall Advise(
        ICredentialProviderCredentialEvents* pcpce) override;
    HRESULT __stdcall UnAdvise(void) override;
    HRESULT __stdcall SetSelected(BOOL* pbAutoLogon) override;
    HRESULT __stdcall SetDeselected(void) override;
    HRESULT __stdcall GetFieldState(
        DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
        CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis) override;
    HRESULT __stdcall GetStringValue(DWORD dwFieldID,
        LPWSTR* ppsz) override;
    HRESULT __stdcall GetBitmapValue(DWORD dwFieldID,
        HBITMAP* phbmp) override;
    HRESULT __stdcall GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked,
        LPWSTR* ppszLabel) override;
    HRESULT __stdcall GetSubmitButtonValue(DWORD dwFieldID,
        DWORD* pdwAdjacentTo) override;
    HRESULT __stdcall GetComboBoxValueCount(
        DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem) override;
    HRESULT __stdcall GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem,
        LPWSTR* ppszItem) override;
    HRESULT __stdcall SetStringValue(DWORD dwFieldID, LPCWSTR psz) override;
    HRESULT __stdcall SetCheckboxValue(DWORD dwFieldID,
        BOOL bChecked) override;
    HRESULT __stdcall SetComboBoxSelectedValue(
        DWORD dwFieldID, DWORD dwSelectedItem) override;
    HRESULT __stdcall CommandLinkClicked(DWORD dwFieldID) override;
    HRESULT __stdcall GetSerialization(
        CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
        CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
        LPWSTR* ppszOptionalStatusText,
        CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) override;
    HRESULT __stdcall ReportResult(
        NTSTATUS ntsStatus, NTSTATUS ntsSubstatus,
        LPWSTR* ppszOptionalStatusText,
        CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) override;
    HRESULT __stdcall GetUserSid(LPWSTR* sid) override;

private:
    HRESULT RunEnrollmentSetup();
    HRESULT RunResetMosaic();
    HRESULT CaptureEnrollmentPattern(LPCWSTR pszTitle, LPCWSTR pszPrompt, std::wstring& patternOut);
    HRESULT RunPatternProvisioningFlow(
        CredentialProviderState firstPatternState,
        CredentialProviderState confirmPatternState,
        LPCWSTR passwordPrompt,
        LPCWSTR firstPatternPrompt,
        LPCWSTR confirmPatternPrompt,
        LPCWSTR mismatchMessage,
        LPCWSTR successMessage);
    void ResetPasswordCollectionState(CredentialProviderState passwordState);

    std::wstring m_userSid;
    std::wstring m_enrollmentPassword;
    std::wstring m_firstEnrollmentPattern;
    CComPtr<ICredentialProviderCredentialEvents> m_pEvents;
    HWND m_hwndParent;
    CredentialProviderState m_cpState;
};


