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
#include "MosaicCpCredentialCoClass.h"
#include "CredentialFlow.h"
#include "EnrollmentCrypto.h"
#include "EnrollmentStore.h"
#include "FieldDescriptors.h"
#include "ProductConfig.h"
#include "VerifyDialog.h"
#include "WindowsAuth.h"
#include "dprintf.h"

#include <vector>

HRESULT CMosaicCredentialProviderCredential::CaptureEnrollmentPattern(LPCWSTR pszTitle, LPCWSTR pszPrompt, std::wstring& patternOut)
{
    MosaicPatternResult result;
    const HRESULT hr = OpenVerifyDialog(m_hwndParent, m_userSid.c_str(), pszTitle, pszPrompt, result);
    if (SUCCEEDED(hr)) {
        patternOut = result.normalizedPattern;
    }
    return hr;
}

void CMosaicCredentialProviderCredential::ResetPasswordCollectionState(CredentialProviderState passwordState)
{
    SecureClearString(m_enrollmentPassword);
    SecureClearString(m_firstEnrollmentPattern);
    m_cpState = passwordState;

    if (passwordState == CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD) {
        SetEnterPasswordUIState(m_pEvents, this, L"Enter your Windows password to begin mosaic enrollment.");
    }
    else if (passwordState == CredentialProviderState::CPSTATE_RESET_PASSWORD) {
        SetEnterPasswordUIState(m_pEvents, this, L"Enter your Windows password to reset your mosaic pattern.");
    }
}

//
// Shared provisioning flow used by both first-time enrollment and mosaic reset.
// - Validate the current Windows password for the selected user.
// - Open the mosaic dialog and capture the first pattern.
// - Open the mosaic dialog again and capture the confirmation pattern.
// - Compare both captured patterns and reject the flow if they differ.
// - Derive the protected enrollment blob from the password and the pattern.
// - Persist the protected blob to the per-user enrollment store.
// - If everything succeeds, switch the credential back to the ready-for-login state.
//
HRESULT CMosaicCredentialProviderCredential::RunPatternProvisioningFlow(
    CredentialProviderState firstPatternState,
    CredentialProviderState confirmPatternState,
    LPCWSTR passwordPrompt,
    LPCWSTR firstPatternPrompt,
    LPCWSTR confirmPatternPrompt,
    LPCWSTR mismatchMessage,
    LPCWSTR successMessage)
{
    UNREFERENCED_PARAMETER(passwordPrompt);

    HRESULT hr = ValidateWindowsPassword(m_userSid, m_enrollmentPassword);
    if (FAILED(hr)) {
        ResetPasswordCollectionState(
            firstPatternState == CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST
                ? CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD
                : CredentialProviderState::CPSTATE_RESET_PASSWORD);
        MessageBoxW(m_hwndParent, L"The password is not valid for this Windows account.", g_wszProductName, MB_OK | MB_ICONERROR);
        return hr;
    }

    m_cpState = firstPatternState;
    if (m_pEvents != nullptr) {
        m_pEvents->SetFieldString(this, CP_FIELD_EXPLAIN, GetExplainTextForState(firstPatternState, false, 0));
    }

    hr = CaptureEnrollmentPattern(g_wszProductName, firstPatternPrompt, m_firstEnrollmentPattern);
    if (FAILED(hr)) {
        ResetPasswordCollectionState(
            firstPatternState == CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST
                ? CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD
                : CredentialProviderState::CPSTATE_RESET_PASSWORD);
        if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            MessageBoxW(m_hwndParent, L"Unable to capture the first mosaic pattern.", g_wszProductName, MB_OK | MB_ICONERROR);
        }
        return hr;
    }

    m_cpState = confirmPatternState;
    if (m_pEvents != nullptr) {
        m_pEvents->SetFieldString(this, CP_FIELD_EXPLAIN, GetExplainTextForState(confirmPatternState, false, 0));
    }

    std::wstring confirmPattern;
    hr = CaptureEnrollmentPattern(g_wszProductName, confirmPatternPrompt, confirmPattern);
    if (FAILED(hr)) {
        ResetPasswordCollectionState(
            firstPatternState == CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST
                ? CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD
                : CredentialProviderState::CPSTATE_RESET_PASSWORD);
        if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            MessageBoxW(m_hwndParent, L"Unable to capture the confirmation mosaic pattern.", g_wszProductName, MB_OK | MB_ICONERROR);
        }
        return hr;
    }

    if (m_firstEnrollmentPattern != confirmPattern) {
        ResetPasswordCollectionState(
            firstPatternState == CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST
                ? CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD
                : CredentialProviderState::CPSTATE_RESET_PASSWORD);
        MessageBoxW(m_hwndParent, mismatchMessage, g_wszProductName, MB_OK | MB_ICONWARNING);
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    std::vector<BYTE> protectedBlob;
    hr = CreateProtectedEnrollmentBlob(m_userSid, m_enrollmentPassword, m_firstEnrollmentPattern, protectedBlob);
    SecureClearString(m_enrollmentPassword);
    SecureClearString(m_firstEnrollmentPattern);
    if (FAILED(hr)) {
        m_cpState = firstPatternState == CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST
            ? CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD
            : CredentialProviderState::CPSTATE_RESET_PASSWORD;
        MessageBoxW(m_hwndParent, L"Unable to derive protected enrollment data.", g_wszProductName, MB_OK | MB_ICONERROR);
        if (m_pEvents != nullptr) {
            m_pEvents->SetFieldString(this, CP_FIELD_EXPLAIN, GetExplainTextForState(m_cpState, false, 0));
        }
        return hr;
    }

    hr = SaveEnrollmentInfo(m_userSid, protectedBlob);
    if (!protectedBlob.empty()) {
        SecureZeroMemory(protectedBlob.data(), protectedBlob.size());
        protectedBlob.clear();
    }
    if (FAILED(hr)) {
        m_cpState = firstPatternState == CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST
            ? CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD
            : CredentialProviderState::CPSTATE_RESET_PASSWORD;
        MessageBoxW(m_hwndParent, L"The mosaic data could not be saved. No changes were committed.", g_wszProductName, MB_OK | MB_ICONERROR);
        if (m_pEvents != nullptr) {
            m_pEvents->SetFieldString(this, CP_FIELD_EXPLAIN, GetExplainTextForState(m_cpState, false, 0));
        }
        return hr;
    }

    m_cpState = CredentialProviderState::CPSTATE_READY_FOR_LOGIN;
    SetReadyForLoginUIState(m_pEvents, this);

    MessageBoxW(m_hwndParent, successMessage, g_wszProductName, MB_OK | MB_ICONINFORMATION);
    return S_OK;
}

HRESULT CMosaicCredentialProviderCredential::RunEnrollmentSetup()
{
    return RunPatternProvisioningFlow(
        CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST,
        CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_CONFIRM,
        L"Enter your Windows password to begin mosaic enrollment.",
        L"Please enter your mosaic puzzle for enrollment.",
        L"Please re-enter your mosaic puzzle for enrollment.",
        L"The two mosaic patterns do not match. Start enrollment again.",
        L"Enrollment completed successfully.");
}

HRESULT CMosaicCredentialProviderCredential::RunResetMosaic()
{
    return RunPatternProvisioningFlow(
        CredentialProviderState::CPSTATE_RESET_PATTERN_FIRST,
        CredentialProviderState::CPSTATE_RESET_PATTERN_CONFIRM,
        L"Enter your Windows password to reset your mosaic pattern.",
        L"Please enter your new mosaic puzzle.",
        L"Please re-enter your new mosaic puzzle.",
        L"The two mosaic patterns do not match. Start the reset again.",
        L"Your mosaic pattern was updated successfully.");
}

void CMosaicCredentialProviderCredential::SetUserData(const std::wstring& userSid)
{
    m_userSid = userSid;
    SecureClearString(m_enrollmentPassword);
    SecureClearString(m_firstEnrollmentPattern);

    UserEnrollmentInfo enrollmentInfo;
    const HRESULT hrEnrollment = GetEnrollmentInfo(m_userSid, enrollmentInfo);
    m_cpState = HasCommittedEnrollment(hrEnrollment, enrollmentInfo)
        ? CredentialProviderState::CPSTATE_READY_FOR_LOGIN
        : CredentialProviderState::CPSTATE_INITIAL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::Advise(ICredentialProviderCredentialEvents* pcpce)
{
    dprintfW(L"Advise called with pcpce=%p\n", pcpce);

    m_pEvents = pcpce;
    const HRESULT hr = pcpce->OnCreatingWindow(&m_hwndParent);
    if (SUCCEEDED(hr)) {
        dprintfW(L"Stored parent window handle: %p\n", m_hwndParent);
    }
    else {
        dprintfW(L"OnCreatingWindow failed with HRESULT: 0x%08X\n", hr);
    }

    return hr;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::UnAdvise(void)
{
    dprintfW(L"UnAdvise called\n");

    SecureClearString(m_enrollmentPassword);
    SecureClearString(m_firstEnrollmentPattern);
    m_pEvents = nullptr;
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::SetSelected(BOOL* pbAutoLogon)
{
    dprintfW(L"SetSelected called with pbAutoLogon=%p\n", pbAutoLogon);
    if (pbAutoLogon != nullptr) {
        *pbAutoLogon = FALSE;
    }
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::SetDeselected(void)
{
    dprintfW(L"SetDeselected called\n");
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetFieldState(
    DWORD dwFieldID,
    CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis)
{
    dprintfW(L"GetFieldState called with dwFieldID=%d\n", dwFieldID);

    if (pcpfs == nullptr || pcpfis == nullptr) {
        return E_POINTER;
    }

    UserEnrollmentInfo enrollmentInfo;
    const HRESULT hrEnrollment = GetEnrollmentInfo(m_userSid, enrollmentInfo);
    const bool isEnrolled = HasCommittedEnrollment(hrEnrollment, enrollmentInfo);
    return InternalGetFieldState(dwFieldID, m_cpState, isEnrolled, enrollmentInfo.dwEnabled, pcpfs, pcpfis);
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetStringValue(DWORD dwFieldID, LPWSTR* ppsz)
{
    dprintfW(L"GetStringValue called with dwFieldID=%d\n", dwFieldID);

    if (ppsz == nullptr) {
        return E_POINTER;
    }

    UserEnrollmentInfo enrollmentInfo;
    const HRESULT hrEnrollment = GetEnrollmentInfo(m_userSid, enrollmentInfo);
    const bool isEnrolled = HasCommittedEnrollment(hrEnrollment, enrollmentInfo);

    switch (dwFieldID) {
    case CP_FIELD_HEADER:
        return SHStrDupW(g_wszProductName, ppsz);
    case CP_FIELD_EXPLAIN:
        return SHStrDupW(GetExplainTextForState(m_cpState, isEnrolled, enrollmentInfo.dwEnabled), ppsz);
    case CP_FIELD_VERIFY_CMD:
        return SHStrDupW(L"Verify", ppsz);
    case CP_FIELD_SETUP_CMD:
        return SHStrDupW(L"Setup", ppsz);
    case CP_FIELD_ENROLL_PASSWORD:
        return SHStrDupW(L"", ppsz);
    case CP_FIELD_ENROLL_SUBMIT:
        return SHStrDupW(L"Continue", ppsz);
    case CP_FIELD_FORGOT_MOSAIC:
        return SHStrDupW(L"I forgot my mosaic", ppsz);
    default:
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp)
{
    dprintfW(L"GetBitmapValue called with dwFieldID=%d\n", dwFieldID);

    if (phbmp == nullptr) {
        return E_POINTER;
    }

    *phbmp = nullptr;
    if (dwFieldID != CP_FIELD_TILE_IMAGE) {
        return E_NOTIMPL;
    }

    HBITMAP hBmp = LoadBitmap(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDB_BITMAP1));
    if (hBmp == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *phbmp = hBmp;
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, LPWSTR* ppszLabel)
{
    dprintfW(L"GetCheckboxValue called with dwFieldID=%d\n", dwFieldID);
    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo)
{
    dprintfW(L"GetSubmitButtonValue called with dwFieldID=%d\n", dwFieldID);

    if (pdwAdjacentTo == nullptr) {
        return E_POINTER;
    }

    if (dwFieldID == CP_FIELD_VERIFY_CMD) {
        *pdwAdjacentTo = CP_FIELD_EXPLAIN;
        return S_OK;
    }

    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem)
{
    dprintfW(L"GetComboBoxValueCount called with dwFieldID=%d\n", dwFieldID);
    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, LPWSTR* ppszItem)
{
    dprintfW(L"GetComboBoxValueAt called with dwFieldID=%d, dwItem=%d\n", dwFieldID, dwItem);
    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::SetStringValue(DWORD dwFieldID, LPCWSTR psz)
{
    dprintfW(L"SetStringValue called with dwFieldID=%d\n", dwFieldID);

    if (dwFieldID != CP_FIELD_ENROLL_PASSWORD) {
        return E_NOTIMPL;
    }

    SecureClearString(m_enrollmentPassword);
    m_enrollmentPassword = psz != nullptr ? psz : L"";
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
    dprintfW(L"SetCheckboxValue called with dwFieldID=%d, bChecked=%d\n", dwFieldID, bChecked);
    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem)
{
    dprintfW(L"SetComboBoxSelectedValue called with dwFieldID=%d, dwSelectedItem=%d\n", dwFieldID, dwSelectedItem);
    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::CommandLinkClicked(DWORD dwFieldID)
{
    dprintfW(L"CommandLinkClicked called with dwFieldID=%d\n", dwFieldID);

    if (dwFieldID == CP_FIELD_SETUP_CMD) {
        SecureClearString(m_enrollmentPassword);
        SecureClearString(m_firstEnrollmentPattern);
        m_cpState = CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD;
        SetEnterPasswordUIState(m_pEvents, this, L"Enter your Windows password to begin mosaic enrollment.");
        return S_OK;
    }

    if (dwFieldID == CP_FIELD_FORGOT_MOSAIC) {
        SecureClearString(m_enrollmentPassword);
        SecureClearString(m_firstEnrollmentPattern);
        m_cpState = CredentialProviderState::CPSTATE_RESET_PASSWORD;
        SetEnterPasswordUIState(m_pEvents, this, L"Enter your Windows password to reset your mosaic pattern.");
        return S_OK;
    }

    if (dwFieldID == CP_FIELD_ENROLL_SUBMIT) {
        if (m_cpState == CredentialProviderState::CPSTATE_RESET_PASSWORD) {
            return RunResetMosaic();
        }
        return RunEnrollmentSetup();
    }

    if (dwFieldID == CP_FIELD_VERIFY_CMD) {
        return S_OK;
    }

    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetSerialization(
    CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
    LPWSTR* ppszOptionalStatusText,
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    dprintfW(L"GetSerialization called\n");

    if (pcpgsr == nullptr || pcpcs == nullptr) {
        return E_POINTER;
    }

    *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    ZeroMemory(pcpcs, sizeof(*pcpcs));
    if (ppszOptionalStatusText != nullptr) {
        *ppszOptionalStatusText = nullptr;
    }
    if (pcpsiOptionalStatusIcon != nullptr) {
        *pcpsiOptionalStatusIcon = CPSI_NONE;
    }

    if (m_cpState == CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD) {
        return RunEnrollmentSetup();
    }

    if (m_cpState == CredentialProviderState::CPSTATE_RESET_PASSWORD) {
        return RunResetMosaic();
    }

    if (m_cpState == CredentialProviderState::CPSTATE_READY_FOR_LOGIN) {
        std::wstring normalizedPattern;
        HRESULT hr = CaptureEnrollmentPattern(g_wszProductName, L"Please enter your mosaic puzzle to unlock.", normalizedPattern);
        if (FAILED(hr)) {
            if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
                MessageBoxW(m_hwndParent, L"Unable to capture the mosaic verification pattern.", g_wszProductName, MB_OK | MB_ICONERROR);
            }
            return hr;
        }

        std::wstring recoveredPassword;
        UserEnrollmentInfo enrollmentInfo;
        const HRESULT hrEnrollment = GetEnrollmentInfo(m_userSid, enrollmentInfo);
        if (!HasCommittedEnrollment(hrEnrollment, enrollmentInfo)) {
            SecureClearString(recoveredPassword);
            MessageBoxW(m_hwndParent, L"The user is not enrolled with a valid mosaic pattern.", g_wszProductName, MB_OK | MB_ICONERROR);
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }

        hr = RecoverPasswordFromProtectedBlob(m_userSid, normalizedPattern, enrollmentInfo.protectedBlob, recoveredPassword);
        if (FAILED(hr)) {
            SecureClearString(recoveredPassword);
            MessageBoxW(m_hwndParent, L"The mosaic pattern is not valid for this account.", g_wszProductName, MB_OK | MB_ICONERROR);
            return hr;
        }

        hr = BuildSerializationForCurrentUser(m_userSid, recoveredPassword, pcpgsr, pcpcs);
        SecureClearString(recoveredPassword);
        return hr;
    }

    return E_NOTIMPL;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::ReportResult(
    NTSTATUS ntsStatus,
    NTSTATUS ntsSubstatus,
    LPWSTR* ppszOptionalStatusText,
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    dprintfW(L"ReportResult called with ntsStatus=%d, ntsSubstatus=%d\n", ntsStatus, ntsSubstatus);
    return S_OK;
}

HRESULT __stdcall CMosaicCredentialProviderCredential::GetUserSid(LPWSTR* sid)
{
    dprintfW(L"GetUserSid called\n");

    if (sid == nullptr) {
        return E_POINTER;
    }

    if (m_userSid.empty()) {
        *sid = nullptr;
        return E_NOTIMPL;
    }

    return SHStrDupW(m_userSid.c_str(), sid);
}
