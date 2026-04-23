# Mosaic Credential Provider

This project includes a Credential Provider for Windows implementing a 
mosaic puzzle as a login mechanism for the user, similar  to the patterns
used to unlock smartphones.

## Requirements and Installation

:warn: DONT play with Credential Providers on your main Windows system,
as any bug can leave your logon UI in an inoperable state, forcing you to boot into 
Safe Mode. 

If this happens, it is enough to delete the Credential Provider DLL and reboot
(or reload the Logon UI).


* VS2022 Community Edition (free) is enough to build the Credential Provider DLL.

Additional tools are recommended for testing and development, see below.

### Additional Tools

In particular for this project I used:

* VMWare / HyperV for development.
* Remote Debugging Tools for VS2022  https://visualstudio.microsoft.com/es/downloads/
* SysInternals DebugView https://learn.microsoft.com/en-us/sysinternals/downloads/debugview

The debugging tools must be run with Administrator account to work
as Credential Providers are instantiated under the security context of the LogonUI process.


* To build the project, open the solution on VS2022 or higher. Select your 
desired configuration (x64/x86).

* The output DLL will be at `build\bin\<arch>\<conf>` where <arch> is x86 or x64, 
and <conf> `Debug` or `Release`.  





## Features





