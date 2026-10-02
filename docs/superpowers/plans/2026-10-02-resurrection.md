# Original runtime resurrection implementation plan

> **For agentic workers:** Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build the original implementation and demonstrate native → interpreted x86-64 → native execution.

**Architecture:** Preserve the existing Autotools build, decoder, interpreter, and assembly trampoline. Fix demonstrated compatibility and round-trip failures, with executable regression coverage.

**Tech Stack:** Linux x86-64, C++, GNU Autotools, pthreads, glog, gflags, udis86.

**Spec:** User instructions in this chat: get the original building and tested; preserve the core trick before a later redesign.

## Global constraints

- Focus on getting the original implementation running.
- Defer architectural redesign and optimization.

## Review focus

Entry/exit ABI, arithmetic flags on exit, actual interpretation versus immediate fallback, repeated entry, and unsupported-instruction fallback need runtime checks.

## Task 1: Restore the build

- [x] Reproduce bootstrap/configuration/compiler failures.
- [x] Fix compatibility failures in `bin/autogen.sh`, `configure.ac`, and affected C++ files only as demonstrated.
- [x] Build `libjit++` and `test_interpreter` with a current compiler.

## Task 2: Prove the core round trip

- [x] Run the original demo and diagnose failures.
- [x] Add a deterministic test in `test/` proving interpreted execution and native resumption; register it in `Makefile.am`.
- [x] Reproduce failures before fixing trampoline/interpreter behavior.
- [x] Run `make check` and the original demo; check optimized and unoptimized builds.

## Task 3: Make the result reproducible

- [x] Document dependencies, bootstrap/build/test commands, and known limitations in `README`.
- [x] Ignore generated build artifacts.
- [x] Review the diff and rerun the complete test suite before reporting results.

## Validation results

- GCC 11.4, Ubuntu 22.04 x86-64: `make check` passes 2/2 at `-O0 -g` and `-O2 -g`.
- Original demo reproduced the too-small worker stack and debug-only assembly/C++ stop-symbol collision before fixes.
- Assembly regression reproduced stale lazy flags on explicit stop before its fix.
- Pinned udis86 helper completed a fresh download, build, and install.
- Review corrected the step-limit test to assert the exact resume PC.
- GitHub Actions workflow is added but has not run remotely.
- Existing Autotools deprecation and flags-helper return-type warnings remain.
