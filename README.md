# About *NewLang* project

*NewLang* is a universal high-level programming language. The main feature of the language is a simple, logical and consistent syntax, based on a strict system of grammar rules. With the help of macros the language syntax can be easily extended to a more familiar form based on keywords. Tensor calculations and rational numbers of unlimited precision are supported at the level of language syntax and basic data types without the use of additional libraries.

Current version 0.3 of 07.11.2022 ([New in current version and release history](/en/versions.html))

## Main properties and features of the language:

- Ability to work both in interpreter and compiler mode*.
- Dynamic and static typing with the possibility of explicitly specifying types.
- Static typing is conditionally strict (there is no automatic type conversion, but conversion between some data types is allowed. For example, an integer may be automatically converted into a real or rational number, but not vice versa)
- Automatic memory management
- OOP in the form of explicit class inheritance and "duck typing".
- Several types of functions are supported at the syntax level (normal and pure functions without side effects)
- Optional and named parameters of functions
- It is possible to insert the code in the implementation language (C/C++)*
- Easy integration with already existing software libraries (including import of native variables, functions and classes from C/C++)
- Has a REPL [read-eval-print loop](https://wikipedia.org/wiki/REPL)

---
*) This feature is planned to be implemented when creating the compiler.

---
[Language syntax](/en/syntax.html)

[Detailed description of the type system](/en/types.html)


### Example of "Hello world!" script in NewLang language

```bash
    #!../output/nlc --eval

    hello(str) := { 
        # Import and call the standard function printf
        printf := :Pointer('printf(format:FmtChar, ...):Int32');
        printf('call: %s', $str);
        $str;
    };

    hello('Hello, World!');
```

Output:

```bash
    call: Hello, World!
    Hello, World!
```

## Why do we need *NewLang*?

All modern programming languages constantly develop (complicate) their syntax as new versions are released. It is a kind of payment for appearance of new features and is perceived by users as a natural phenomenon.

But it is also a serious problem because with release of versions new keywords and syntactic constructions are added, which inevitably raises the entrance threshold for new users. Another consequence of this process is a constant increase of complexity of development and support of already created software products, when the old code is finalized using already new standards.

With *NewLang* the complexity of language constructions is naturally limited by dividing the language syntax into two parts - this makes it easier to learn and use.

*Base Syntax* - for writing programs in object-oriented (imperative) and declarative styles, which is based not on reserved keywords, but on a system of strict [grammar rules](/en/syntax.html). It is possible to extend the basic syntax by using macros. *Expanded syntax* - program inserts in the implementation language (C/C++) when the basic syntax becomes insufficient.

Another inconvenience of modern languages is that most of them were created before the era of machine learning, so tensor calculations in them are implemented as separate libraries. The same applies to calculations with unlimited precision, which also require additional library functions.

In *NewLang*, tensor calculations and rational numbers of unlimited precision are available "out of the box". They are supported at the syntax level for writing literals of corresponding types, and simple arithmetic data types are scalars (zero-dimensional tensors). The implementation of tensor calculations is based on [libtorch](https://pytorch.org/) and rational numbers using [OpenSSL](https://github.com/openssl/openssl/blob/master/crypto/bn/README.pod).

### Example of calculating the factorial 40 in NewLang

```bash
    #!../output/nlc --eval

    fact := 1\1;    # Rational number without precision restrictions
    mult := 40...1...-1?;   # Get iterator for multipliers from 40 to 2
    [mult ?!] <-> {         # Cycle until the iterator data runs out
        fact *= mult !;     # Get the current multiplier and go to the next iterator element
    };

    fact # Print the final result
```

Output:

```bash
    815915283247897734345611269596115894272000000000\1
```

#### Downloading binary builds

- [Windows Binary Builds and Tests](https://github.com/rsashka/newlang/releases/download/v0.3.0/nlc_win_64.zip) 
- [Ubuntu binary builds and tests](https://github.com/rsashka/newlang/releases/download/v0.3.0/nlc_lin_64.tar.xz) 
- You will need shared libraries libLLVM-15 and libtorch ([archive with libraries for Windows](https://github.com/rsashka/newlang/releases/download/v0.3.0/nlc_dll.zip)), ([archive with libraries for Ubuntu](https://github.com/rsashka/newlang/releases/download/v0.3.0/nlc_so.tar.xz)) to run binary builds.

## Compiling the project from source

### Preparing repository

- Download the sources [https://github.com/rsashka/newlang](https://github.com/rsashka/newlang)
- Download and deploy archive [libtorch](https://pytorch.org/) in directory *contrib* (PyTorch Build: Stable (1.13.*) -> Your OS: Linux -> Package: LibTorch -> Language: C++ / Java -> Compute Platform: CPU -> Download here (cxx11 ABI):
[libtorch-shared-with-deps-1.13+cpu.zip](https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-1.13.0%2Bcpu.zip)
- Activate and download submodule sources (`git submodule init && git submodule update`)
- In the directory *contrib* run the file `build.sh`
- In directory *src* run file `compile_syntax.sh` to generate parser and lexical analyzer files. You may also need to install the utilities *flex* and *bison* (flex 2.6.4 and bison (GNU Bison) 3.7.4)

### Build

- Unit Tests (newlang-unit-tests): In the *src* directory, run the `make CONF=UnitTest` command.  
- Interpreter (nlc): in the *src* directory, run the command `make CONF=Debug`.  

---;

Builds are done with the usual make utility, but the build files are generated automatically in the outdated version NetBeans 8.2. This is the only universal development environment with Makefile support "out of the box", while the current Apache NetBeans version does not provide full support for C/C++ development. Started a gradual transition to using VSCodium editor (analog of VSCode, which cleaned up telemetry from Microsoft) and generation of build skips using smake, but this process is not completed yet. 

## Feedback

If you have any suggestions for developing or improving NewLang, [writing](https://github.com/rsashka/newlang/discussions).


## Roadmap

### Макропроцессор

[x] - Минимальный варинат с одним термином без анализа аргументов (для реализации keyword based синтаксиса)  
[ ] - Поддержка именованных аргументов (аналог макропроцессора С/С++)  
[ ] - Поддержка типов у аргументов макроса  
[ ] - Шаблоны в идентификаторах макросов (поддержка импорта модулей, определения переменных, функций и классов для создание DSL)  
[ ] - Поддержка Simple Python DSL - выполнение простых скриптов Python как собственного кода  

### Синтаксис
[ ] - Контроль множественных типов у аргументов функций <:Type1, :Type2, :Type3>   
[ ] - Контроль возвращаемых типов у функций   
[ ] - Ссылки на объекты в синтаксисе и коде   
[ ] - Константные объекты в синтаксисе и коде   
[ ] - Документирующие комментарии в коде   

### Функциональные возможности
[ ] - Поддержка ООП  
[ ] - Импорт нативных С/С++ типов Struct\Union\Enum    
[ ] - Импорт С++ переменных (namespace + mangled имена)   
[ ] - Импорт С++ функций (namespace + mangled имена)   
[ ] - Импорт С++ объектов  


## Доп. возможности
[--] - Менеджер пакетов conda (mamba)   
[--] - Поддержка jupyter notebook   



