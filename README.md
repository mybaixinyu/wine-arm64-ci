# wine-arm64-ci

Rebuilds single modules of the native arm64 Wine from
[athei/wine-build](https://github.com/athei/wine-build) (release
`arm64-11.19-1`, built from `athei/wine@e2e42653b41`) with the patches in
`patches/`, to replace the same files in an installed bundle.

## Patches

- `0001-wowsilicon-audio-follow-system-device.patch`: WoWSilicon's
  `0006-dsound-follow-default-playback-device` and
  `0007-winecoreaudio-add-spatialized-stereo`
  ([WoWSilicon/WoWSilicon](https://github.com/WoWSilicon/WoWSilicon),
  `Packaging/WineRuntime/patches/`), as adapted to Wine 11.19 and rebased onto
  `athei/wine@e2e42653b41`. Output streams follow the macOS default output
  device (for example a Bluetooth headset connected while a game runs), and
  DirectSound reopens its default playback device when it changes. It also
  carries WoWSilicon's spatial stereo and loudness normalization, both off
  unless `WOWSILICON_SPATIAL_AUDIO_MODE` or `WOWSILICON_NORMALIZE_AUDIO` turn
  them on.

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
| `patched/dlls/dsound/i386-windows/dsound.dll` | `lib/wine/i386-windows/dsound.dll` |
