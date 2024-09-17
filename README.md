# OpenJE
### Reverse engineered and open source Jefferson Engine

## Disclaimer
- This project is not affiliated with Bethesda Softworks LLC, Interplay Entertainment Corp., Bethesda Game Studios, or Black Isle Studios.
- This project is completely open source and no commercial use is intended.
- All copyrights belong to their respective owners.

## Description
- This project is the reverse engineered implementation of the Jefferson Engine used in the cancelled Fallout 3 project, codenamed Van Buren. The goal of this project is to rewrite the engine in c++ as close to the original source code as possible.

- Progress is ongoing, but alot of work still needs to be done.

- Once the engine is fully reverse engineered, the goal is to swap out the win32 and directx8 api calls with cross platform implementations/libraries, and add/complete the missing features of the engine.

## Documentation
- Documentation for the structs, classes, and functions of the Jefferson Engine: [`Documentation`](/docs/je.md)

## Contributing
- If you want to contribute to this project, contact me on **Discord**: **shenmarukai** or by **Email**: **shanemulc@comcast.net**. A lot of work still needs to be done, and any help is appreciated.

## Building
- The project is currently being compiled using Microsoft Visual Studio .NET 2003 on Windows XP to replicated the original build process of the van buren tech demo. But it should compile on any modern version of Visual Studio as well.

## Information
- I use both IDA and Ghidra to reverse engineer, using the differences between the decompilers to better understand the code. Because of this the reverse engineered code may look a bit different from either/both of the decompilers outputs.

## OpenVB - Fallout: Van Buren
- [`OpenVB`](https://github.com/OpenJE/openvb)
