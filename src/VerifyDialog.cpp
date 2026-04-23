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
#include "VerifyDialog.h"
#include "MosaicPattern.h"
#include "MosaicRender.h"
#include "dprintf.h"

namespace
{
    constexpr int BUTTON_BASE_ID = 3000;
    constexpr int BUTTON_SIZE = 48;
    constexpr int BUTTON_GAP = 4;
    constexpr int PROMPT_HEIGHT = 40;
    constexpr int PROMPT_TOP = 10;
    constexpr int PROMPT_MARGIN = 10;

    struct VerifyDialogContext
    {
        LPCWSTR pszUserSid{ nullptr };
        LPCWSTR pszTitle{ nullptr };
        LPCWSTR pszPrompt{ nullptr };
        MosaicPatternResult* result{ nullptr };
    };

    struct VerifyDialogState
    {
        HWND buttons[kMosaicButtonCount]{};
        int faceState[kMosaicButtonCount]{};
        HBITMAP bitmaps[3]{};
        VerifyDialogContext context{};
    };

    void ApplyButtonFace(VerifyDialogState* state, int index)
    {
        if (state == nullptr || index < 0 || index >= kMosaicButtonCount || state->buttons[index] == nullptr) {
            return;
        }

        const int face = state->faceState[index];
        HBITMAP hFace = nullptr;
        if (face >= MOSAIC_BITMAP_CROSS && face <= MOSAIC_BITMAP_CIRCLE) {
            hFace = state->bitmaps[face - 1];
        }

        SendMessageW(state->buttons[index], BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hFace));
    }

    void CreateMosaicButtons(HWND hwndDlg, VerifyDialogState* state)
    {
        RECT rcClient{};
        GetClientRect(hwndDlg, &rcClient);

        const int mosaicWidth = (kMosaicButtonCols * BUTTON_SIZE) + ((kMosaicButtonCols - 1) * BUTTON_GAP);
        const int mosaicHeight = (kMosaicButtonRows * BUTTON_SIZE) + ((kMosaicButtonRows - 1) * BUTTON_GAP);
        const int originX = (rcClient.right - mosaicWidth) / 2;
        const int promptBottom = state->context.pszPrompt != nullptr ? (PROMPT_TOP + PROMPT_HEIGHT + 8) : 0;
        const int availableHeight = rcClient.bottom - promptBottom - 28;
        const int originY = promptBottom + max(0, (availableHeight - mosaicHeight) / 2);

        if (state->context.pszPrompt != nullptr) {
            CreateWindowExW(
                0,
                L"STATIC",
                state->context.pszPrompt,
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                PROMPT_MARGIN,
                PROMPT_TOP,
                rcClient.right - (PROMPT_MARGIN * 2),
                PROMPT_HEIGHT,
                hwndDlg,
                nullptr,
                ATL::_AtlBaseModule.GetModuleInstance(),
                nullptr);
        }

        for (int row = 0; row < kMosaicButtonRows; ++row) {
            const int y = originY + row * (BUTTON_SIZE + BUTTON_GAP);
            for (int col = 0; col < kMosaicButtonCols; ++col) {
                const int index = row * kMosaicButtonCols + col;
                const int x = originX + col * (BUTTON_SIZE + BUTTON_GAP);
                HWND hButton = CreateWindowExW(
                    0,
                    L"BUTTON",
                    L"",
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_BITMAP,
                    x,
                    y,
                    BUTTON_SIZE,
                    BUTTON_SIZE,
                    hwndDlg,
                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(BUTTON_BASE_ID + index)),
                    ATL::_AtlBaseModule.GetModuleInstance(),
                    nullptr);

                state->buttons[index] = hButton;
                state->faceState[index] = MOSAIC_STATE_EMPTY;
                ApplyButtonFace(state, index);
            }
        }
    }

    void CleanupState(VerifyDialogState* state)
    {
        if (state == nullptr) {
            return;
        }

        for (HBITMAP hBmp : state->bitmaps) {
            if (hBmp != nullptr) {
                DeleteObject(hBmp);
            }
        }

        delete state;
    }
}

INT_PTR VerifyDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
    {
        auto* context = reinterpret_cast<VerifyDialogContext*>(lParam);
        dprintfW(L"[dlg] initialized for user SID: %s\n", context != nullptr ? context->pszUserSid : L"(null)");
        if (context != nullptr && context->pszTitle != nullptr) {
            SetWindowTextW(hwndDlg, context->pszTitle);
        }

        if (HWND hwndParent = GetParent(hwndDlg)) {
            RECT rcParent{}, rcDlg{};
            GetWindowRect(hwndParent, &rcParent);
            GetWindowRect(hwndDlg, &rcDlg);
            const int x = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
            const int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;
            SetWindowPos(hwndDlg, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }

        auto* state = new (std::nothrow) VerifyDialogState();
        if (state == nullptr) {
            EndDialog(hwndDlg, IDCANCEL);
            return FALSE;
        }

        state->context = context != nullptr ? *context : VerifyDialogContext{};
        state->bitmaps[0] = CreateFaceBitmap(MOSAIC_BITMAP_CROSS, BUTTON_SIZE);
        state->bitmaps[1] = CreateFaceBitmap(MOSAIC_BITMAP_SQUARE, BUTTON_SIZE);
        state->bitmaps[2] = CreateFaceBitmap(MOSAIC_BITMAP_CIRCLE, BUTTON_SIZE);
        SetWindowLongPtrW(hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        CreateMosaicButtons(hwndDlg, state);
        return TRUE;
    }

    case WM_COMMAND:
    {
        const WORD controlId = LOWORD(wParam);
        const WORD notifyCode = HIWORD(wParam);
        auto* state = reinterpret_cast<VerifyDialogState*>(GetWindowLongPtrW(hwndDlg, GWLP_USERDATA));

        if (notifyCode == BN_CLICKED && controlId >= BUTTON_BASE_ID && controlId < BUTTON_BASE_ID + kMosaicButtonCount && state != nullptr) {
            const int index = static_cast<int>(controlId - BUTTON_BASE_ID);
            state->faceState[index] = (state->faceState[index] + 1) % MOSAIC_STATE_COUNT;
            ApplyButtonFace(state, index);
            return TRUE;
        }

        if (controlId == IDOK && state != nullptr) {
            if (state->context.result != nullptr) {
                state->context.result->confirmed = true;
                state->context.result->normalizedPattern = BuildNormalizedPattern(state->faceState, kMosaicButtonCount);
            }

            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }

        if (controlId == IDCANCEL) {
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
    }

    case WM_DESTROY:
    {
        auto* state = reinterpret_cast<VerifyDialogState*>(GetWindowLongPtrW(hwndDlg, GWLP_USERDATA));
        CleanupState(state);
        SetWindowLongPtrW(hwndDlg, GWLP_USERDATA, 0);
        return TRUE;
    }
    }

    return FALSE;
}

HRESULT OpenVerifyDialog(HWND hwndParent, LPCWSTR pszUserSid, LPCWSTR pszTitle, LPCWSTR pszPrompt, MosaicPatternResult& result)
{
    result = MosaicPatternResult{};

    VerifyDialogContext context{};
    context.pszUserSid = pszUserSid;
    context.pszTitle = pszTitle;
    context.pszPrompt = pszPrompt;
    context.result = &result;

    const INT_PTR dlgResult = ATL::AtlAxDialogBoxW(
        ATL::_AtlBaseModule.GetModuleInstance(),
        MAKEINTRESOURCEW(IDD_VERIFY_DIALOG),
        hwndParent,
        VerifyDlgProc,
        reinterpret_cast<LPARAM>(&context));

    if (dlgResult == -1) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (dlgResult != IDOK) {
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    return S_OK;
}
