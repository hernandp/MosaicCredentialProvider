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
#include "CredentialFlow.h"

#include "FieldDescriptors.h"
#include "ProductConfig.h"

bool HasCommittedEnrollment(HRESULT hrEnrollment, const UserEnrollmentInfo& enrollmentInfo)
{
    return hrEnrollment == S_OK && !enrollmentInfo.protectedBlob.empty();
}

void SecureClearString(std::wstring& value)
{
    if (!value.empty()) {
        SecureZeroMemory(value.data(), value.size() * sizeof(wchar_t));
        value.clear();
    }
}

LPCWSTR GetExplainTextForState(CredentialProviderState state, bool isEnrolled, DWORD enrollmentEnabled)
{
    switch (state) {
    case CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD:
        return L"Enter your Windows password to begin mosaic enrollment.";
    case CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_FIRST:
        return L"Create your mosaic pattern in the verify dialog.";
    case CredentialProviderState::CPSTATE_ENROLLMENT_PATTERN_CONFIRM:
        return L"Repeat the same mosaic pattern to confirm enrollment.";
    case CredentialProviderState::CPSTATE_RESET_PASSWORD:
        return L"Enter your Windows password to reset your mosaic pattern.";
    case CredentialProviderState::CPSTATE_RESET_PATTERN_FIRST:
        return L"Enter your new mosaic pattern in the verify dialog.";
    case CredentialProviderState::CPSTATE_RESET_PATTERN_CONFIRM:
        return L"Repeat the new mosaic pattern to confirm the reset.";
    case CredentialProviderState::CPSTATE_READY_FOR_LOGIN:
        return enrollmentEnabled != 0
            ? L"Use the arrow to enter your mosaic pattern."
            : L"Your mosaic access is disabled. Please contact your administrator.";
    case CredentialProviderState::CPSTATE_INITIAL:
    default:
        if (isEnrolled) {
            return enrollmentEnabled != 0
                ? L"Use the arrow to enter your mosaic pattern."
                : L"Your mosaic access is disabled. Please contact your administrator.";
        }
        return L"You are not enrolled yet. Click Setup to create your mosaic access pattern.";
    }
}

HRESULT InternalGetFieldState(
    DWORD dwFieldID,
    CredentialProviderState state,
    bool isEnrolled,
    DWORD enrollmentEnabled,
    CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis)
{
    if (pcpfs == nullptr || pcpfis == nullptr) {
        return E_POINTER;
    }

    *pcpfis = CPFIS_NONE;
    *pcpfs = CPFS_HIDDEN;

    switch (dwFieldID) {
    case CP_FIELD_TILE_IMAGE:
    case CP_FIELD_HEADER:
    case CP_FIELD_EXPLAIN:
        *pcpfs = CPFS_DISPLAY_IN_BOTH;
        return S_OK;
    case CP_FIELD_VERIFY_CMD:
        if (isEnrolled &&
            (state == CredentialProviderState::CPSTATE_READY_FOR_LOGIN || state == CredentialProviderState::CPSTATE_INITIAL)) {
            *pcpfs = CPFS_DISPLAY_IN_BOTH;
            if (enrollmentEnabled == 0) {
                *pcpfis = CPFIS_DISABLED;
            }
        }
        return S_OK;
    case CP_FIELD_FORGOT_MOSAIC:
        if (isEnrolled && enrollmentEnabled != 0 &&
            (state == CredentialProviderState::CPSTATE_READY_FOR_LOGIN || state == CredentialProviderState::CPSTATE_INITIAL)) {
            *pcpfs = CPFS_DISPLAY_IN_BOTH;
        }
        return S_OK;
    case CP_FIELD_SETUP_CMD:
        if (!isEnrolled && state == CredentialProviderState::CPSTATE_INITIAL) {
            *pcpfs = CPFS_DISPLAY_IN_BOTH;
        }
        return S_OK;
    case CP_FIELD_ENROLL_PASSWORD:
    case CP_FIELD_ENROLL_SUBMIT:
        if (state == CredentialProviderState::CPSTATE_ENROLLMENT_PASSWORD ||
            state == CredentialProviderState::CPSTATE_RESET_PASSWORD) {
            *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
        }
        return S_OK;
    default:
        return E_INVALIDARG;
    }
}

void SetEnterPasswordUIState(ICredentialProviderCredentialEvents* pEvents, ICredentialProviderCredential* credential, LPCWSTR explainText)
{
    if (pEvents == nullptr || credential == nullptr) {
        return;
    }

    pEvents->SetFieldState(credential, CP_FIELD_VERIFY_CMD, CPFS_HIDDEN);
    pEvents->SetFieldState(credential, CP_FIELD_FORGOT_MOSAIC, CPFS_HIDDEN);
    pEvents->SetFieldState(credential, CP_FIELD_SETUP_CMD, CPFS_HIDDEN);
    pEvents->SetFieldState(credential, CP_FIELD_ENROLL_PASSWORD, CPFS_DISPLAY_IN_SELECTED_TILE);
    pEvents->SetFieldState(credential, CP_FIELD_ENROLL_SUBMIT, CPFS_DISPLAY_IN_SELECTED_TILE);
    pEvents->SetFieldString(credential, CP_FIELD_EXPLAIN, explainText);
    pEvents->SetFieldInteractiveState(credential, CP_FIELD_ENROLL_PASSWORD, CPFIS_FOCUSED);
}

void SetReadyForLoginUIState(ICredentialProviderCredentialEvents* pEvents, ICredentialProviderCredential* credential)
{
    if (pEvents == nullptr || credential == nullptr) {
        return;
    }

    pEvents->SetFieldState(credential, CP_FIELD_ENROLL_PASSWORD, CPFS_HIDDEN);
    pEvents->SetFieldState(credential, CP_FIELD_ENROLL_SUBMIT, CPFS_HIDDEN);
    pEvents->SetFieldState(credential, CP_FIELD_SETUP_CMD, CPFS_HIDDEN);
    pEvents->SetFieldState(credential, CP_FIELD_VERIFY_CMD, CPFS_DISPLAY_IN_BOTH);
    pEvents->SetFieldState(credential, CP_FIELD_FORGOT_MOSAIC, CPFS_DISPLAY_IN_BOTH);
    pEvents->SetFieldString(credential, CP_FIELD_EXPLAIN, L"Use the arrow to enter your mosaic pattern.");
}
