---
slug: reference-solution
title: Possible solution to the problem of references in programming languages
date: 2024-04-23
---

![](https://habrastorage.org/webt/xl/4d/uy/xl4duyyseltgpsqlhu09_vp-zvk.jpeg)

Every programmer is familiar with the concept of "reference." This term usually refers to a small object whose main task is to provide access to another object physically located elsewhere. Because of this, references are convenient to use, they are easily copied, and they make it very easy to access the object to which the reference points, allowing access to the same data from different parts of the program.

Unfortunately, manual memory management, or more precisely, manual memory control, is the most common cause of various errors and vulnerabilities in software. All attempts at automatic memory management through various managers are hampered by the need to control the creation and deletion of objects, as well as periodically run garbage collection, which negatively affects application performance.

However, references in one form or another are supported in all programming languages, although the term often implies not completely equivalent terms. For example, the word "reference" can be understood as a reference as an address in memory (as in C++) and a reference as a pointer to an object (as in Python or Java).  
  
Although there are programming languages that try to solve these problems through the concept of "ownership" ([Rust](https://www.rust-lang.org/), [Argentum](https://aglang.org/), or [NewLang](https://newlang.net/)). The possible solution to these and other existing problems with references will be discussed further.


## What types of references exist?  
For example, in the C language, there are pointers, but working with them is not very convenient and at the same time very dangerous due to the presence of pointer arithmetic (the ability to directly modify the pointer address to data in the computer's memory). In C++, a separate entity appeared — reference, and in C++11, references received further development, rvalue-references appeared.  
  
While in C++, there are several types of references at once, Python developers probably deliberately tried to "simplify" working with references and generally abandoned them. Although de facto in Python, every object is a reference, although some types (simple values) are automatically passed by value, whereas complex types (objects) are always passed by reference.

## Circular references  
There is also a global problem of circular references that affects almost all programming languages (when an object directly or through several other objects points to itself). Often, language developers (especially those with garbage collectors) have to resort to various algorithmic tricks to clean up the pool of created objects from such "frozen"  link and circular references, although usually this problem is left to developers, for example, in C++ there are strong (std::shared_ptr) and weak (std::weak_ptr) pointers.

## Ambiguous semantics of references  
Another equally important but often ignored problem with references is the language semantics for working with them. For example, in C/C++, separate operators like asterisk "\*", arrow "->", and dot "." are used to access data by reference and by value. However, working with reference variables in C++ is done as with regular variables "by value", although in fact this is not the case. Of course, with explicit typing in C++, the compiler will not allow mistakes, but when reading the code normally, you will not be able to distinguish a reference variable from a regular one "by value".  
  
In Python, it is very easy to confuse how a variable will be passed as an argument to a function, by value or by reference. This depends on the data contained in the variable. The fact that Python is a dynamically typed language adds a special twist, and in general, it is not known in advance what value is stored in the variable.

## Who is to blame and what to do?  
It seems to me that the main reason, at least for the ambiguous semantics, is the constant [growth of complexity of development tools](https://habr.com/ru/companies/timeweb/articles/551754/), and as a result - the complication and refinement of the syntax of programming languages for new concepts and capabilities while maintaining backward compatibility with old legacy code.  
  
But what if we start from a clean slate? For example, a universal concept of object and object reference management that does not require manual memory management by the user (programmer), for which no garbage collector is needed, and errors when working with memory and object references become impossible due to full control of memory management even at the stage of compiling the application source code!  
  
#### Terms:  
Object - data in the computer's memory in machine (binary) representation.  
Variable - a human-readable identifier in the program body, which is uniquely determined by its name and identifies the object (the actual value of the object or a reference to it). Variables can be:  
- Owner variable - the only constant reference to the object (shared_ptr).  
- Reference variable - a temporary reference to the object (weak_ptr).

### Possible operations:  
- creating a new owner variable and initializing the object value.  
- creating a reference variable to an existing owner variable.  
- assigning a new value to the object by the owner variable name.  
- assigning a new value to the object pointed to by the reference variable.  
- assigning a new reference to the reference variable from another owner variable.  
  
Example of source code and conditional abbreviations:  
- "**&**" - creating a reference to a variable  
- "**\***" - accessing object data  
  
```python
# variables - owners
val1 := 1;  
val2 := 2;  
# val1 = 1, and val2 = 2  
  
val1 = val2; # Error - only one owner!  
*val1 = *val2; # OK - value is assigned 

# To avoid too many asterisks in your code, you can omit them when accessing
# object data if the owner variable is used in an expression as an rvalue
*val1 = val2; # This is also valid  
# val1 = 2, and val2 = 2  
  
# variables - references  
ref1 := &val1;  
ref2 := &val2;  
# ref1 -> val1, and ref2 -> val2  
  
ref1 = 3; # Error - variable is a reference!  
*ref1 = 3; # OK - value is assigned "by reference"  
# val1 = 3, and val2 = 2  
# *ref1 = 3, *ref2 = 2  
  
*ref2 = *ref1; # OK - one value is assigned to another value "by reference"
# val1 = 3, а val2 = 3
# *ref1 = 3,  *ref2 = 3

ref1 = ref2; # OK - one link is assigned to another link
# ref1 -> val2, а ref2 -> val2
# *ref1 = 3,  *ref2 = 3

*val2 = 5;
# *ref1 -> 5,  *ref2 -> 5


# Obtaining data "by reference" for fields of a structure (class).

class A {
  A ref;
};

A owner := A();
owner.ref := &owner;  # Circular link :-)
# owner.*ref -> owner

A link := &owner;
# *link.ref  - reference field
# *link.*ref -> owner

```

With this syntax, everything becomes simple, visual, and clear. 
[But if I missed something somewhere, please write in the comments](https://habr.com/ru/articles/809877/) or a personal message.

