# Code style

The naming, formatting and file rules every source file follows.

`.clang-tidy` is the authority on naming and is enforced in CI. In short:
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

Say **what**, never **why**. The why belongs in the commit message and the pull
request. And only say the what when the code does not already: if a reader
understands the line without the comment, the comment does not go in.

There is not a single `TODO` in the repository. Unfinished work is an issue,
not a comment.
