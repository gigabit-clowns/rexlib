# Pull requests and continuous integration

How a pull request is opened, what the workflows do and the binary archives they publish.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

| Workflow | Does |
|---|---|
| `build-and-test.yml` | Builds and tests the matrix and a system dependency build, then the SonarQube scan |
| `deploy.yml` | Builds the documentation, and the binary archives, and publishes both |
| `release.yml` | Tags and releases, through the shared workflow of the organisation |
| `clean-up-caches.yml` | Returns Actions cache space |

The matrix covers Linux, macOS and Windows, gcc, clang and MSVC, x86_64 and
Arm. MSVC builds through Ninja rather than the Visual Studio generator, since
that is the only way a compiler cache can be used, and the environment is set
up with `vswhere` and `vcvars` beforehand.

Beside the matrix, one `ubuntu-latest` job builds against system dependencies,
with `REXLIB_BUILD_DEPENDENCIES` off, so that every `find_package` branch
keeps being exercised. It takes boost, eigen, spdlog, catch2 and trompeloeil
from apt and puts the half and pocketfft headers on the include path the way a
packager would, which is what covers `Findhalf.cmake` and
`Findpocketfft.cmake`. The pins and the hash of half it reads from
`CMakeLists.txt`. The job also asserts that no dependency reached the install
tree.

Every entry runs the suites, and the `ubuntu-latest` ones run them under a
memory checker with `REXLIB_REGISTER_TESTS_PER_BINARY`. The rest run them
plainly: the valgrind of `ubuntu-22.04` predates DWARF 5 and gives up on what
clang emits there, and macOS and Windows have no checker set up. A leak
therefore fails the pull request rather than being found afterwards.

Compilation is cached with ccache. The Actions cache is 10 GB for the whole
repository while access is per branch, so entries pile up faster than they look
like they should; `clean-up-caches.yml` drops what a newer run replaced, and
everything belonging to a pull request when it closes.

The scan runs in a job of its own, so the `ubuntu-latest` gcc entry hands it
what it reads as an artifact. That is `compile_commands.json` and the include
directories named in it, which are the headers of the fetched dependencies;
the object files and the test binaries are the bulk of a build folder and the
scanner opens none of them. The list is derived from the compilation database
rather than written down, so a new dependency needs no change here.

### Binary archives

`deploy.yml` publishes an archive per platform, so that the continuous
integration of a plugin unpacks the library instead of building it. They are
built Debug: their only reader is a test run, and Debug is what leaves the
`REXLIB_ASSERT` in `src/` compiled in, where Release drops them with `NDEBUG`.
The cost is size, a Debug archive being an order of magnitude larger.

Windows publishes both configurations, because there the plugin has to match:
`/MD` and `/MDd` disagree on `_ITERATOR_DEBUG_LEVEL` and on the container
layouts that follow from it. Linux and macOS publish only Debug, since a
Release plugin links a Debug library there without trouble.

They are not for users, who build from source. Nothing here replaces the
`manylinux` guarantees the wheels used to carry.
