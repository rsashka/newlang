---
slug: release-v0.4
title: Release of the programming language NewLang 0.4
date: 2023-06-06
#draft: true
---

## New features and changes in the syntax of NewLang
- Reworked the definition of object types using [prefix naming (sigils)](https://newlang.net/docs/syntax/naming/)
- Interrupting the execution flow and returning can now be done for [named code blocks](https://newlang.net/docs/ops/throw/).
- Simplified the syntax for importing [native variables and functions (C/C++)](https://newlang.net/docs/types/native/)
- Stabilized the syntax for [initializing tensor, dictionary, and function argument values](https://newlang.net/docs/ops/create/#comprehensions) with initial data.
- Added built-in macros for writing code using keywords in a [DSL style](https://newlang.net/docs/syntax/dsl/)

## New compiler features (nlc)
- Completely redesigned the macroprocessor.
- Reworked the compiler architecture with division into parser, macroprocessor, syntax analyzer, interpreter, and code generator.

## Miscellaneous
- The documentation [website](http://newlang.net) has been translated to [Hugo](https://gohugo.io/) and made bilingual.
- Instead of binary builds, a section [Playground and example code](https://newlang.net/playground/) has been added to the website for small experiments.
- Transition to clang-16 has been completed (transition to clang-17 and newer is planned after full implementation of coroutines and support for extended floating-point number formats).
- The number of project contributors has increased to more than one!
