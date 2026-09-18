#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only

"""
Copy an extension's type stubs out of an installed package.

pybind11-stubgen renders a submodule holding submodules of its own as a
directory of stubs rather than as one file, so what it writes is a tree and
this carries the tree. Invoked from the workflow that generates the stubs
once and hands them to the builds that cannot generate their own.
"""

from __future__ import annotations

import argparse
import importlib
import pathlib
import shutil

def parse_arguments() -> argparse.Namespace:
	"""Read the module whose stubs to copy, and where to put them."""
	parser = argparse.ArgumentParser(description=__doc__)
	parser.add_argument("--module", required=True)
	parser.add_argument(
		"--output",
		required=True,
		help="Directory the stubs are copied to.",
	)
	return parser.parse_args()

def find_stubs(module: str) -> pathlib.Path:
	"""Locate the stub directory installed beside an extension."""
	package, _, name = module.rpartition(".")
	root = importlib.import_module(package)
	if root.__file__ is None:
		raise SystemExit(f"{package} has no location on disk")
	return pathlib.Path(root.__file__).parent / name

def copy_stubs(source: pathlib.Path, destination: pathlib.Path) -> int:
	"""Copy every stub under source, keeping the path it sits at."""
	copied = 0
	for stub in sorted(source.rglob("*.pyi")):
		target = destination / stub.relative_to(source)
		target.parent.mkdir(parents=True, exist_ok=True)
		shutil.copyfile(stub, target)
		print(target)
		copied += 1
	return copied

def main() -> None:
	"""Copy the stubs, failing when there is not one to copy."""
	arguments = parse_arguments()

	source = find_stubs(arguments.module)
	if not source.is_dir():
		raise SystemExit(f"No stub directory at {source}")

	if not copy_stubs(source, pathlib.Path(arguments.output)):
		raise SystemExit(f"No .pyi files under {source}")

if __name__ == "__main__":
	main()
