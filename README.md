[![License](https://img.shields.io/badge/license-GPL3-blue.svg)](LICENSE.txt)
[![Language](https://img.shields.io/badge/language-C%2FC%2B%2B-orange.svg)]()
# vmdconv

VMD animation converter to glTF 2.0.

Converts MikuMikuDance (MMD) motion data (`.vmd`) into a glTF 2.0 binary file (`.glb`), using a PMD/PMX model as the target skeleton. The tool simulates the animation frame by frame and bakes the resulting bone transforms into a standard glTF animation.

## Requirements

- **C++17** compiler (MSVC, Clang, or GCC)
- **Premake5**
- **vcpkg** for dependencies (set `VCPKG_ROOT` environment variable)
- **CMake** (optional, for some workflows)

Currently only **Windows x86_64** is tested. Other platforms may work with minor adjustments to the build script.

## Building

1. Clone the repository:
   ```bash
   git clone https://github.com/Alex9932/vmdconv.git
   cd vmdconv
   ```
2. Run Premake to generate project files:
   ```bash
   premake5 vs2022
   ```
   (or `premake5 gmake2` for GNU Make)
3. Build:
   - **Visual Studio**: open `build/vmdconv.sln` and build the solution.
   - **Make**:
     ```bash
     cd build
     make
     ```
     
The executable will be placed in `bin/Debug/` or `bin/Release/`.

## Usage

```
vmdconv -mdl <model.pmd> -vmd <animation.vmd> -o <output.glb>
```
- `-mdl` : Path to the PMD/PMX model file.
- `-vmd` : Path to the VMD animation file.
- `-o`   : Output glTF binary file (`.glb`).

### Example

```bash
vmdconv -mdl Miku_Hatsune.pmd -vmd wavefile_v2.vmd -o anim.glb
```

## Description

1. **Model loading**: The PMD/PMX model is parsed to create an internal skeleton representation.
2. **Animation loading**: The VMD file is read and bone/keyframe data are extracted.
3. **Simulation**: A fixed‑timestep simulation (1/60 s) steps through the animation. At each frame, the bone transformations are updated according to the VMD keyframes and the model's hierarchy.
4. **Export**: The final bone transforms are written into a glTF 2.0 binary asset.

The tool prints the frame counter during simulation and reports the total error count at the end.

## Limitations

- Only a single model/anim pair can be processed per execution.
- The exported glTF contains purely skeletal animation; no mesh, material, or texture data is written.

## Acknowledgments

- Built with love for the Talos‑II pioneers. xD
- Uses the [cJSON](https://github.com/DaveGamble/cJSON) library for JSON handling.
- Inspired by the MikuMikuDance community.

## License
GPL 3.0, see [LICENSE.txt](LICENSE.txt) for details.