lacam
---
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
[![CI](https://github.com/Kei18/lacam/actions/workflows/ci.yml/badge.svg)](https://github.com/Kei18/fast-mapf/actions/workflows/ci.yml)

The code repository of the paper "LaCAM: Search-Based Algorithm for Quick Multi-Agent Pathfinding."

## Building

All you need is [CMake](https://cmake.org/) (≥v3.16). The code is written in C++(17).

First, clone this repo with submodules.

```sh
git clone --recursive https://github.com/Kei18/lacam.git
cd lacam
```
Then, build the project.

```sh
cmake -B build && make -C build
```

### Docker

You can also use the docker environment (based on Ubuntu18.04) instead of the native one.

```sh
# ~10 min, mostly for CMake build
docker compose up -d
docker compose exec dev bash
> cmake -B build && make -C build
```

## Usage

```sh
build/main -i assets/scen/random-32-32-20-random-1.scen -m assets/map/random-32-32-20.map -N 50 -v 1
```
The result will be saved in `build/result.txt`.

<details><summary>Output File</summary>

This is an example output of `random-32-32-20-random-1.scen`.
`(x, y)` denotes location.
`(0, 0)` is the left-top point.
`(x, 0)` is the location at `x`-th column and 1st row.

```
agents=50
map_file=random-32-32-20.map
solver=planner
solved=1
soc=1489
soc_lb=1082
makespan=51
makespan_lb=48
comp_time=1
seed=0
starts=(5,16),(21,29),[...]
solution=
0:(5,16),(21,29),[...]
1:(5,17),(21,28),[...]
[...]
```

</details>

You can find details of all parameters with:
```sh
build/main --help
```

## Experiments

First, unzip instance files.

```sh
unzip assets/scen/mapf-scen-random.zip -d assets/scen
unzip assets/scen/scen-warehouse.zip -d assets/scen
```

The experimental scripts are written in Julia 1.7.
Please build the virtual environment.

```sh
julia --project=. -e 'using Pkg; Pkg.instantiate()'
```

*The docker image does not include Julia.

Finally, run the following scripts.

```sh
julia --project=. --threads=auto

# small complicated instance
> include("scripts/small-complex-eval.jl"); main()

# MAPF benchmark
> include("scripts/eval.jl"); main("scripts/config/mapf-bench.yaml")

# instances with massive agents
> include("scripts/eval.jl"); main("scripts/config/warehouse.yaml")
```

## Notes

- The grid maps and scenarios in `assets/` are from [MAPF benchmarks](https://movingai.com/benchmarks/mapf.html).
- The evaluation script is inspired by [Hydra](https://hydra.cc/).
- The experiment of design choices is not included. Those are in different branches.
- The repo was developed on macOS-10.15.
- `tests/` is not comprehensive. It was used in early developments.
- Auto formatting (clang-format) when committing:

```sh
git config core.hooksPath .githooks
chmod a+x .githooks/pre-commit
```

## Licence

This software is released under the MIT License, see [LICENSE.txt](LICENCE.txt).

## Author

[Keisuke Okumura](https://kei18.github.io) is a Ph.D. student at Tokyo Institute of Technology, interested in controlling multiple moving agents.
