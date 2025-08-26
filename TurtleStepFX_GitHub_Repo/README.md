# TurtleStep FX (JUCE VST3)

An **Effectrix‑inspired** step‑sequenced multi‑effect for Windows, built with JUCE 7 and CMake.

> Legal note: This is an *original* work inspired by the concept of step‑sequenced effects. It does **not** copy
> Sugar Bytes' Effectrix code, UI assets, names, or presets.

## Features
- 16‑step sequencer synced to host tempo (1/16 grid).
- 6 effect lanes: Gate, Stutter, Filter, Bitcrush, Delay, Reverb.
- Per‑lane enable grid: click steps to toggle effect on that slice.
- Global Wet/Dry mix and Output Gain.
- Basic, clean UI with resizable editor.

## Build (Windows)
1. Install **Visual Studio 2022** (Desktop C++), **CMake 3.22+**, and optionally **Ninja**.
2. Ensure you have internet access during first configure (JUCE is fetched automatically).
3. In a VS Developer PowerShell:
   ```sh
   cd "/mnt/data/TurtleStepFX"
   cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
4. The plugin is produced at:
   `build/TurtleStep_FX_artefacts/Release/VST3/TurtleStep FX.vst3`

> If you prefer the Visual Studio generator:
> ```sh
> cmake -B build -S . -G "Visual Studio 17 2022" -A x64
> cmake --build build --config Release
> ```

## Installing in FL Studio
- Copy the `.vst3` folder to `C:\Program Files\Common Files\VST3`.
- In FL Studio, do **Options → Manage plugins → Find Plugins**.
- Load **TurtleStep FX** on an insert slot.

## Controls
- **Top bar**: Wet/Dry, Output Gain.
- **Grid**: 6 rows (one per effect), 16 columns (steps). Click to toggle.
- **Tempo sync**: Each step is one 1/16 note at the host BPM.
- **Effects (per step when active)**:
  - Gate: hard gate (silence) for the slice.
  - Stutter: retriggers the slice at 1/8 subdivisions within the step.
  - Filter: 24dB low‑pass at 1.2kHz with moderate resonance.
  - Bitcrush: 8‑bit / 22kHz downsample (simple decimation).
  - Delay: 1/8 note feedback delay.
  - Reverb: small room algorithm (JUCE Reverb).

> This is a minimal demo. You can expand it with per‑lane parameters, presets, swing, randomize, etc.

## License
MIT for this template. JUCE is licensed separately.
