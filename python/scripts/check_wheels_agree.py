"""Fail if the wheels of one build carry different rexlib libraries.

ccache reuses rexlib's objects between interpreters, which is sound only
because rexlib names no Python: no find_package(Python), no pybind11, so its
objects cannot depend on which one a wheel was built for. This asserts that,
on the artefact, before anything is published.

Compared within a platform tag, not across the job: an interpreter that
requires a newer deployment target than its siblings gets a wheel of its own
tag, and a library legitimately built for it.

A correct build leaves the libraries identical but for what the linker stamps
into them - a timestamp, a build id - so a handful of bytes may differ. Code
diverging would show as thousands.
"""

from __future__ import annotations

import sys
import zipfile
from pathlib import Path

# Enough for a timestamp and a build id, far below any real difference.
TOLERANCE = 64


def library_of(wheel: Path) -> tuple[str, bytes]:
    """Return the name and bytes of the one rexlib library a wheel carries."""
    with zipfile.ZipFile(wheel) as z:
        names = [
            n
            for n in z.namelist()
            if Path(n).name.startswith(("librexlib", "rexlib.dll"))
            and not n.endswith((".lib", ".a"))
        ]
        if len(names) != 1:
            raise SystemExit(f"{wheel.name}: expected one rexlib library, found {names}")
        return names[0], z.read(names[0])


def platform_tag(wheel: Path) -> str:
    """Return the wheel's platform tag, which is its filename's last field."""
    return wheel.stem.rsplit("-", 1)[-1]


def main(directory: str) -> int:
    """Compare the wheels of each platform tag against the first of that tag."""
    groups: dict[str, list[Path]] = {}
    for wheel in sorted(Path(directory).glob("*.whl")):
        groups.setdefault(platform_tag(wheel), []).append(wheel)

    failed = False
    for tag, wheels in sorted(groups.items()):
        if not wheels[1:]:
            print(f"{tag}: one wheel, nothing to compare.")
            continue

        name, reference = library_of(wheels[0])
        print(f"{tag}: comparing {name} across {len(wheels)}, against {wheels[0].name}")
        for wheel in wheels[1:]:
            _, other = library_of(wheel)
            if len(other) != len(reference):
                print(f"  {wheel.name}: size {len(other)}, expected {len(reference)}")
                failed = True
                continue
            differing = sum(a != b for a, b in zip(reference, other))
            verdict = "ok" if differing <= TOLERANCE else "DIFFERS"
            print(f"  {wheel.name}: {differing} byte(s) differ - {verdict}")
            failed |= differing > TOLERANCE

    if failed:
        print(
            "\nThe rexlib in these wheels is not the same library. Either the "
            "compiler cache handed one interpreter's objects to another that "
            "needed different ones, or something interpreter-specific reached "
            "rexlib's own compilation."
        )
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1] if len(sys.argv) > 1 else "wheelhouse"))
