"""Point pyproject.toml at one CUDA major.

One repository builds one distribution per CUDA major: rexlib-cuda12
depends on cuda-toolkit 12, rexlib-cuda13 on 13, and so on. Both the
distribution name and the toolkit constraint are rewritten here, so a
build only has to say which major it is for.

Edits are made line by line rather than through a TOML round trip, which
would reformat the file and drop every comment in it.
"""

import argparse
import pathlib
import re

NAME = re.compile(r'^(name\s*=\s*"[^"]*?)\d*(")')
TOOLKIT = re.compile(r'^(\s*"cuda-toolkit\[[^\]]+\]==)\d+(\.\*")')


def patch(text: str, major: int) -> str:
	"""Rewrite the distribution name and the CUDA toolkit constraint."""
	out = []
	for line in text.splitlines(keepends=True):
		if NAME.match(line):
			line = NAME.sub(rf"\g<1>{major}\g<2>", line)
		else:
			line = TOOLKIT.sub(rf"\g<1>{major}\g<2>", line)
		out.append(line)
	return "".join(out)


def main() -> None:
	"""Parse the arguments and patch the file."""
	parser = argparse.ArgumentParser(description=__doc__)
	parser.add_argument("-i", "--input", required=True, type=pathlib.Path)
	parser.add_argument(
		"-o", "--output", type=pathlib.Path,
		help="Defaults to editing the input in place.",
	)
	parser.add_argument(
		"--cuda-major", required=True, type=int,
		help="CUDA major version this build targets, e.g. 12.",
	)
	args = parser.parse_args()

	output = args.output or args.input
	output.write_text(patch(args.input.read_text(), args.cuda_major))


if __name__ == "__main__":
	main()
