# Mosaic Credential Provider

## Scope
- This repository contains a Windows Credential Provider implemented with ATL/C++.
- The supported runtime surfaces are `CPUS_LOGON` and `CPUS_UNLOCK_WORKSTATION`.

## Build And Environment
- Primary development environment: Visual Studio on Windows.
- Project file: `MosaicCredProv.vcxproj`.
- Treat this as a Windows-native codebase. Do not introduce cross-platform abstractions unless there is a concrete need.

## Editing Rules
- Preserve `CRLF` line endings for every text file in this repository.
- When editing an existing file, normalize the full file back to `CRLF` before finishing. Do not leave mixed line endings in touched files.
- Use plain `UTF-8` without BOM for repository text files.
- Keep the MIT license header at the top of every checked-in source file:
  - `*.h`
  - `*.cpp`
  - `*.c`
  - `*.idl`
  - `*.rc`
- Keep the root `LICENSE` file in sync with those headers.

## Generated And Interface Files
- `src/MosaicCredProv_i.h` and `MosaicCredProv_i.c` are generated/interface artifacts but are committed in the repository.
- Do not edit generated files casually. If interface definitions change, regenerate them consistently instead of patching them by hand unless there is a specific reason.

## Credential Provider Guardrails
- Do not change CLSIDs, interface wiring, or resource IDs casually.
- Preserve the behavior of these flows unless the task explicitly changes them:
  - enrollment
  - login
  - reset/recovery
- Changes that affect field visibility, serialization, or dialog flow should be tested against both:
  - `CPUS_LOGON`
  - `CPUS_UNLOCK_WORKSTATION`
- Be careful with anything that affects LogonUI rendering or secure-desktop behavior.

## Current Structure
- `MosaicCpCredentialCoClass.*`: COM credential implementation and orchestration.
- `CredentialFlow.*`: field-state and credential flow helpers.
- `EnrollmentCrypto.*`: protected enrollment blob creation and recovery.
- `WindowsAuth.*`: SID resolution, password validation, and credential serialization.
- `EnrollmentStore.*`: per-user enrollment persistence.
- `ProviderRegistration.*`: provider registration and uninstall registry logic.
- `MosaicDialog.*`: modal mosaic dialog plumbing.
- `MosaicPattern.*`: normalized mosaic model helpers.
- `MosaicRender.*`: mosaic glyph bitmap rendering.

## Change Discipline
- Prefer behavior-preserving refactors unless the task explicitly asks for product changes.
- When adding a new source file, include the repository license header immediately.
- Keep comments factual and sparse.

