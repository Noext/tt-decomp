# 001 — A native Linux and Windows executable for Tourist Trophy

Plan only: no code is written by this finding. It states what already exists (measured), the four
gaps that stop a Windows build today, and an ordered set of steps, each with a criterion that can be
checked by running something.

The runtime is `snuri00/ps2-web-recomp` (MIT). Tourist Trophy (Europe) already runs on it: the
generated tree in `generated/tt/` is 291 translation units, `build/native-tt/ps2run` renders real
frames offline, and the exploring run reaches the game's menus.

---

## 1. What exists, measured

| Fact | Evidence |
|---|---|
| The native runner builds and runs | `build/native-tt/ps2run`, 62,041,672 bytes; 3,600 frames in 2.21 s wall, 242 MB peak RSS |
| It renders without a window | `--frames-dir out --frame-every 300` wrote `frame_00300/600/900.ppm`, 640×512, 987,192 distinct-colour samples |
| It uses the GPU path | `-DPS2_GPU` in `build/native-tt/build.ninja`; log `[gpu] NVIDIA GeForce RTX 4060 Laptop GPU`; `[gs] 105886 primitives drawn` at 900 frames |
| It has **no** window on this machine | `CMakeCache.txt`: `SDL2_DIR:SDL2_DIR-NOTFOUND`; `nm ps2run` finds no `open_window`/`run_window`; no `SDL_Init` string in the binary |
| The window is one file, and one file only | `host/window.cpp` is the sole file in the repository (outside `build/`, `tools/`, `third_party/`) containing `SDL_*` |
| The window is optional at configure time | `CMakeLists.txt:119-123`: `find_package(SDL2 QUIET)` → `PS2_WINDOW` + `host/window.cpp` + `SDL2::SDL2` |
| GPU memory stays on the GPU | GS 4 MB in swizzled layouts, rasterised by WebGPU compute shaders (`README.md:25`); wgpu-native `v29.0.1.1` vendored at `third_party/wgpu-native` |
| Three native threads already exist | `host/window.cpp:415-427`: render thread, VU1 thread, emulation thread; `PS2_ONE_THREAD=1` collapses them |
| The threading layer is portable | `runtime/src/gs/queue.cpp:64-95` uses a futex on Linux and `std::atomic::wait`/`notify_all` everywhere else |
| The disc reader is portable | `runtime/src/disc.cpp:65-84` has `_WIN32` branches (`_fseeki64`, `_ftelli64`); everything else is `fread` |
| The memory card is a portable file | 528 × 0x4000 = **8,650,752 bytes** (`runtime/src/iop/sio2.cpp:48`), read with `std::ifstream` (:176), written with `std::ofstream` (:337) |
| PAL comes from the game, not a flag | `runtime/src/hw.cpp:181`: `if (!g_options.rate_forced) g_options.pal = mode == 3u;`; the TT log shows `[gs] SetGsCrt interlace 1 mode 0x3 field 1`, so 50 Hz by itself |
| Audio is a plain sink | `spu2.hpp:22-23`, `spu2.cpp:586`: `set_sink(function<int16_t*, frames>)`, called with **32-frame** stereo batches at 48 kHz (`spu2.cpp:24`, `:333`) |
| The exploring run is clean | 146 log lines, 0 matching `unhandled\|missing\|unknown\|unsupported\|error\|warn\|fail`; 14 IRX modules loaded; 36 thread dumps |
| Windows binary of wgpu-native is published | release `v29.0.1.1` ships `wgpu-windows-x86_64-msvc-release.zip` (9.12 MB `wgpu_native.dll`, 54.81 MB `.lib`) and a `gnu` variant |
| `Primary` backends include DX12 | `webgpu/wgpu.h:808`: Vulkan \| Metal \| DX12 \| BrowserWebGPU — no code change needed to reach DX12 |

## 2. The four gaps

1. **No Windows surface.** `SurfaceTarget::Kind` is `{ X11, Wayland, Canvas, Metal }`
   (`runtime/src/gs/gpu/gpu.hpp:18`) and `attach_surface` (`runtime/src/gs/gpu/gpu.cpp:1701`) has no
   Win32 branch, although the vendored header defines `WGPUSurfaceSourceWindowsHWND`
   (`webgpu/webgpu.h:3260`). On Windows the renderer would create a device and then fail to attach a
   surface, falling back to the software rasteriser.
2. **SDL2, and the port is not a rename.** `host/window.cpp:85` uses `SDL_GetWindowWMInfo` and
   `SDL_syswm.h`; that header is **removed** in SDL3, the same information now arriving as window
   properties. Also gone: `SDL_QueueAudio`, `SDL_GetQueuedAudioSize`, `SDL_PauseAudioDevice(dev, 0)`,
   `SDL_GameController*`, `SDL_NumJoysticks`, `SDL_IsGameController`, `SDL_WINDOW_FULLSCREEN_DESKTOP`,
   `SDL_WINDOW_ALLOW_HIGHDPI`. Verified against upstream `SDL3` (release 3.4.20 headers).
3. **POSIX-only host code.** `host/window.cpp` reads `getrusage` (:292), `/proc/self/statm` (:304),
   `sysconf` (:308) and calls `std::_Exit` (:432). None has a `_WIN32` branch.
4. **Build plumbing assumes `uname`.** `scripts/fetch_wgpu.sh` maps `uname -s` to `linux`/`macos`
   only, and no SDL3 development package exists on Ubuntu 24.04 (`apt-cache` offers `libsdl2-dev`
   2.30.0, no `libsdl3-dev`), so the Linux side must use the upstream SDL3 release archive or a
   source build.

## 3. What GT4Recomp actually offers

`gt4-staticrecomp` is a **host shell**, not a runtime: the repository contains no `runtime/` and no
recompiler; its own `CMakeLists.txt` builds `native/*.cpp` only. Its stack is also different from
ours: raw **Vulkan** (`native/native_vulkan.cpp`, 57.4 KB, compute shaders) rather than WebGPU, a
**Win32** window (`CreateWindowExW`/`WndProc`, `native/window.cpp:194`) rather than SDL, and
**WinMM** audio (`waveOutOpen`, 48 kHz 16-bit stereo, `native/host_io.cpp:298`). SDL3 appears there
only for gamepads, pinned at 3.4.18 (`CMakeLists.txt:675-693`).

So its GS, its window and its audio cannot be lifted onto our renderer. What is genuinely reusable is
its **host design**, and three pieces are worth copying by description:

- `native/pacing.hpp` — a frame pacer that stores the deadline scaled by the rate, so 10,000 frames
  accumulate no drift; lateness under 1 ms is treated as jitter and not counted; a full period of
  lateness restarts the grid instead of replaying missed frames.
- `native/pacing.hpp` — a PI controller on the audio queue (target 2,880 frames = 60 ms, gains
  0.03/0.002, clamp ±0.5 %) driving a linear resampler, so audio *rate* corrects clock drift instead
  of the emulation thread blocking on the device queue.
- `installer/gt4recomp.nsi` — an NSIS package, which is how its Windows build ships at all.

## 4. Steps

Order is deliberate: SDL3 first, because it can be verified **on this machine** before anything
Windows-specific is written.

### S0 — Freeze a baseline
Record, in `docs/findings/002-native-baseline.md`: the three PPm frames from `--frames-dir`, their
SHA-256, the 146-line log, and the 3,600-frame wall time.

*Criterion:* two consecutive runs produce byte-identical PPms and identical log line counts. Nothing
after this step is accepted unless it leaves those frames unchanged.

### S1 — SDL3 window on Linux
Rewrite `host/window.cpp` against SDL3 3.4.x: `SDL_GetWindowProperties` +
`SDL_PROP_WINDOW_X11_DISPLAY_POINTER` / `SDL_PROP_WINDOW_X11_WINDOW_NUMBER` (and the Wayland pair)
instead of `SDL_GetWindowWMInfo`; `SDL_OpenAudioDeviceStream` + `SDL_PutAudioStreamData` +
`SDL_GetAudioStreamQueued` instead of the queue API; `SDL_Gamepad*` names; `SDL_SetWindowFullscreen(win, bool)`;
`SDL_WINDOW_HIGH_PIXEL_DENSITY`; `SDL_SetTextureScaleMode`. CMake: `find_package(SDL3 CONFIG)`,
target `SDL3::SDL3`, `PS2_WINDOW`.

*Criterion:* `--window` opens, the title line updates, and at 900 and 1,800 frames the GG-frame hash
equals S0's. Audio: `SDL_GetAudioStreamQueued` oscillates around its target for 60 s without
underruns.

### S2 — Win32 surface in the renderer
Add `Windows` to `SurfaceTarget::Kind` and a `#if defined(_WIN32)` branch in `attach_surface` filling
`WGPUSurfaceSourceWindowsHWND { hinstance, hwnd }`. `host/window.cpp` passes
`SDL_PROP_WINDOW_WIN32_HWND_POINTER` and `SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER`.

*Criterion:* on Linux the X11 path still presents and S0's frames are unchanged (the new branch is
compiled out); the Win32 branch compiles under `_WIN32` in the cross check.

### S3 — Wall clock and audio that do not block the guest
Replace the busy-wait in `pace()` (`host/window.cpp:274-276`, waits while > 80 ms queued) with
GT4Recomp's split: the emulation thread releases against a frame grid derived from
`ps2::frame_cycles()` — already PAL-aware through `hw.cpp:181` — and the audio sink pushes into a
lock-free ring of ~10 ms that a resampler drains into the SDL3 stream, with the PI controller nudging
the ratio inside ±0.5 %.

*Criterion:* a 60 s run at 50 fps ±1 %, guest frame count equal to within 1 frame of wall seconds × 50,
`SDL_GetAudioStreamQueued` inside target ±20 %, zero underruns logged. With `PS2_ONE_THREAD=1` the same
run must still pass with pacing off.

### S4 — Input
Keep the existing key table and pad mapping (`host/window.cpp:38-57`, `:157-205`) but move it behind a
small `host/input.cpp` with the SDL3 names, and keep `--press frame:button` working headlessly.

*Criterion:* `--press 300:start --press 600:cross` produces the same sequence of `set_pad_buttons`
transitions as S0; a physical gamepad produces the same transitions for the same presses; rumble
reaches `SDL_RumbleGamepad`.

### S5 — Local ISO and memory card
`disc::open` and the card reader already compile for Windows. The work is to prove it and to make
the failures legible: a missing ISO, a read past the end, and a card that is present but unformatted
must each print one line and stop, not fault.

*Criterion:* a 2.96 GiB ISO at a path with spaces and accents opens; the card file is created at
8,650,752 bytes on first save, reloaded on the next start, and its directory entries re-read
identically; deleting the ISO gives `[ee] halted: <reason>`, not a crash.

### S6 — Windows build and package
A `PS2_WINDOWS` branch in CMake with `win32`-guard: link `wgpu_native.lib` (MSVC) or
`libwgpu_native.a` (MinGW), copy `wgpu_native.dll` next to `ps2run.exe`, add a windres icon, and an
NSIS script modelled on `installer/gt4recomp.nsi`. `scripts/fetch_wgpu.sh` gains a Windows case and
verifies the archive's SHA-256 against a pinned value.

*Criterion:* `ps2run.exe` runs on Windows 10/11 with a clean profile (no SDL, no Vulkan SDK, no VC++
redistributable installed by hand), presents through DX12 **or** Vulkan and reports which, and
produces the same frame hash as S0 at 900 frames.

Local pre-check available here: Wine 9.0 with `winevulkan.so` is installed, and
`x86_64-w64-mingw32-gcc` 13 exists — but **no** `x86_64-w64-mingw32-g++` and no `cc1plus`, so a MinGW
C++ build cannot be produced on this machine as configured. The honest options are an MSVC or MinGW
toolchain somewhere else, or installing the C++ half of the MinGW package.

### S7 — Tourist Trophy without our tools
`scripts/port.sh:88-91` recognises `CORE.GT4` only. A TT user must first run our verified
`tools/core2elf.py` (Salsa20 + raw deflate, `docs/STATE-EU.md`) to get `core.elf`.

*Criterion:* a fresh clone plus the retail ISO, one command, reaches the game's menus with no manual
step, on Linux and on Windows.

## 5. Non-goals, and why

- **Replacing WebGPU with Vulkan** as GT4Recomp did. wgpu-native already reaches Vulkan and DX12
  through `WGPUInstanceBackend_Primary`; a second renderer would be a rewrite of 86 KB of GS compute
  shaders for no measured gain.
- **A second window layer.** `host/window.cpp` is the only SDL file; keeping it that way is what
  makes "Linux and Windows" one file each of `#if`.
- **Changing `generated/`.** Translation output is game-agnostic and already correct; every step above
  is host-side or renderer-surface-side.

## 6. Risks

| Risk | Mitigation |
|---|---|
| SDL3 API drifts between 3.4.x releases | Pin one version in CMake, as GT4Recomp pins 3.4.18 |
| DX12 path of wgpu-native is less exercised than Vulkan | `PS2_GPU_BACKEND`-style override; log the chosen adapter, as the current `[gpu]` line does |
| 32-frame audio batches (0.67 ms) are too fine for a device stream | S3 buffers to ~10 ms in the ring; the sink contract (`spu2.hpp:22`) does not change |
| Frame hash is sensitive to GPU driver maths | Compare hashes only within one machine and driver; across machines compare the log and primitive counts |
| No Windows machine in this environment | S2 compiles the branch; S6 needs a real or virtual Windows box to accept |
