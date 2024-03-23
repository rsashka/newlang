# About *NewLang* project

*NewLang* is a universal high-level programming language. The main feature of the language is a simple, logical and consistent syntax, based on a strict system of grammar rules. With the help of macros the language syntax can be easily extended to a more familiar form based on keywords. Tensor calculations and rational numbers of unlimited precision are supported at the level of language syntax and basic data types without the use of additional libraries.

Current version 0.4 of 23.03.2024 ([New in current version and release history](https://github.com/rsashka/newlang/releases/tag/v0.4.0))

## Main properties and features of the language:

- The ability to work both in interpreter and compiler mode **\***.
- Dynamic and static typing with the ability to specify types explicitly.
- Static typing is conditionally strong (there is no automatic type casting, but conversion between some data types is allowed. *For example, an integer can be automatically converted to real or rational, but not vice versa.*
- Automatic memory management without garbage collector **\***
- Object-oriented programming in the form of explicit class inheritance and "duck typing" **\***
- Several types of functions (regular and pure functions without side effects) are supported at the syntax level of the language
- Optional and named function arguments
- Direct insertion of C/C++ code is possible **\***
- Easy integration with existing software libraries (including import of native variables, functions and classes* from C/C++).
- There is a <a href="https://en.wikipedia.org/wiki/REPL">REPL (read-eval-print loop)</a>
- Symbolic calculations **\*\***

---
**\***) These features are planned for implementation when the compiler is created.

**\*\***) Symbolic calculations are supported at the syntax level, but not implemented.

---

[Language syntax](http://newlang.net/docs/syntax/)

[Detailed description of the type system](http://newlang.net/docs/types/)

[Operators and control structures](http://newlang.net/docs/ops/)

[Playground and example code](http://newlang.net/playground/?id=1)

### Example of "Hello world!" script in NewLang language

```python
    #!../output/nlc 

    print('Hello, world!\n');
```

Output:

```python
    Hello, world!

    14
```


## Why do we need NewLang?

All modern programming languages have a constant development (complication) of syntax as new versions are released.
This is a kind of payment for the emergence of new features and is perceived by users as a natural process.

But at the same time it is also a serious problem, since with the release of versions new keywords and syntactic constructions are added, which inevitably raises the entry threshold for new users.

Another consequence of this process is the constant increase in the complexity of developing and supporting already created software products, when the old code is being finalized using the already new standards.

NewLang naturally limits the complexity of language constructs by splitting the syntax of the language into two parts, making it easier to learn and use.

Basic syntax - for writing programs in object-oriented (imperative) and declarative styles, which is based not on reserved keywords, but on a system of strict grammar rules. It is possible to extend the basic syntax through the use of macros.
Extended syntax - program inserts in the implementation language (C/C++), when the main syntax becomes insufficient.

Another disadvantage of modern languages is that most of them were created before the era of machine learning, therefore, tensor calculations are performed in the form of separate libraries.
The same applies to calculations with unlimited precision, which also require the use of additional library functions.

NewLang has tensor calculus and unlimited-precision rationals out of the box.
They are supported at the syntax level for writing literals of the corresponding types, and simple arithmetic data types are scalars (tensors of dimension zero).
The implementation of tensor calculations is based on the library libtorch, and rational numbers using OpenSSL.


### Example of calculating factorial 40 using DSL syntax

```python
    #!../output/nlc 

    fact := 1\1;                # Rational number 1 (no precision limit)
    mult := @iter( 40..1..-1 ); # Iterator from range for factors from 40 to 2
    @while( @curr(mult)) {      # Loop until the iterator data runs out
        fact *= @next(mult);    # Get the current multiplier and move to the next iterator element
    };

    @assert(fact == 815915283247897734345611269596115894272000000000\1);

    fact    # Return final result    
```

Output:

```bash
    815915283247897734345611269596115894272000000000\1
```

## Feedback

If you have any suggestions for developing or improving NewLang, [writing](https://github.com/rsashka/newlang/discussions).

