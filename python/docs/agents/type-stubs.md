# Type stubs

How the stubs of the extension are generated and carried into the package.

The extension's `.pyi` files are generated while it is built, by
`scripts/generate_stubs.py` through the stubgen of the nanobind pip installed,
and land in `rexlib/_binding/` beside it. They are not in the
repository and there is nothing to run by hand.

Generating them imports the extension, which a cross-compiling build cannot
do. Those builds are handed stubs made elsewhere through
`REXLIB_PYTHON_STUBS_DIR`; stubs describe the Python API, so the same ones are
correct on every platform. A build that can neither generate nor be given them
fails rather than producing an untyped package.

The package is typed — `py.typed` ships — so a change to the binding's
signatures is a change to the stubs, generated for free, and a change to
`python/rexlib/` is only as typed as it was written.

Importing the extension imports the package first, so stubgen sees what
`_ndarray.py` installs onto `Array` and writes it into the class with its
annotations. It does so only for a function whose name and module are those of
a method of `Array`; any other it renders as an import, which is not a stub.
`_install` gives each one that name and module.

A submodule that holds submodules of its own becomes a directory of stubs
rather than one file: `_binding/<name>/__init__.pyi` beside one `.pyi` for
each it holds. `em` does, holding `functional`, and every step that moves
stubs carries that tree: `scripts/collect_stubs.py` copies it and the artifact
holding it is uploaded as a directory. A flat `*.pyi` glob at either step
takes the top level and drops the rest, which fails nothing and publishes
wheels typed everywhere except the area that was just added.
