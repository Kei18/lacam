A cache approach in warehouse based on LACAM algorithm

## Building

All you need is [CMake](https://cmake.org/) (≥v3.16). The code is written in C++(17).

First, clone this repo with submodules.

```sh
git clone --recursive https://github.com/HarukiMoriarty/lacam-test.git
cd lacam
```
Then, build the project.

```sh
cmake -B build && make -C build
```

## Assumption

1. Assume cargoes 