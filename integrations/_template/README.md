# Adding a new integration

1. Copy this `_template` directory to `integrations/<name>/` (all lowercase).
2. Replace the placeholders:
   - `<NAME>`   — upper-case name used by the top-level switch (e.g. `OPENSSL`)
   - `<name>`   — lower-case name (directory / target / file names)
   - `<ExternalLib>` — the external library's `find_package` package name
3. Top-level `CMakeLists.txt`:
   - add `set(SCL2EXT_<NAME> "AUTO" CACHE STRING "...")`
   - add `<name>` to the `foreach(ext ...)` list
4. Write:
   - `include/SCL2Ext/<name>.hpp` — public API (namespace `scl2ext`)
   - `src/<name>.cpp` — implementation

## Guidelines

- Each integration is independent: it only depends on `find_package`'d external
  library + SharedCppLib2. No cross-integration coupling.
- Public target: `SCL2Ext::<name>` (internal: `scl2ext_<name>`).
- Header path: `<SCL2Ext/<name>.hpp>`, matching the `<SharedCppLib2/...>` style.
- Namespace: `scl2ext` (do not pollute `scl2`).
- If the external library has several package-name flavors (e.g. bit7z's
  official vs vcpkg `unofficial-bit7z`), try each via `find_package(... QUIET)`
  and pick the first available target.
- If the external library is runtime-loaded (e.g. bit7z loads 7-Zip DLL via
  `LoadLibrary`), note it in the integration's README / CMake so consumers know
  to copy the DLL next to their executable.
