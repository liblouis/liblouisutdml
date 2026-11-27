# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

liblouisutdml: a C library (LGPL-3+) that does braille transcription and formatting of XML, HTML and text documents, built on top of [liblouis](https://github.com/liblouis/liblouis) (which does the actual character/contraction translation). It also ships the `file2brl` CLI (GPL-3+, `tools/file2brl.c`) and JNI-based Java bindings (`java/`).

## Build & test

Autotools project. It requires liblouis >= 3.28 and libxml2 (via pkg-config). Java bindings are on by default and need `jni.h`.

```sh
./autogen.sh
./configure                         # add --disable-java-bindings if no JDK/JNI
make
make check                          # full suite; results in tests/test-suite.log
```

- Run a single test: `make -C tests check TESTS=boxline.test` (or `TESTS=mathml_nemeth/msup_01.test`). Add `VERBOSE=1` to print the test README and a diff on failure.
- Sanitizer build (as in CI): `CFLAGS="-fsanitize=address -g" ./configure && make check`.
- Java bindings: `cd java && ant` (`ant test` runs `java/tests`).
- Windows cross-builds: `make distwin`, or `docker build -f Dockerfile.win32 .` / `Dockerfile.win64`.
- CI (`.github/workflows/main.yml`) builds against both the latest liblouis stable release (`LIBLOUIS_VERSION`) and liblouis master.

## Tests

Each data-driven test is a directory `tests/<name>.test/` that contains its own `input.xml`, `expected.txt`, `liblouisutdml.ini`, `styles.cfg`, `README`, plus any tables and `.sem` files it needs, so it does not depend on installed files. `tests/run_test.sh` runs `file2brl -f styles.cfg input.xml` from the directory holding the `.ini` (paths in it are relative), then diffs the output against `expected.txt`. Files are found by searching upward, so the subtests under `mathml_nemeth/`, `nemeth_MathCAT/` and `test_mathml_woluwe/` share config files from their parent directory.

To add a test, create the directory and list it in `tests/Makefile.am` under `dist_suite_TESTS` (and under `SUBDIRS` if it is a new top-level directory). Many tests are listed in `XFAIL_TESTS`, because liblouisutdml was never updated for a liblouis 3.28 change (see PR #101). When a fix makes an XFAIL test pass, `make check` reports XPASS, so remove that test from the list.

## Architecture

Processing pipeline (`liblouisutdml/`):

1. **Config** (`readconfig.c`): reads `liblouisutdml.ini` / `.cfg` files and `-C` settings into the global `ud` struct (`louisutdml.h`). This covers page layout, tables, styles, the `semanticFiles` setting, and `formatFor`, which selects the output format (textDevice, browser, utd, pef, transInXml, volumes, brf, dsbible, ...).
2. **Semantics** (`semantics.c`, `examine_document.c`): `.sem` files (examples in `lbu_files/`) map XML element names or XPath-like patterns to semantic actions. `examine_document` walks the libxml2 tree and tags each node with a `semantics` attribute.
3. **Transcription** (`transcribe_document.c` dispatches to `transcribe_paragraph.c`, `_math.c`, `_chemistry.c`, `_music.c`, `_computerCode.c`, `_graphic.c`, `_cdataSection.c`). These collect text and call liblouis to translate it.
4. **Formatting** (`transcriber.c`, the largest file): styles, line filling, page numbers, boxlines, headers/footers. It has two code paths: the classic text output path, and the `utd_*` functions that build UTD (Universal Tactile Document) markup inside the XML tree.
5. **UTD post-processing**: `convert_utd.c` and the `utd2*.c` files convert UTD to brf, pef, volumes, transinxml or dsbible. `makeContents.c` and `makeEndnotes.c` build the table of contents and endnotes.

The public API is in `liblouisutdml.h` (`lbu_translateString`, `lbu_translateFile`, `lbu_backTranslateFile`, logging, `lbu_free`). `java/Jliblouisutdml.c` is compiled into the same `.la` when the bindings are enabled.

### Semantic action names are generated

`sem_enum.h` is the list of semantic actions. `sem_names.h` is generated from it. If you edit `sem_enum.h`, regenerate it with `cd liblouisutdml && make -f Makesemnames`.

## Repo hygiene

The working tree usually contains build products (`config.status`, `*.o`, `tests/*.log`), old release tarballs and Emacs backup files (`*~`). Do not commit them.
