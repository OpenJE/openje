# JE::cls_0x4d8d70

[Return to `JE`](/docs/je.md)

## C++

- [`cls_0x4d8d70.hpp`](/src/je/cls_0x4d8d70.hpp)
- [`cls_0x4d8d70.cpp`](/src/je/cls_0x4d8d70.cpp)

## Reverse engineering notes

Full annotations live in the original database (`artifacts/F3.exe.i64`):
`F3::global_cls_0x4d8d70_0x70c668`, `cls_0x4d8d70::ctor_0x59f500`, `cls_0x4d8d70::dtor_0x59f4d0`,
`cls_0x4d8d70::buynode_0x59ec40` (was `sub_59EC40`), `F3::GLOBAL__I_command_registry` (`0x643f60`),
`F3::atexit_thunk_command_registry` (was `unknown_libname_602`, `0x646b90`), and the search/insert helpers
`0x545710`, `0x59f3e0`, `0x59f540`, `0x59f0b0`, `0x59ece0`.

### Confirmed

- The class is an MSVC STL tree control block (a `map<string, ...>`-shaped `_Tree`/`_Tmap_traits`). The name
  was never recovered: there is no RTTI for it, so the database keeps the address-derived name.
  `sizeof(cls_0x4d8d70) == 0x38`.
- Only three members are evidenced by the code paths inspected: `+0x00` (byte), `+0x04` (`_Myhead`, also the
  end sentinel) and `+0x08` (node count). The rest of the 0x38-byte object is recorded in the database but is
  not modelled by any inspected function.
- Nodes are 0x30 bytes, allocated by `buynode_0x59ec40` (`operator new(0x30)`, links cleared,
  `+0x2C _Color = 1`, `+0x2D _Isnil = 0`):
  - `+0x00 _Left`, `+0x04 _Parent`, `+0x08 _Right` (the header node links to itself)
  - `+0x10` key `std::string`, MSVC 0x18-byte layout (inline buffer/pointer at `+0x10`, size at `+0x20`,
    capacity at `+0x24`; the SSO test compares capacity against `0x10`)
  - `+0x28` mapped value — `F3::RegisterCommand` stores the command handler here (`meth_0x59f540` returns
    `node + 40`, and the accessor `0x59EC80` returns `[node+0x28]`)
  - `+0x2C _Color`, `+0x2D _Isnil`
- The F3 command registry is one statically-constructed instance at `0x70C668`
  (`F3::global_cls_0x4d8d70_0x70c668`). It is **not** heap-allocated and can never be NULL:
  `F3::GLOBAL__I_command_registry` (`0x643f60`, list-B slot 624 of 890 in the hand-rolled static-init walker
  reached from the entry point via `F3::crt_run_static_ctors` at `0x61898D`) does
  `mov ecx, offset 0x70C668; call ctor_0x59f500; push 0x646b90; call _atexit`. There is no `.CRT`/`.initterm`
  section in this PE, which is why the construction shows up as explicit `_GLOBAL__I_` stubs instead.
- Constructor side effects, in order: `0x59f508 [this]=AL`, `0x59f50f mbr_0x4 = buynode()`,
  `0x59f512 node[0x2D] = 1`, `0x59f519 _Parent = self`, `0x59f51f _Left = self`, `0x59f524 _Right = self`,
  `0x59f527 mbr_0x8 = 0`. The `AL` byte is loaded with `push ecx; mov al,[esp+3]`, i.e. from a caller stack
  slot below the return address; the only shipped caller (the `_GLOBAL__I_` stub) sets nothing up, so the
  value is unspecified and is deliberately not reproduced.
- Destructor (`0x59f4d0`, reached through the `_atexit` thunk `0x646b90`): `_Erase(header->_Left, header)` via
  `0x59f0b0`, then `free(header)` at `0x59f4eb`, then `mbr_0x4 = 0` / `mbr_0x8 = 0`.
- `0x59EC80` is a lookup accessor, **not** a constructor: it calls `meth_0x545710` on the global and returns
  `[node+0x28]`, or 0 when the result equals the `byte_70C66C` sentinel.
- `buynode_0x59ec40` is shared engine-wide; it is also the tree-B node allocator of `JE::ModelCache`
  (see `/docs/je/ModelCache.md`).

### Implementation status in the reimplementation

- `F3::g_commandRegistryRoot` (`src/F3.hpp`, `src/F3.cpp`) is now a namespace-scope `JE::cls_0x4d8d70`
  **object**, matching the original: MSVC runs the reconstructed constructor before `WinMain` and registers
  the destructor with `atexit`. Previously it was a `cls_0x4d8d70*` initialised to `NULL`, which made
  `F3::RegisterCommand("quit", F3::Quit)` fault on a read of `0x28`.
- `modules/openje/src/JE/cls_0x4d8d70.hpp` declares its members in ascending offset order. The previous
  declaration sorted them alphabetically, which silently moved `mbr_0x4` to offset `0x28` — that offset is
  what appeared in the original crash report.
- Still open in `cls_0x4d8d70.cpp`: the transcribed tree methods index the node with `int*` where the binary
  uses **byte** offsets, so e.g. the `_Isnil` test reads `node+0xB4` instead of `node+0x2D`, the key capacity
  `node+0x90` instead of `node+0x24`, and the key buffer `node+0x40` instead of `node+0x10`. This is the
  current cause of the follow-on page fault on read access to `000000B4` at the second iteration of the
  `meth_0x545710` walk. The correct table is in the comment on `0x545710` in the original IDB.
- Also still open: `F3::RegisterCommand` writes the handler through the pointer returned by `meth_0x59f3e0`
  (i.e. at `node+0x00`) instead of calling the original's `meth_0x59f540`, which returns `node+0x28`.
  `meth_0x59f0b0` / `meth_0x59ece0` (element destruction on teardown) are not implemented, so the destructor
  leaks inserted elements.

### Open questions / hypotheses

- HYPOTHESIS (not confirmed): the database lists `cls_0x4d8d70` as a base class of ~15 unrelated types
  (`JE::G3D_Water`, `JE::cls_0x40a220`, `JE::cls_0x4dff90`, `JE::cls_0x6059e1`, `VFXEFFECTTYPE_*`). That is
  most likely auto-analysis attaching the same tree blob rather than real inheritance; none of them were
  verified.
- Exact template arguments of the tree. The key is a case-insensitive `std::string` (`_stricmp`); the value
  is written by `F3::RegisterCommand` as a `void (__stdcall *)()` handler read back at `node+0x28`.
- `meth_0x59f3e0`'s parameter string offsets differ from `meth_0x545710`'s (`param+0x10` vs `param+0x04` for
  the key buffer); re-confirm against the disassembly before transcribing.

[Return to `JE`](/docs/je.md)
