---
title: Naming Objects
description: Rules for naming variables, functions, and data types
weight: 10
---

Objects and data types can be named using letters, numbers, and underscores in any combination, provided that the first character of the name is not a digit.

All identifiers must be unique, and to avoid collisions, you can use [namespaces](/docs/syntax/namespace/) and a [modular code structure](/docs/syntax/modules/) supported simultaneously by *NewLang*.

Overloading [functions](/docs/types/funcs/) based on argument types is absent in *NewLang*, so defining multiple functions with the same name but different arguments is not possible, but you can [override a function](/docs/types/funcs/), including extending the types of accepted arguments or increasing their number.

An object identifier can contain one or more special characters - qualifiers (or [sigils](https://en.wikipedia.org/wiki/Sigil_(computer_programming))), which represent specific values.
An object identifier that does not contain a qualifier is called *simple*:

### Name Qualifiers: {#sigil}
- '**@**' - the *at sign* prefix is used to specify the name of a [macro](/docs/syntax/macros/), which is processed by the preprocessor before the start of the program's syntactic analysis.
- '**@**' - the *at* prefix is used to specify the name of a [macro](/docs/syntax/macros/) that is processed by the preprocessor before the start of the syntactic analysis of the source code of the program.
- '**$**' - the dollar sign at the beginning of an object's name denotes an automatic name for a temporary variable, the memory space for which is dynamically allocated during the application's execution, and the [lifetime is limited by the language semantics](/docs/syntax/memory/).
- '**::**' - the double colon serves as a separator for [namespaces](/docs/syntax/namespace/) and indicates a *static* object, the memory space for which is allocated during the compilation of the application or module. If the name starts with '**::**', the object's scope will be global and it will be accessible throughout the application. Otherwise, the object's visibility will be limited to the current [program module](/docs/syntax/modules/).
- '**.**' - the *dot* prefix is used when accessing a module or class field (limits the scope to the current object). The *dot* prefix can be used when defining (calling) a function to explicitly identify a named argument, preventing it from being overridden by a preprocessor macro.
- '**\\**' - the *backslash* at the beginning of a term denotes the name of a [program module](/docs/syntax/modules/), and also separates directory names in the hierarchy of program module placement.
- '**:**' — colon at the beginning of the term denotes the name of a [data type](/docs/types/) or a [class constructor](/docs/types/class/)
- '**%**' — the *percent sign* prefix is used for [imported symbols (native variables and functions)](/docs/types/native/)
- '**^**' — the caret symbol after the name is used to give immutability (constancy, non-modifiability) to the object.

### Name Lookup {#name-lookup}
If the object name does not contain a [qualifier](/docs/syntax/naming/#sigil), it is called *simple*. Creating an object with a *simple* name is equivalent to creating a local object.

In other cases, when *NewLang* encounters a *simple* object name without a qualifier ([sigil](https://en.wikipedia.org/wiki/Sigil_(computer_programming))), a special algorithm comes into play that associates the *simple* name found in the source code of the program with its declaration or a specific object by its [internal name](/docs/arch/mangling/).

Resolution of *simple* names *without a qualifier* (*name lookup* for function/variable names) always occurs in a strictly defined order:
- First, the search for the name is among macros
- Next, the search is among local objects up to objects of the current module
- Lastly, the search is among global objects with a gradual expansion of the namespace search scope from current to global

Such a sequence of name resolution always provides the opportunity to redefine global/local objects or argument names in functions for existing code without serious changes.

For example, for the name `name` in the namespace **`ns`**, the search occurs in the following sequence:
`@name` -> `$name` -> `ns::name` -> `::ns::name` -> `::name`,
and for the name `arg`, only `@arg` is checked:
```python
    ns:: {
        name(arg="value");
    };
```

And at the same time, there will always be the possibility to specify a specific object regardless of the algorithm for resolving *simple* names. It is sufficient to explicitly specify the qualifier in the object name.

For example, to access the global object **name** from the namespace **ns** in the above example, you need to use the full object name `::ns::name`, and the named argument *'**.** arg'* will not be replaced by the `@arg` macro if it is defined:
```python
    ::ns::name(.arg="value");
```
### Namespace Search Extension {#using}
To specify multiple namespaces for an extended search when resolving *simple* unqualified names, 
a syntactic construction `... = ns::name, ns::name2;` or `@using(ns::name, ns::name2);` is used with DSL.

The search in the listed namespaces is performed in the order of their specification until the end of the current module 
or until the next extended search operator or until `... = _;`, which cancels the extended search in namespaces.

### Forward Declaration {#forward-declaration}
In the program text, you can only refer to actually existing (created) objects. 
But in cases where it is necessary to refer to an object that is created in another module or will be created later, 
you can make a preliminary declaration, in which the compiler registers the name and type of the object without its actual creation.

By means of a forward declaration, one can refer only to static objects (data types), or local class fields that the compiler does not yet know about but which will be defined during the compilation process later.

For a forward declaration, only the fully qualified name can be used, which must exactly match the object's name when it is subsequently created.

The same syntax is used for a forward declaration as for the actual [creation](/docs/ops/create/) of an object, only an ellipsis should be specified to the right of the creation operator.


The scope of a forward declaration corresponds to the scope of its placement, not the actual scope of the object (even for global objects).
```python

    # Preliminary definition of a module variable
    # Applies to the entire module
    var_module:Int32 := ...;

    func() ::= {

        # Preliminary announcement using DSL
        # (only works inside a function body)
        @declare( func2(arg:Int32):Int32 );

        var_module = func2(var_module);
        @return var_module;
    };


    func2(arg:Int32):Int32 ::= {
        @return $arg*$arg;
    }

    var_module:Int32 := 1;
```


### Argument Names, Special, System Names {#args}
The notation of argument names in functions is very similar to referencing arguments in bash scripts, where "**$1**" or "**$name**" represents the ordinal number or name of the corresponding argument.

The reserved name "**$0**" denotes the current object, and the name "**$$**" denotes the parent object.

All function arguments are collected in a single dictionary with the special name **$\***

The immutable variable "**$^**" contains the result of the last operator or code block execution.

The full name of the current module is contained in the variable **@\\\\**, and the current namespace in the variable **@::**, i.e.:
```bash
# File name filename.src in directory dir

ns:: { # Use namespace ns
    name:: {
        # Preprocessor command "@#" - convert to a string
        ns_str :=  @#  @::; # String with namespase "::ns::name::"
        mod_str :=  @#  @\\; # String with module name "\\dir\filename"
    };
};
```

