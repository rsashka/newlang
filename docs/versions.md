# NewLang version history

## [Release 0.4 (in development)](https://github.com/rsashka/newlang/releases/tag/v0.4.0)

- An unscheduled version with a new version of the parser which can change the input directly during parsing of the source code. This was required to expose macros and support new syntax features: [Examples of defining and using new macros](https://newlang.net)
- Translation of documentation into English
- Initial build support with cmake and development with VSCode

---;

### Miscellaneous in Release 0.4

---;

## [Release 0.3 (07.11.2022) - current version](https://github.com/rsashka/newlang/releases/tag/v0.3.0)

### New features and changes in NewLang 0.3 syntax

- Simple pure functions removed.
- The syntax of the [condition](https://newlang.net/ru/ops.html#условный-оператор) and [loops](https://newlang.net/ru/ops.html#операторы-циклов) check operators has been fixed. 
- The **while** loop operator now supports the [**else**](https://newlang.net/ru/ops.html#операторы-циклов) construct.
- Added [namespaces](https://newlang.net/ru/syntax.html#пространства-имен) to the NewLang syntax.
- Implemented part of the OOP concept and added support for [class definitions](https://newlang.net/ru/type_oop.html).
- Reworked [function returns and exception handling] ideology(https://newlang.net/ru/newlang_doc.html#операторы-прерывания-выполнения-оператор-возврата).

### Miscellaneous in Release 0.3

- Migration to clang 15 has been implemented.
- Released function calls with libffi
- Fully supported for Windows

---;

## [Release 0.2 (11.08.2022)](https://github.com/rsashka/newlang/releases/tag/v0.2.0)

### New features and changes in NewLang 0.2 syntax

- Added macros (newer, more familiar keyword based syntax)
- Iterators added
- New data type added - rational numbers of unlimited precision
- Nested multi-line comments became nested
- Built-in type names have been renamed, specifying the size

### Other important changes

- Changed to clang instead of using gcc, and replaced libffi with a JIT compile call for native functions
- Linux binaries added to the release
- Starting to port code to Windows

## [Release 0.1 (24.06.2022) - first public version](https://github.com/rsashka/newlang/releases/tag/v0.1.0)

- Introducing the general concept of the language
- Compiling tests and examples for Linux from source code
