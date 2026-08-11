#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only

"""
Extract xmipp4-core's compiled artifacts from its release wheel.

Extracts the compiled artifacts (bin/include/lib) from an xmipp4-core
release wheel into a plain <prefix>/{bin,include,lib} directory, bypassing
pip entirely. Used on Windows, where pip refuses to install a wheel tagged
for a different platform than the current host (needed when cross-compiling
for ARM64 on an amd64 runner), and where scikit-build-core's own
CMAKE_PREFIX_PATH auto-detection doesn't account for xmipp4-core's
wheel.install-dir = "/data" layout. See deploy.yml for how this is wired up.
"""

import pathlib
import sys
import zipfile

ARCH_TAGS = {"AMD64": "win_amd64", "ARM64": "win_arm64"}


def main() -> None:
	"""Extract the release wheel matching argv and print the staged prefix.

	Only the final staged prefix goes to stdout, since the caller captures
	it to set CMAKE_PREFIX_PATH; everything else is logged to stderr so it
	still shows up in CI logs without corrupting that capture.
	"""
	wheel_dir, dest_dir, arch = sys.argv[1], sys.argv[2], sys.argv[3]
	tag = ARCH_TAGS[arch]
	wheel = next(pathlib.Path(wheel_dir).glob(f"*-py3-none-{tag}.whl"))
	dest = pathlib.Path(dest_dir)
	print(f"Staging {wheel.resolve()} into {dest.resolve()}", file=sys.stderr)

	extracted = 0
	with zipfile.ZipFile(wheel) as zf:
		for name in zf.namelist():
			if ".data/data/" not in name:
				continue
			rel = name.split(".data/data/", 1)[1]
			if not rel:
				continue
			target = dest / rel
			target.parent.mkdir(parents=True, exist_ok=True)
			target.write_bytes(zf.read(name))
			extracted += 1

	print(f"Extracted {extracted} files", file=sys.stderr)
	print(dest.resolve())


if __name__ == "__main__":
	main()
