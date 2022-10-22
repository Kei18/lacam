lacam
---
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
[![CI](https://github.com/Kei18/lacam/actions/workflows/ci.yml/badge.svg)](https://github.com/Kei18/fast-mapf/actions/workflows/ci.yml)

The code repository of the paper ["LaCAM: Search-Based Algorithm for Quick Multi-Agent Pathfinding"](https://kei18.github.io/lacam).

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
build/main -i assets/random-32-32-10-random-1.scen -m assets/random-32-32-10.map -N 50 -v 1
```
The result will be saved in `build/result.txt`.

<details><summary>Output File</summary>

This is an example output of `random-32-32-10-random-1.scen`.
`(x, y)` denotes location.
`(0, 0)` is the left-top point.
`(x, 0)` is the location at `x`-th column and 1st row.

```
agents=50
map_file=random-32-32-10.map
solver=planner
solved=1
soc=1316
soc_lb=1113
makespan=55
makespan_lb=53
sum_of_loss=1191
sum_of_loss_lb=1113
comp_time=1
seed=0
starts=(11,6),(29,9),[...]
goals=(7,18),(1,16),[...]
solution=
0:(11,6),(29,9),[...]
1:(10,6),(29,10),[...]
[...]
```

</details>

You can find details of all parameters with:
```sh
build/main --help
```

## Visualizer

[@Kei18/mapf-visualizer](https://github.com/kei18/mapf-visualizer) is available.

## Experiments

The experimental script is written in Julia ≥1.7.
Setup may require around 10 minutes.

```sh
sh scripts/setup.sh
```

Edit the config file as you like.
Examples are in `scripts/config` .
The evaluation starts by following commands.

```
julia --project=scripts/ --threads=auto
> include("scripts/eval.jl"); main("scripts/config/mapf-bench.yaml")
```


## Notes

- The empirical data of the manuscript was obtained with [exp/AAAI2023](https://github.com/Kei18/lacam/releases/tag/exp%2FAAAI2023).
- The grid maps and scenarios in `assets/` are from [MAPF benchmarks](https://movingai.com/benchmarks/mapf.html).
- `scen-warehouse.zip` is obtained from [MAPF-LNS2](https://github.com/Jiaoyang-Li/MAPF-LNS2).
- `tests/` is not comprehensive. It was used in early developments.
- Auto formatting (clang-format) when committing:

```sh
git config core.hooksPath .githooks && chmod a+x .githooks/pre-commit
```

## Licence

This software is released under the MIT License, see [LICENSE.txt](LICENCE.txt).

## Author

[Keisuke Okumura](https://kei18.github.io) is a Ph.D. student at Tokyo Institute of Technology, interested in controlling multiple moving agents.
