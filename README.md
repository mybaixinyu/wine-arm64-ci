# wine-arm64-ci

Rebuilds single modules of the native arm64 Wine from
[athei/wine-build](https://github.com/athei/wine-build) (release
`arm64-11.19-2`, built from `athei/wine@d57b522eea5`) with the patches in
`patches/`, to replace the same files in an installed bundle.

## Patches

`0001`–`0006` come from [WoWSilicon/WoWSilicon](https://github.com/WoWSilicon/WoWSilicon)
(`Packaging/WineRuntime/patches/`, its Wine 11.13 runtime), rebased onto
`athei/wine@e2e42653b41`. They apply in order.

| Patch | WoWSilicon original | What it does |
|-------|---------------------|--------------|
| `0001-wowsilicon-audio-follow-system-device` | `0006` + `0007` | Output streams follow the macOS default output device (a Bluetooth headset connected while a game runs) and DirectSound reopens its default playback device when it changes. Also carries WoWSilicon's spatial stereo and loudness normalization, which `WOWSILICON_SPATIAL_AUDIO_MODE=off` and `WOWSILICON_NORMALIZE_AUDIO=0` turn off |
| `0002-wowsilicon-winemac-use-input-source-language-in-hkl` | `0005` | The keyboard layout handle carries the language of the macOS input source |
| `0003-wowsilicon-wininet-skip-legacy-game-alert-requests` | `0008` | `GET .../alert` requests of legacy game clients fail at once instead of waiting on the network; `WOWSILICON_BLOCK_LEGACY_ALERTS=0` turns it off |
| `0004-wowsilicon-winemac-ignore-momentum-scroll-events` | `0011` | Trackpad momentum scrolling no longer sends wheel events, and continuous scrolling is quantized into wheel clicks |
| `0005-wowsilicon-winemac-add-gcmouse-raw-input` | `0012` | Raw mouse movement from GameController's GCMouse; the Mac driver option `UseGCMouse=N` turns it off |
| `0006-wowsilicon-winemac-normalize-toplevel-client-surface-origin` | `0004` | A top-level window's client surface view starts at its own origin |

`0007-winemac-place-the-input-method-window-for-programs-that-never-do` is not
WoWSilicon's: a program that draws its own caret and never calls
`ImmSetCompositionWindow` (WoW) left the macOS input method window at the
screen's top-left corner. It now goes to a fixed point of the window, by
default 70,205 points from the content's bottom-left, just below WoW's chat edit
box. The Mac driver option `IMEFallbackPosition` takes `x,y` to move it, or
`N` to turn it off.

`0008-winemac-show-the-dock-once-resigned-not-while-resigning` is not
WoWSilicon's either: athei/wine hides the Dock and menu bar through the
presentation options while a window covers the screen, and set them back
from applicationWillResignActive. The window server then made the game the
front process again (`[SetMenuBars]` in its log), so Command-Tab out of a
fullscreen game often bounced back. They are now set back from
applicationDidResignActive. Not setting them back at all is not enough: a
native probe shows AppKit ignores `-miniaturize:` while an inactive
application still has them, and setting them once it is inactive does not
undo that, so a game that minimizes itself when it loses the focus would
stay on the screen. `tests/minimize-on-deactivate.c` is such a program.

Not carried, since this Wine has no use for them or already has them:
`0001` (fixes WineAndAqua's DXMT glue, which this tree does not have),
`0002` (relaxes wined3d's Vulkan feature level check for MoltenVK; this bundle
runs wined3d on OpenGL or KosmicKrisp), `0003` (attaches Rosetta's x87sidecar;
this Wine runs x86 code through FEX), `0009` (superseded upstream by
`cgDisplay:wasReconfiguredWithFlags:`), `0010` (athei/wine has its own fix for
cursor hide/show present stalls) and `0013` (removes WineAndAqua's `~/Wine`
link, which this tree never creates).

## Run

```bash
gh workflow run build.yml --repo mybaixinyu/wine-arm64-ci -f wine_ref=<full athei/wine SHA>
```

The artifact `wine-arm64-modules` holds each module built without
(`unpatched/`) and with (`patched/`) the patches from the same tree, plus
`BUILD-INFO.txt`. Replace in the bundle:

| Artifact path | Bundle path |
|---------------|-------------|
| `patched/dlls/winecoreaudio.drv/winecoreaudio.so` | `lib/wine/aarch64-unix/winecoreaudio.so` |
| `patched/dlls/winemac.drv/winemac.so` | `lib/wine/aarch64-unix/winemac.so` |
| `patched/dlls/dsound/i386-windows/dsound.dll` | `lib/wine/i386-windows/dsound.dll` |
| `patched/dlls/wininet/i386-windows/wininet.dll` | `lib/wine/i386-windows/wininet.dll` |

Only the i386 PE modules are built, for 32-bit programs; 64-bit programs keep
the bundle's unpatched ARM64X `dsound.dll` and `wininet.dll`.
