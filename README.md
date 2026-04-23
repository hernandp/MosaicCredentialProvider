
<div align="center">
<img width="243" height="238" alt="logo" src="https://github.com/user-attachments/assets/26b30031-d23e-4b86-83db-064853f15a96" />

<h1>Mosaic Credential Provider</h1>
</div>

This project consists of a **Credential Provider for Windows** COM object implementing a mosaic puzzle as a login mechanism for the user, similar  to the patterns used to unlock smartphones.  

## Overview 

What this  Credential Provider does?

It offers the user to enroll to the "Mosaic Credential Provider", which will allow him to use a personal Mozaic Puzzle design as an unlock credential. The Mosaic Puzzle is a 4x4 grid that can contain a blank space, a green square, a blue circle or a red cross(X) in each cell. This puzzle generates a cryptographically-generated key for which the main logon password account can be derived. Secrets are stored using the Windows DPAPI under a SYSTEM secured registry key.

The system also offers a secure mechanism to reset the mosaic pattern to a new one, if the user forgot it.

## Usage flows

### Enrollment

1. When a new user is going to be enrolled, he can click the Mosaic Credential Provider icon in the Logon UI to switch to it. A message inviting the user to start the enrollment process will appear:

   <img width="40%" height="50%" alt="image" src="https://github.com/user-attachments/assets/40cb302c-e808-4810-8732-cb062e8adf0a" />

2. Clicking the **Setup** Button will ask the user for its Windows account logon password. 

   <img width="563" height="289" alt="image" src="https://github.com/user-attachments/assets/b8c93e63-76f5-47b0-8baf-0a4885cf4096" />

3. After LSA successfully validates the password as valid, the popup dialog to enter the Mosaic pattern will appear. Here the user can click cells to "draw" its preferred pattern consisting of blank spaces, green squares, blue circles or red crosses.

   <img width="224" height="244" alt="enrollment1" src="https://github.com/user-attachments/assets/bdb6a385-4150-4b81-b05d-f227b85d82bb" />

4. The Credential Provider will request the user to re-enter the  pattern in a second Mosaic dialog to confirm.
5. When the user enters a matching pattern, the system will report the enrollment as complete:

   <img width="203" height="121" alt="enrollment_complete" src="https://github.com/user-attachments/assets/a737820f-a7c3-47a6-bb0e-9f95d2e1c59e" />


## Requirements and Installation

---

:warning: **DONT** play with Credential Providers on your main Windows system, as any bug can leave your logon UI in an inoperable state, forcing you to boot into Safe Mode. 

If this happens, it is enough to delete the Credential Provider DLL and reboot, or wait for Winlogon to reload the Logon UI.

---

**VS2022 Community Edition (free)** is enough to build the Credential Provider DLL.
Additional tools are recommended for testing and development, see below.

### Additional Tools

In particular for this project I used:

* VMWare / HyperV for development.
* Remote Debugging Tools for VS2022  https://visualstudio.microsoft.com/es/downloads/
* SysInternals DebugView https://learn.microsoft.com/en-us/sysinternals/downloads/debugview

The debugging tools must be run with Administrator account to work as Credential Providers are instantiated under the security context of the LogonUI process.

### Building and installing

* To build the project, open the solution on VS2022 or higher. Select your 
desired configuration (x64/x86).
* The output DLL will be at `build\bin\<arch>\<conf>` where `<arch>` is x86 or x64, 
and `<conf>` either `Debug` or `Release`.  
* Copy the binary to your target system. e.g Use good ol' `XCOPY` from the project root dir targeting your networked VM/directory:
    ```
    xcopy build\bin\x64\Debug\MosaicCredProv.dll \\win11vm\temp /y
    ```
* Go to your target VM, launch a Command Prompt with Administrative rights and run:
    ```
    regsvr32 /i C:\TEMP\MosaicCredProv.dll
    ```
  Of course, replace `TEMP` with the directory where the DLL was copied.
 
## After-Install registry keys 

Besides the `HKEY_CLASSES_ROOT\CLSID` entries for COM registrations, the following registry keys will be installed on your target system:

| Registry key | Used for |
|---|---|
| `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{30106E01-B65F-480E-993E-92D5D7310C5E}` | Registers the main Mosaic Credential Provider tile with LogonUI. The default value is `Mosaic Credential Provider`. |
| `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Provider Filters\{5DAAB89B-38AC-437E-94F9-2379127F8564}` | Registers the optional Credential Provider Filter COM class. The default value is `Mosaic Credential Provider Filter`. |
| `HKLM\SOFTWARE\HernanDiPietro\MosaicCredentialProvider` | Product configuration root created by `DllInstall`. Currently stores the provider-wide `Enabled` flag used by `SetUsageScenario` to decide whether the provider should participate. |
| `HKLM\SOFTWARE\HernanDiPietro\MosaicCredentialProvider\Enrollment` | Parent container for per-user enrollment state. This key is created on demand as users enroll. |
| `HKLM\SOFTWARE\HernanDiPietro\MosaicCredentialProvider\Enrollment\{User SID}` | Per-user enrollment record. Stores `Enabled` and the DPAPI-protected `ProtectedBlob` that ties the Windows password to the enrolled mosaic pattern. Example SID form: `S-1-5-21-...-1001`. |

### CLSIDs and COM identifiers

| Symbol / class | GUID | Used for |
|---|---|---|
| `LIBID_MosaicCredProvLib` | `5183C3D9-9A6A-4A8C-9034-FB77634C68B5` | Type library ID for the project COM metadata generated from `MosaicCredProv.idl`. |
| `CLSID_MosaicCredentialProvider` | `30106E01-B65F-480E-993E-92D5D7310C5E` | Main Credential Provider COM class exposed to LogonUI. |
| `CLSID_MosaicCredentialProviderCredential` | `6EDDC324-1233-4597-B163-2C989210ACEB` | Credential/tile COM class created by the provider for each displayed credential. |
| `CLSID_MosaicCredentialProviderFilter` | `5DAAB89B-38AC-437E-94F9-2379127F8564` | Credential Provider Filter COM class. |


Let's explain with some detail th

The Credential Provider (from now CP) will 



