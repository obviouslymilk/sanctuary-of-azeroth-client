# AGENTS.md

C++ decompilation of the World of Warcraft 3.3.5a client (`Wow.exe`, build 12340). See
[README.md](README.md) for requirements, options and running.

## Build

Out-of-source only (in-source builds are refused). Output lands in `bin/Win/`.

```powershell
cmake -S . -B build/x64 -A x64
cmake --build build/x64 --config Release --target WowClient --parallel
```

Win32 (needs NASM): same with `build/win32` and `-A Win32`. No tests; the check is the build.

## Hard constraints

- Every ported function keeps its original address in an `// OFFSET: 0x...` comment directly
  above it; porting progress is measured from these tags.
- Source mirrors the original client: `Engine/Source/<Module>`, `WoW/Source/<Module>`, original
  file names.
- x64 is the main target; Win32 must still build, as it checks struct layouts against the 32-bit
  original.
- Never commit game data (the client's `Data` folder, `DivxDecoder.dll`).

## Docs

- [Agent setup](.agents/docs/onboarding.md) — read when adding a doc, skill or rule.

## Plans

Save plans, requirements and investigations in `.agents/plans/<slug>/` (gitignored).
