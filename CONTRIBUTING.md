<!--
SPDX-FileCopyrightText: 2026 Edward Kmett <ekmett@gmail.com>
SPDX-License-Identifier: BSD-2-Clause OR Apache-2.0
-->

# Contributing

Contributions to jit++ are offered under `BSD-2-Clause OR Apache-2.0`, allowing
recipients to choose either license. See [COPYING](COPYING) and the complete
texts in [LICENSES](LICENSES). Any contribution requiring different licensing
needs Edward Kmett's explicit approval before inclusion.

## SPDX headers

Use the [SPDX short-form identifiers](https://spdx.dev/learn/handling-license-info/)
and [REUSE conventions](https://reuse.software/spec/) to record licensing and
copyright in each source, test, script, configuration and documentation file.
The project-owned file header is:

```cpp
// SPDX-FileCopyrightText: 2026 Edward Kmett <ekmett@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR Apache-2.0
```

- Put the header at the top, after a required shebang and before code or the
  module declaration. Keep preambles to SPDX metadata rather than full license
  boilerplate or author-history blocks.
- Use the file's comment syntax: `//` for C++ and preprocessed assembly, `#` for
  shell, Perl, CMake, YAML and ignore files, and an HTML comment for Markdown.
- Use the appropriate copyright holder and creation year for new files. Preserve
  existing attribution and original years when editing or moving code; add a
  separate copyright line for additional holders where appropriate. Do not
  replace someone else's attribution with your own.
- Keep the expression exactly `BSD-2-Clause OR Apache-2.0` for project-owned
  files. `OR` offers a choice of licenses; do not substitute `AND`.
- If a file cannot contain comments, use a neighboring `<filename>.license`
  sidecar containing its SPDX copyright and license tags.

## Third-party material

Keep upstream copyright, license and required notice text when importing code
or assets. Record their actual SPDX expression and source/version; do not apply
the project's dual-license header to material with different terms. Store any
additional license text under `LICENSES/<SPDX-identifier>.txt` and document the
exception alongside the imported material.

External dependencies such as glog, gflags and udis86 remain under their upstream
licenses. The license documents themselves are kept as complete license texts,
without prepending the project's source-file header.
