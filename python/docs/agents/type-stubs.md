# Type stubs

How the stubs of the extension are generated and carried into the package.

The extension's `.pyi` files are generated when the package is installed,
last, by the stubgen of the nanobind pip installed, through
`nanobind_add_stub(INSTALL_TIME RECURSIVE)`, and land in `rexlib/_binding/`
beside the extension. They are not in the repository and there is nothing to
run by hand.

Generating them imports the installed package, so the library has to be
installed before it: in the library's build, `python/` is added after
`src/`. nanobind does not fail the install when stubgen fails, so
`tests/python/test_stubs.py` checks that the stubs are there.

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
each it holds. `em` does, holding `functional` and `ops`, which holds
`parameters` in turn, and so does the top-level `ops`, holding `fourier`.
