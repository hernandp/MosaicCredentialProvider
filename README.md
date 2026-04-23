# Mosaic Credential Provider

This project includes a Credential Provider for Windows implementing a mosaic puzzle as a login mechanism for the user, similar  to the patterns used to unlock smartphones.

## Overview 

What this  Credential Provider does?

Offers the user to enroll to the "Mosaic Credential Provider", which will allow him to use a personal Mozaic Puzzle design -a 4x4 grid that can contain a square, a circle or an X, 

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

The following registry keys will be installed on your target system:

| Registry key | Used for |
|---|---|
| `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{30106E01-B65F-480E-993E-92D5D7310C5E}` | Registers the main Mosaic Credential Provider tile with LogonUI. The default value is `Mosaic Credential Provider`. |
| `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Provider Filters\{5DAAB89B-38AC-437E-94F9-2379127F8564}` | Registers the optional Credential Provider Filter COM class. The default value is `Mosaic Credential Provider Filter`. |
| `HKEY_LOCAL_MACHINE\SOFTWARE\HernanDiPietro\MosaicCredentialProvider` | Product configuration root created by `DllInstall`. Currently stores the provider-wide `Enabled` flag used by `SetUsageScenario` to decide whether the provider should participate. |
| `HKEY_LOCAL_MACHINE\SOFTWARE\HernanDiPietro\MosaicCredentialProvider\Enrollment` | Parent container for per-user enrollment state. This key is created on demand as users enroll. |
| `HKEY_LOCAL_MACHINE\SOFTWARE\HernanDiPietro\MosaicCredentialProvider\Enrollment\{User SID}` | Per-user enrollment record. Stores `Enabled` and the DPAPI-protected `ProtectedBlob` that ties the Windows password to the enrolled mosaic pattern. Example SID form: `S-1-5-21-...-1001`. |
| `HKEY_CLASSES_ROOT\CLSID\{30106E01-B65F-480E-993E-92D5D7310C5E}` | COM registration for `MosaicCredentialProvider`. Created by COM self-registration (`DllRegisterServer`) and the `MosaicCredentialProvider.rgs` script. |
| `HKEY_CLASSES_ROOT\CLSID\{6EDDC324-1233-4597-B163-2C989210ACEB}` | COM registration for `MosaicCredentialProviderCredential`. This is the credential object instantiated by the provider for each tile. |
| `HKEY_CLASSES_ROOT\CLSID\{5DAAB89B-38AC-437E-94F9-2379127F8564}` | COM registration for `MosaicCredentialProviderFilter`. Created by COM self-registration and the `MosaicCpFilterCoClass.rgs` script. |
| `HKEY_CLASSES_ROOT\TypeLib\{5183C3D9-9A6A-4A8C-9034-FB77634C68B5}` | Type library registration for `MosaicCredProvLib`. This is part of normal ATL/MIDL COM registration performed by `DllRegisterServer`. |

### CLSIDs and COM identifiers

| Symbol / class | GUID | Used for |
|---|---|---|
| `LIBID_MosaicCredProvLib` | `5183C3D9-9A6A-4A8C-9034-FB77634C68B5` | Type library ID for the project COM metadata generated from `MosaicCredProv.idl`. |
| `CLSID_MosaicCredentialProvider` | `30106E01-B65F-480E-993E-92D5D7310C5E` | Main Credential Provider COM class exposed to LogonUI. |
| `CLSID_MosaicCredentialProviderCredential` | `6EDDC324-1233-4597-B163-2C989210ACEB` | Credential/tile COM class created by the provider for each displayed credential. |
| `CLSID_MosaicCredentialProviderFilter` | `5DAAB89B-38AC-437E-94F9-2379127F8564` | Credential Provider Filter COM class. |


Let's explain with some detail th

The Credential Provider (from now CP) will 



