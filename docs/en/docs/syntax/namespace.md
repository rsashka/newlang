---
Title: Namespaces
Weight: 20
---

*NewLang* supports namespaces, where the separator, like in C++, is a double colon "**::**".

A namespace can be specified for an individual identifier or for an entire [code block](/docs/ops/block/).

Unlike *namespace* in C++, namespaces in *NewLang* are used not only to organize code into logical groups and avoid name conflicts, 
but also to associate an identifier, explicitly specifying the namespace, 
with static objects (memory allocated during the compilation stage of the program).

The global name cannot be overridden by a macro or local variable during [name lookup](/docs/syntax/naming/). To create a global (static) variable in the current namespace, you can specify the variable with the **@::** preprocessor.

To use a namespace in a [code block](/docs/ops/block/), it must be specified before the opening curly brace. 
In such a named code block, it is not necessary to specify the closing "**::**".

```bash
::var ::= 0; # Name of global variable explicitly (cannot be overridden)

ns { # Equivalent to ns::

    var ::= 0; # Name ns::var (can be overridden by macro or local variable)
    @::var ::= 0; # Name of global variable ::ns::var (cannot be overridden)

    name:: {
        var = 0; # Reference to variable ns::var
        var2 ::= 0; # Name of variable will be ns::name::var2
        ::var = 1; # Variable from global namespace
    };

    :: { # Global namespace
        var = 1; # Name of global variable ::var (can be overridden)
        ::ns::var = 0; # Name of another global variable (cannot be overridden)
    };
};
```

## Namespaces, Modules, and Packages

The namespace in *NewLang* supports both a [modular code structure](/ru/docs/syntax/modules/) like in Java and Python languages, 
and when specifying the full name of an object, program modules and namespaces can be combined.

For example, the full name of a variable can be written with the indication of the program module `\root\dir\module::ns::name::var`, 
where **root** and **dir** are directories in the file system relative to the current module, 
and **module** is the file name, i.e. *root/dir/module.src*.

## Namespace and OOP

In *NewLang*, name decoration (mangling) based on function argument types is not used.

However, when creating unique identifiers for [class](/ru/docs/types/class/) methods, 
*NewLang* uses an approach similar to that used in the Python language. When creating a [class method](/ru/docs/types/class/), 
a global function is created with the class name and method name combined with the separator "**::**".

For example, for the class `:NewClass`, when creating the method `method`, a function named `NewClass::method` will be created.

This method naming scheme fully corresponds to the naming of functions in the namespace, 
allowing class methods to be defined outside the body of the class simply by specifying the desired name in the namespace or explicitly.

Example of creating a method of the class *NewClass* in the namespace:
```
    NewClass::  {
        method() := { };
    }
```

Example of creating a method of the class *NewClass* with specifying the full name:
```
    NewClass::method() := { };
```