# Code style

The naming, formatting and file rules every source file follows.

`.clang-tidy` is the authority on naming and is enforced in CI (see
[clang-tidy](#clang-tidy)). In short:
`lower_case` for everything but template parameters, which are `CamelCase`;
private members carry `m_`; macros are `UPPER_CASE` with a `REXLIB_` prefix.

Indentation is tabs, four columns wide, and lines stay within 80 columns;
`.editorconfig` states both, with the rules for the other kinds of file. CI
checks every file git tracks against it with editorconfig-checker, which
counts a tab as one character: it holds lines to 80 characters, and the 80
columns are for the editor's ruler and review to hold.

Every source file, the CMake files included, starts with
`SPDX-License-Identifier: GPL-3.0-only`.

Declarations go in a `.hpp`, definitions that must be visible go in an `.inl`
the header includes at its end, and everything else goes in a `.cpp`.

## Comments

In C++, Python and CMake alike, say **what**, never **why**. The why belongs in the commit message and the pull
request. And only say the what when the code does not already: if a reader
understands the line without the comment, the comment does not go in.

There is not a single `TODO` in the repository. Unfinished work is an issue,
not a comment.

## clang-tidy

The `lint` preset runs clang-tidy on the library's sources as they compile,
and a finding fails the build; CI runs it with clang-tidy 23.1.3, from PyPI.
The tests are left out by `tests/.clang-tidy`. `.clang-tidy` turns on the
families `bugprone`, `clang-analyzer`, `cppcoreguidelines`, `misc`,
`performance` and `readability`, and turns off the checks below. A finding
the code means to keep is marked where it is, with `NOLINT(<check>)` and
nothing broader.

| Turned off | Because |
|---|---|
| `readability-identifier-length` | Short names such as `i`, `n` and `x` are the idiom |
| `readability-named-parameter` | An unnamed parameter is how an unused one is marked |
| `readability-implicit-bool-conversion`, `readability-math-missing-parentheses`, `readability-trailing-comma`, `readability-use-concise-preprocessor-directives`, `cppcoreguidelines-use-default-member-init`, `cppcoreguidelines-macro-to-enum` | Matters of taste the code does not follow |
| `readability-magic-numbers`, `cppcoreguidelines-avoid-magic-numbers` | Numerical kernels are made of constants |
| `readability-function-cognitive-complexity`, `readability-convert-member-functions-to-static` | Judgements, not rules |
| `bugprone-crtp-constructor-accessibility` | The CRTP bases are layered (`basic_operation` under `trivial_operation` and `parametric_operation`), so the class the check would befriend is not the one that constructs them |
| `bugprone-easily-swappable-parameters` | Functions take several extents or sizes by design |
| `bugprone-signed-bitwise` | The bit helpers work on any integer type |
| `bugprone-throwing-static-initialization` | Program builders register through static objects by design |
| `cppcoreguidelines-macro-usage` | The `REXLIB_` macros (exports, registration, features) are the design |
| `cppcoreguidelines-pro-bounds-*`, `cppcoreguidelines-pro-type-reinterpret-cast` | Kernels and memory code reach raw memory by design |
| `cppcoreguidelines-narrowing-conversions` | The same check as `bugprone-narrowing-conversions` |
| `cppcoreguidelines-owning-memory` | Memory is owned through RAII types, not `gsl::owner` |
| `cppcoreguidelines-explicit-constructor`, `misc-explicit-constructor` | `span`, `flagset` and the array references convert implicitly by design |
| `misc-header-include-cycle` | Every `.hpp` includes its `.inl`, which includes the `.hpp` |
| `misc-include-cleaner` | It would take an include-what-you-use pass of its own |
| `misc-no-recursion` | Recursion is used where it reads best |
| `performance-enum-size` | An enum's underlying type is part of the API |

Two checks are tuned. `readability-simplify-boolean-expr` leaves a negated
range alone, as in `!(lower <= x && x <= upper)`, which also rejects NaN.
`cppcoreguidelines-rvalue-reference-param-not-moved` accepts an rvalue
container whose elements are moved one by one.

The configuration keeps the forms that every clang-tidy reads, the oldest
included: `Checks` as one comma-separated string and `CheckOptions` as a list
of `key`/`value` pairs. `HeaderFilterRegex` is `.*`: with the dependencies
installed, their headers are system headers, which clang-tidy skips.
