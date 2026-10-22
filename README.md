# ARABESQUE — Turkish Emotional Synthesizer

A JUCE polyphonic synthesizer plugin inspired by the sound of modern Turkish pop
and arabesk production. Dark-gold interface, PolyBLEP anti-aliased oscillators,
a real resonant TPT filter, three envelopes, LFO, four FX units and a
tempo-synced arpeggiator.

## Formats

| Platform | Format | Architecture |
|----------|--------|--------------|
| Windows  | VST3   | x64          |
| macOS    | AU + VST3 | Universal (arm64 + x86_64) |

## Build with GitHub Actions (recommended)

1. Create a GitHub repository and push these files (default branch `main` or `master`).
2. Open **Actions → Build ARABESQUE → Run workflow** (it also runs on every push).
3. Download the artifacts when the run finishes:
   - `ARABESQUE-Windows-x64-VST3` → contains `ARABESQUE.vst3`
   - `ARABESQUE-macOS-AU-VST3` → contains `ARABESQUE.component` (AU) and `ARABESQUE.vst3`

JUCE 8.0.9 is fetched automatically and cached between runs.

> The builds are unsigned. On Windows/macOS you may need to approve the plugin
> the first time the DAW scans it. For distribution, add code signing /
> notarization to the workflow later.

### Install paths

- Windows VST3: `C:\Program Files\Common Files\VST3\`
- macOS AU: `~/Library/Audio/Plug-Ins/Components/`
- macOS VST3: `~/Library/Audio/Plug-Ins/VST3/`

## Local build

Install a C++17 compiler and CMake 3.22+, then clone JUCE next to this folder
(or pass its path):

```bash
cmake -S . -B build -DJUCE_DIR=/path/to/JUCE -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Your plugins appear in `build/ARABESQUE_artefacts/Release/`.

## Features

- **2 main oscillators** (Saw / Square / Triangle / Sine) with octave, semitone
  and fine tuning — PolyBLEP anti-aliasing
- **Sub oscillator** (sine, −1 oct) + **noise** source, 4-slot mixer in dB
- **TPT state-variable filter**: LP 6 / 12 / 24 dB, HP 12, BP 12 with true
  resonance and key tracking
- **3 ADSR envelopes** (amp / filter / pitch) edited from one tabbed panel
- **LFO** (5 shapes) with host-tempo sync, routed to vibrato (+ mod wheel)
- **FX chain**: Chorus → Phaser → tempo-synced Delay → Reverb (Hall / Plate /
  Room / Cathedral), with master drive, pan and output level
- **Arpeggiator**: tempo sync, 8-step gate pattern, 1–4 octave cycling
- **16 factory presets** with prev/next navigation, browse menu and user-preset
  saving (`SAVE` button)
- Up to **32 voices**, glide in the musical pitch domain, pitch wheel ±2 st,
  velocity sensitivity
- Resizable UI (fixed aspect ratio)

## State & presets

DAW sessions store the full parameter state plus the current preset. User
presets are XML files saved to the OS app-data folder
(`ARABESQUE Audio/Presets/*.arabesque`) and show up under **BROWSE → User**.
