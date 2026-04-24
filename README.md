<div align="center">
<img width="243" height="238" alt="logo" src="https://github.com/user-attachments/assets/26b30031-d23e-4b86-83db-064853f15a96" />

<h1>Mosaic Credential Provider</h1>
</div>

This project implements a **Windows Credential Provider** as a COM DLL. It introduces a mosaic puzzle as an alternate sign-in mechanism, conceptually similar to the pattern-based unlock flows commonly used on smartphones.

The target audience is Windows developers, reverse engineers, and systems programmers interested in LogonUI integration, Credential Provider development, secure credential derivation, and authentication-related COM infrastructure.

## Overview

The provider allows a user to enroll in the **Mosaic Credential Provider** and use a personal mosaic puzzle as an unlock credential.

The mosaic is a `4 x 4` grid. Each cell can contain one of four states:

- blank
- green square
- blue circle
- red cross

During enrollment, the provider derives a cryptographic key from the user-selected mosaic pattern. That key is then used to protect data that allows the underlying Windows logon password to be recovered during authentication. Secrets are stored using Windows DPAPI in a registry location secured for SYSTEM/administrative access.

The provider also supports a recovery/reset flow for users who forgot their mosaic pattern. That flow requires re-entry of the user's Windows password before a new pattern can be enrolled.

## Usage Flows

### Enrollment

1. A user who is not yet enrolled can switch to the Mosaic Credential Provider tile in LogonUI. The tile displays a message inviting the user to start enrollment.

   <img width="40%" alt="image" src="https://github.com/user-attachments/assets/40cb302c-e808-4810-8732-cb062e8adf0a" />

2. Clicking **Setup** prompts the user for the current Windows account password.

   <img width="40%" alt="image" src="https://github.com/user-attachments/assets/b8c93e63-76f5-47b0-8baf-0a4885cf4096" />

3. After the password is validated by Windows, a modal mosaic dialog is shown. The user clicks the cells to build the desired pattern using blank cells, green squares, blue circles, and red crosses.

   <img  alt="enrollment1" src="https://github.com/user-attachments/assets/bdb6a385-4150-4b81-b05d-f227b85d82bb" />

4. The provider then asks the user to enter the same pattern again for confirmation.

5. If both entries match, enrollment completes successfully.

   <img width="203" height="121" alt="enrollment_complete" src="https://github.com/user-attachments/assets/a737820f-a7c3-47a6-bb0e-9f95d2e1c59e" />

### Login

1. When an already enrolled user selects the Mosaic Credential Provider tile, LogonUI presents the provider in login mode instead of enrollment mode.

   <img width="352" height="284" alt="image" src="https://github.com/user-attachments/assets/686bd74e-979b-4944-bfa4-7fb0e3876de9" />

2. The tile invites the user to continue with the mosaic credential by using the submit arrow.

   <!-- TODO: add login prompt screenshot -->
   [Login prompt image placeholder]

3. Activating the submit arrow opens the mosaic dialog. The user re-enters the previously enrolled `4 x 4` mosaic pattern.

   <!-- TODO: add login mosaic dialog screenshot -->
   [Login mosaic dialog image placeholder]

4. If the entered mosaic matches the enrolled pattern, the provider reconstructs the Windows credential material and returns a credential serialization to LogonUI.

5. Windows then continues the normal sign-in flow using the recovered account credential.

   <!-- TODO: add successful login flow screenshot -->
   [Login success image placeholder]

### Mosaic Recovery

1. If an enrolled user no longer remembers the mosaic pattern, the user can select the recovery/reset option exposed by the provider tile.

   <!-- TODO: add recovery entry-point screenshot -->
   [Recovery entry-point image placeholder]

2. The provider asks for the current Windows account password before allowing the mosaic to be changed.

   <!-- TODO: add recovery password prompt screenshot -->
   [Recovery password prompt image placeholder]

3. After the password is validated, the provider opens the mosaic dialog and asks the user to enter a new pattern.

   <!-- TODO: add recovery first-pattern screenshot -->
   [Recovery new-pattern image placeholder]

4. The provider then asks the user to enter the same new pattern again for confirmation.

   <!-- TODO: add recovery confirm-pattern screenshot -->
   [Recovery confirmation image placeholder]

5. If both entries match, the old enrolled mosaic is replaced with the new one and the user can continue using the provider with the updated pattern.

   <img width="342" height="164" alt="image" src="https://github.com/user-attachments/assets/fff4bb21-d487-4401-9c16-144f716a7066" />


## Requirements and Installation

---

:warning: **Do not** experiment with Credential Providers on your main Windows installation. A bug in a provider can leave LogonUI unusable and force recovery through Safe Mode or offline remediation.

If that happens, it is usually sufficient to delete or unregister the provider DLL and reboot, or wait for Winlogon to reload LogonUI.

---

**Visual Studio 2022 Community Edition** is sufficient to build the DLL.

For development and testing, additional tools are strongly recommended.

### Additional Tools

The following tools were used during development:

- VMware or Hyper-V for isolated development/testing VMs
- Remote Debugging Tools for Visual Studio 2022  
  https://visualstudio.microsoft.com/es/downloads/
- Sysinternals DebugView  
  https://learn.microsoft.com/en-us/sysinternals/downloads/debugview

Debugging tools must be run with administrative privileges because Credential Providers are instantiated under the security context of `LogonUI.exe`.

### Building and Installing

- Open the solution in Visual Studio 2022 or later and build the desired configuration (`x86` or `x64`).
- The output DLL is generated under:

  ```text
  build\bin\<arch>\<config>\
  ```

  where:

  - `<arch>` is `x86` or `x64`
  - `<config>` is `Debug` or `Release`

- Copy the DLL to the target machine. For example:

  ```powershell
  xcopy build\bin\x64\Debug\MosaicCredProv.dll \\win11vm\temp /y
  ```

- On the target VM, open an elevated Command Prompt and run:

  ```powershell
  regsvr32 /i C:\TEMP\MosaicCredProv.dll
  ```

  Replace `C:\TEMP` with the directory where the DLL was copied.

## After-Install Registry Keys

Besides the `HKEY_CLASSES_ROOT\CLSID` entries created by COM registration, the following registry keys are relevant to the provider at runtime:

| Registry key (root at `HKEY_LOCAL_MACHINE\SOFTWARE`) | Purpose |
|---|---|
| `Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{30106E01-B65F-480E-993E-92D5D7310C5E}` | Registers the main Mosaic Credential Provider tile with LogonUI. The default value is `Mosaic Credential Provider`. |
| `Microsoft\Windows\CurrentVersion\Authentication\Credential Provider Filters\{5DAAB89B-38AC-437E-94F9-2379127F8564}` | Registers the optional Credential Provider Filter COM class. The default value is `Mosaic Credential Provider Filter`. |
| `HernanDiPietro\MosaicCredentialProvider` | Product configuration root created by `DllInstall`. Currently stores the provider-wide `Enabled` flag used by `SetUsageScenario` to determine whether the provider should participate. |
| `HernanDiPietro\MosaicCredentialProvider\Enrollment` | Parent container for per-user enrollment state. This key is created on demand as users enroll. |
| `HernanDiPietro\MosaicCredentialProvider\Enrollment\{User SID}` | Per-user enrollment record. Stores `Enabled` and the DPAPI-protected `ProtectedBlob` that ties the Windows password to the enrolled mosaic pattern. A typical SID looks like `S-1-5-21-...-1001`. |

### CLSIDs and COM Identifiers

| Symbol / class | GUID | Purpose |
|---|---|---|
| `LIBID_MosaicCredProvLib` | `5183C3D9-9A6A-4A8C-9034-FB77634C68B5` | Type library ID generated from `MosaicCredProv.idl`. |
| `CLSID_MosaicCredentialProvider` | `30106E01-B65F-480E-993E-92D5D7310C5E` | Main Credential Provider COM class exposed to LogonUI. |
| `CLSID_MosaicCredentialProviderCredential` | `6EDDC324-1233-4597-B163-2C989210ACEB` | Credential/tile COM class instantiated by the provider for each displayed credential. |
| `CLSID_MosaicCredentialProviderFilter` | `5DAAB89B-38AC-437E-94F9-2379127F8564` | Credential Provider Filter COM class. |

Running `regsvr32 /i MosaicCredProv.dll` performs two distinct tasks:

1. Standard COM registration through `DllRegisterServer`, which creates the `HKCR\CLSID` and type library entries.
2. Product-specific installation through `DllInstall`, which creates the LogonUI registration entries and the provider configuration root under `HKLM\SOFTWARE\HernanDiPietro\MosaicCredentialProvider`.
