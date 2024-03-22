---
title: Character Strings
weight: 20
# tags: [типы данных, строки]
---

*NewLang* supports two types of strings:
- *:StrChar* - universal byte UTF-8 strings
- *:StrWide* - system character (wide characters) 

The single element of a universal UTF-8 string is one byte, 
while the single element of a system character string is a wide character *wchar_t*.
And although the strings differ in the types of single string elements, the internal representation of both types of strings is the same.
All data of character strings is stored as a universal byte UTF-8 sequence.

### Byte Strings *:StrChar* {#StrChar}

The basic element of a universal byte string is one byte (specifically *:Int8*, i.e. an 8-bit signed integer).
The number of characters in a byte string is returned in bytes, and accessing a string element by index is done by the byte of the character sequence.

*Since these strings are interpreted as a sequence of UTF-8 characters, caution must be taken when modifying individual bytes!*


### System Strings *:StrWide* {#StrWide}

The individual element of the system string is a wide character *wchar_t*, and the number of elements in the system character string is returned in wide characters.
The size of the *wchar_t* type depends on the operating system, so the size of one character :StrWide in Windows will be 2 bytes, and in Linux 4 bytes,
which is related to their implementation specifics.

The main purpose of system strings is to simplify working in a text terminal,
as one character always corresponds to one character cell without the need for constant conversion of UTF-8 code points.


### String Formatting {#format}
Any variable can be accessed as a function (by specifying parentheses after its name) to create a copy/clone of the object, 
and for strings, this operation can be used as a template for formatting data.

String formatting can be done in two ways:
- Specify a format string with the types of data to be output (the format string corresponds to formatting in the standard [printf](https://en.wikipedia.org/wiki/Printf) function).
To apply this formatting variant, the format string must specify the corresponding data type 
(*: FmtChar* or *: FmtWide* for regular and wide characters, respectively).  
With this formatting method, named arguments cannot be used, and the compiler checks the types of the passed values 
for compliance with the format string only *during compilation* of the program text. 
*This type of format string can be used as the last argument of a function to check the types of all subsequent arguments 
for compliance with the format string.*

- In all other cases, any strings and named arguments can be used as the format string, 
and the format itself corresponds to the format string from the [{fmt}](https://fmt.dev/latest/syntax.html) library. 
The only difference is the ability to specify not only the positional number of the argument, but also its name. 
Argument validation against the format string is performed during string cloning both at compile time and during program execution.

The formatting method is used by default for any type of string, but the format string is not checked in the function arguments.

For example:
```python
    $fmt := '%s: %d':FmtChar; # Formatting like in printf
    $result := $fmt('value', 123); # "value: 123" - Check compile time only! 

    $template := "{name} {0}"; # fmt::format equivalent "{1} {0}"
    $result := $template("template", name = "String"); # result = "String template"
```

