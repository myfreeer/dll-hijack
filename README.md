# dll-hijack
[![Build status](https://ci.appveyor.com/api/projects/status/a7hr2delcaiabnom?svg=true)](https://ci.appveyor.com/project/myfreeer/dll-hijack)

DLL hijack codes for cmake on windows.

## Usage
### Compile
Requirements: cmake, mingw gcc, ninja.
Use code below or simply run `./build.sh`

```bash
mkdir build
cd build
cmake -GNinja ..
ninja
```

### Generate code for existing system dll
Requirements: bash (msys2), gendef (mingw)

1. Run `gen_code.sh ${name}.dll`
2. Modify `${name}.c` with codes you want to execute when dll attached or detached.
3. Compile (check previous part)

## Credits
* <https://github.com/shuax/GreenChrome>
* <https://github.com/processhacker/phnt>
* <https://github.com/reactos/reactos>
* <https://www.winehq.org/>
