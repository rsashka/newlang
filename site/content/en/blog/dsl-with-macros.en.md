---
title: DSL (domain-specific language) implementation with macros
date: 2023-03-01
description: >
  A short lead description about this content page. Text here can also be
  **bold** or _italic_ and can even be split over multiple paragraphs.
---



https://habr.com/articles/720416/


![image](https://habrastorage.org/webt/r9/4d/tu/r94dtuimvtiiuntx5xpunndaetq.jpeg)
[This is a translation of my own article](https://habr.com/company/timeweb/blog/719332/)

The release of **[NewLang](https://newlang.net/)** language with a brand new "feature" is coming, a remodeled version of the preprocessor that allows you to extend the language syntax to create different DSL dialects using macros.

### What is it about?
> [DSL](https://en.wikipedia.org/wiki/Domain-specific_language) (Subject Oriented Language) is a programming language specialized for a specific application area. It is believed that the use of DSL significantly increases the level of abstractness of the code, and this allows to develop more quickly and efficiently and greatly simplifies the solution of many problems.

#### Conditionally, we can distinguish two approaches to DSL implementation:
- Development of independent syntax translators using lexer and parser generators to define the grammar of the target language through BNF (Backus–Naur form) and regular expressions (Lex, Yacc, ANTLR, etc.) and then compiling the resulting grammar into machine code.
- Development or integration of the DSL dialect into a general-purpose language (metalanguage), including the use of various libraries or special parsers / preprocessors.

We will talk about the second option, namely the implementation of DSL on the basis of general-purpose languages (metalanguages) and the new implementation of macros in **NewLang** as the basis for DSL development.

<cut />

## Two extremes
It probably makes sense to start by describing two extremes in the implementation of a DSL based on a general-purpose language (metalanguage):

### Limited grammar.
If a programming language is limited to its own fixed grammar and does not allow for its expansion, when implementing DSL the developer will be forced to use the existing grammar, the rules for recording operations and generally all syntax will remain the same as in the implementation language. For example, when using C/C++ as a base language, or using various libraries and frameworks in other general-purpose programming languages.

*In this case, the term "DSL" will simply be a set of domain-specific terms, overridden macros and/or operators, but the use of which will be restricted by the implementation language grammar.*

### Unrestricted grammar.
If a language (metalanguage) allows you to modify its own grammar (for example at the level of [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree)), then DSL will no longer be strictly bounded by the syntax of the underlying programming language, and as a result its grammar can be whatever you want. Up to the point that "for every new project you will have to learn a new language...". This can be done by using specialized metalanguages (Lisp, ML, Haskell, Nemerle, Forth, Tcl, Rebol, etc.).

I strongly recommend you to read an excellent article on metaprogramming by @NeoCode [Metaprogramming: what it is and what it should be] (https://habr.com/post/258667/).

## The following implementation of macros is offered for discussion
"There is no perfection in the world", and after the release of **NewLang 0.2**, I received a lot of feedback (mostly negative), about the first version of [macro implementation](https://github.com/rsashka/newlang/blob/v0.3.0/docs/syntax.md#макросы) and [DSL based on it](https://github.com/rsashka/newlang/blob/v0.3.0/docs/syntax_dsl.md). And those criticisms were often valid. So I decided to try to redesign the macros a bit, in hopes of getting a "middle ground" between the two extremes described above when describing the DSL.

### Terminology Used
Macros in *NewLang*, are one or more terms that are replaced by another term or by an entire lexical construct (a sequence of tokens). Macros are both an extension of basic language syntax, when implementing DSL's own dialects, and a syntax sugar.

The main feature of macros is that they allow expressions to be changed before they are evaluated at runtime. Macros are expanded at runtime **lexer**, which allows you to substitute any other terms with them and even modify the language syntax itself.

That's why if you don't specify a modifier before the **NewLang** object name (**\**macros, **$**local_variable or **@**module), the program will first search among macros, then among local variables and finally among modules (object from module). This allows you to use terms without modifiers to specify types of objects.

#### Macro definitions
The definition of macros uses exactly the same [syntax](https://newlang.net/ops.html) as for other language objects (the operators "**::=**", "**=**" or "**:=**" are used respectively to create a new object, assign a new value to an existing object or to create/assign a new value to an object regardless of its presence or absence).

In general, a macro definition consists of three parts **<** macro name **>** **<** creating/assignment operator **>** **<** macro body **>* and final semicolon "**;**".

#### Macro body
The macro body can be a correct language expression, a sequence of tokens (framed in double backslashes, i.e. **\\\\** lexem1 lexem2 **\\\\**) or a plain text string (framed in triple backslashes, i.e. **\\\\\\** text string **\\\\\\**).

To connect two tokens into one (analogous to the **##** operation in the C/C++ preprocessor), used by analogy syntax **\##**. A similar operator is used to frame a lexeme in quotes **\#**, for example, `\macro($arg) := \\\ func_ \## \$arg(\#arg) \;`,  then a call to macro(arg) will be expanded to `func_arg ("arg")`;

#### Macro Name
The macro name can be a single identifier with the macro prefix "**\\**" or a sequence of several lexem. If a sequence of lexem is used as the macro name there must be at least one identifier among them and there can be one or more templates.

A template is a special identifier that can be replaced by any single term during matching. With the help of templates the search by pattern and replacement of the set sequences of lexemes on a body of a macro is made.

To specify a template, you need to put a dollar sign at the beginning of the identifier (which corresponds to the local variable name record), i.e. **\\\\**one_lexem**\\\\**, **\\\\**total three tokens**\\\\** **\\\\** lexem *$template1* *$template2* **\\\\**.

Macros are considered identical if their identifiers are equal, the number of elements in their names is the same, and identifiers and templates are in the same places.

#### Macro Arguments
Terms or templates in a macro name can have arguments, which are specified in parentheses. The passed arguments in the body of macro are written in the disclosure space as a local variable name, but a backslash must be added before the name, i.e. `\$name`.

An arbitrary number of parameters in a macro is marked with a triple colon "...", and the place for inserting these arguments is marked with the token **\$...**. If a macro has several identifiers with arguments, the place marked with the desired identifier to insert arguments from a marked identifier, for example, **\$name...**.

To insert the number of actually passed arguments, the lexeme **\$#** is used, or with the marked identifier, for example, **\$#name**.

*Macros work with lexem that contain various information, including the data type if it is specified. But at the moment data types in macro arguments are not handled in any way, and this is one of the mandatory features that will be implemented in the future.

### Examples:
```
   \macros1 := 123;
   \macros2(arg) := { func( \$arg ); func2(123);};
   \\ macros of(...) tokens \\ := \\ call1(); call2( \$... ); call3() \\;
   \text_macros := \\\ string for the lexer \\\;

    # Ordinary macros (macro body is a valid expression)
    \macro := replace();
    \macro2($arg) := { call( \$arg ); call() };
    # The function takes the number of arguments and the arguments themselves
    \\func name1(...)\\ := name2( \$#, \$name1... );

    # macro body from a sequence of tokens
    \if(...) := \\ [ \$... ] --> \\; # Expression may not be complete
    \else := \\ ,[ _ ] --> \\; # Expression may not be complete

    # Macro body from text string (like in C/C++ preprocessor)
    \macro_str := \\\ string - macro body \\\; # String for the lexer
    \macro($arg) := \\ func_ \## \#arg(\#arg)\\; # macro(arg) -> func_arg ("arg")
```

## What possibilities does this offer?
You can define macros in the following combinations this way:
```
№   Macro name         Macro body
----------------------------------------------------------------
   1.   \identifier                     expression
   2.   \identifier                     \\lexem1 lexeme2\\
   3.   \Identifier                     \\\string for the lexer\\\.
   4.   \\lexeme1 lexeme2\\    expression
   5.   \\lexeme1 lexeme2\\    \\lexeme1 lexeme2\\
   6.   \\lexeme1 lexeme2\\    \\\ line for the lexer \\\
```
### Each of the above combinations has its own properties and limitations:
1. Classic replacement of one term with another term or integer expression. It is handled by lexer and parser once during definition. The expression in the macro body has to be correct in terms of syntax, and if there are errors in it, a message about this is generated immediately, while still defining the macro.

2. Classical replacement of one term by sequences of tokens, including incomplete syntactic constructions. It is processed by the lexer once during macro definition. The body of the macro is parsed by the parser when it is used, so possible syntax errors will be noticed only when the macro is expanded.

3. Classic replacement of a single term with a text string, which is fed to the lexer's input. Only the macro name is once processed by the lexer when it is defined, allowing you to modify the macro body and change/combine/modify lexemes before feeding them into the analyzer. Syntax errors will be noticed only when the macro is expanded.

4, 5 и 6. Replacing a sequence of several tokens (templates) with an expression, a sequence of tokens or a text string, respectively.


## Purpose and examples of use
Macros are also used to convert the basic *NewLang* syntax into a more familiar keyword-based syntax, because such text is much easier to understand when reading the source code later.

If no modifier is specified before the name of NewLang object (**\\**macros, **$**local_variable or **@**module), then macros name is searched first, then the name of local variable and last, the name of module (module object). This makes it possible to define DSL syntax in the usual way without the obligatory prefixes for different types of objects.

For example, writing a conditional statement in the basic syntax **NewLang**:
```
    [condition] --> {
        ...
    } [ condition2 ] --> {
        ...
    } [ _ ] {
        ...
    };

# With macros
    \if(...) := \\ [ \$... ]--> \\;
    \elif(...) := \\ ,[ \$... ]--> \\;
    \else := \\ ,[ _ ]--> \\;

# Turns into classical notation
    if( condition ){
        ...
    } elif( condition2 ){
        ...
    } else {
        ...
    };
```

Or loop to 5:
```
count := 1;
[ 1 ] <-> {
    [ count>5 ] --> {
        ++ 42 ++;
    };
    count += 1;
};
```

will look more familiar with the use of appropriate macros:
```
\while(...) := \\ [ \$... ] <-> \\;
\return(...) := ++ \$... ++;
\true := 1;

count := 1;
while( true ) {
    if( count > 5 ) {
        return 42;
    };
    count += 1;
};
```

### Deleting macros
To delete a macro, you have to assign it an empty sequence of tokens `\macro_str := \\\\;`. You can use a special syntax to delete it: `\\\\ name \\\\;` or `\\\\ \\two terms\\ \\\\;`, i.e. specify the macro name between four backslashes.

The necessity to use a separate syntax construct to remove macros is caused by the fact that macro names are processed by the lexer even before the parsing stage in the parser.

## What's the upside?
1. You can dilute the basic language syntax with additional keywords and turn it into a familiar "keyword-based" syntax.
2. Macros are defined according to the language's vocabulary, and macros are treated as normal objects.
3. Ease of source code analysis and debugging.
4. You can make explicit use of DSL terms and metaprogramming techniques, for example, always prefix the macro name. That way the compiler will know very well that the macro needs to be expanded.
5. Although you can modify the language syntax significantly at your own risk, you can only do so within certain constraints (AST cannot be modified directly), which does not allow you to go very far and, for example, crash or hang the compiler.
6. In spite of very big possibilities to modify the syntax, you get a very simple, fast and unambiguous implementation. And this positively affects the speed of source code analysis, detection and processing of possible errors and simultaneously represents a reasonable compromise between the complexity of implementation of this functionality and possibilities of defining own DSL dialects.
7. If desired, there is room to develop metaprogramming capabilities. In the future we could add pattern-matching (e.g. based on regular expressions), make string parametrization for generating syntax in the body of a macro, including in runtime, and many other different ways to elegantly shoot yourself or your comrade in the foot.

## Conclusion
Any feedback on this macro implementation would be appreciated. And twice as grateful if, in addition to criticism, there are suggestions for its improvement and refinement, if any point has been missed.