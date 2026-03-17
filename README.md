<!-- :toc: macro -->
<!-- :toc-title: -->
<!-- :toclevels: 99 -->

# module_template <!-- omit from toc -->

> Example module layout for [build](https://github.com/lurkydismal/build) system.

## Table of Contents <!-- omit from toc -->

* [General Information](#general-information)
* [Technologies Used](#technologies-used)
* [Features](#features)
* [Setup](#setup)
* [Usage](#usage)
* [Project Status](#project-status)
* [License](#license)

## General Information

* The build system expects each module to provide `config.sh` which exports globs for what to include and what to compile.

## Technologies Used

<!--
GNU bash, version 5.3.3(1)-release (x86_64-pc-linux-gnu)
Copyright (C) 2025 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>

This is free software; you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
-->
* GNU bash 5.3.3
<!--
clang version 21.1.4
Target: x86_64-pc-linux-gnu
Thread model: posix

Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
See https://llvm.org/LICENSE.txt for license information.
-->
* clang 21.1.4
* gtest - 1.17.0-1

## Features

`header_frequency.sh` helps pick candidates for a precompiled header by counting `#include` occurrences.

## Setup

The build system must `source` or `eval` the module's `config.sh`. The template uses the following variables:

```bash
#!/bin/bash
# Export globs. The top-level build system expands these.
export FILES_TO_INCLUDE='include/*.hpp'
export FILES_TO_COMPILE='src/*.cpp'
```

Rules and expectations:

* `FILES_TO_INCLUDE` should point at public headers users of the module must add to include paths.
* `FILES_TO_COMPILE` should list source files the top-level build will compile or add to a library target.
* Use shell globs or explicit file lists. The build system expands globs; do not assume `config.sh` will itself scan directories.
* Keep `config.sh` side-effect free except for `export`ing variables. Make it safe to `source` multiple times.

## Usage

1. Copy this folder into your top-level `<module-name>/`.
2. Edit `include/` and `src/` to implement your API and logic.
3. Update `config.sh` to export the exact globs or file list your build system expects.
4. Commit the module.
5. Run your top-level build tool. The build tool should detect `modules/<module-name>/config.sh`, source it, add includes and compile units to the build graph.

## Project Status

Project is: _complete_.

## License

This project is open source and available under the
[GNU Affero General Public License v3.0](LICENSE).
