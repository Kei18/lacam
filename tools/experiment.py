import argparse
import itertools
import logging
import subprocess
import pandas as pd
import yaml

from pathlib import Path
from typing import TypedDict, List

BASE_PATH = Path(__file__).absolute().parent

LOG = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format="%(message)s")

class ExperimentParameters(TypedDict):
    map: str
    cache: List[str]
    ngoals: List[int]
    goals_k: List[int]
    goals_m: List[int]
    nagents: List[int]
    seed: int  # Assuming a single value for simplicity, modify as needed.
    time_limit_sec: int  # Assuming a single value for simplicity, modify as needed.
    output_step_result: str
    output_csv_result: str
    log_short: bool
    debug: bool

def load_experiment(exp_name: str) -> ExperimentParameters | None:
    exp_path = BASE_PATH / "experiment" / f"{exp_name}.yaml"
    if not exp_path.exists():
        LOG.error(f"Experiment file {exp_path} not found.")
        return None

    with open(exp_path) as f:
        return yaml.safe_load(f)

def generate_combinations(params: ExperimentParameters):
    keys = params.keys()
    values = (params[key] if isinstance(params[key], list) else [params[key]] for key in keys)
    for combination in itertools.product(*values):
        yield dict(zip(keys, combination))

def run_experiment(params: ExperimentParameters, dry_run: bool = False):
    cmd_base = [
        "./build/main",
        "--map", params["map"],
        "--cache", params["cache"],
        "--ngoals", str(params["ngoals"]),
        "--goals-k", str(params["goals_k"]),
        "--goals-m", str(params["goals_m"]),
        "--nagents", str(params["nagents"]),
        "--seed", str(params.get("seed", 0)),
        "--time_limit_sec", str(params.get("time_limit_sec", 10)),
        "--output_step_result", params.get("output_step_result", "./result/step_result.txt"),
        "--output_csv_result", params.get("output_csv_result", "./result/result.csv")
    ]
    if params.get("log_short", False):
        cmd_base.append("--log_short")
    if params.get("debug", False):
        cmd_base.append("--debug")

    if dry_run:
        LOG.info(f"Dry run command: {' '.join(cmd_base)}")
    else:
        LOG.info(f"Executing: {' '.join(cmd_base)}")
        subprocess.run(cmd_base, check=True)

def main():
    parser = argparse.ArgumentParser(description="Run lacam experiments with different parameters.")
    parser.add_argument("experiment", help="Experiment name to run.")
    parser.add_argument("--dry-run", action="store_true", help="Perform a dry run without executing commands.")
    args = parser.parse_args()

    exp_params = load_experiment(args.experiment)
    if exp_params is None:
        return

    for combination in generate_combinations(exp_params):
        try:
            run_experiment(combination, args.dry_run)
        except subprocess.CalledProcessError as e:
            LOG.error(f"Experiment failed with error: {e}")

if __name__ == "__main__":
    main()
