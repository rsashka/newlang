---
title: Keyword syntax (DSL)
# description:  Правила синтаксиса языка
# date: 2017-01-05
weight: 200
---


### Features of associative memory
The syntax of the *NewLang* language is based on strict rules without the use of keywords,
and no matter how logical it may seem, association by keywords is much easier to remember, for example, **if**,
than the combination *minus minus right angle bracket* **-->**. 
Because of this, it makes sense to use not a "pure" basic syntax, but a more familiar dialect with the use of keywords.

*NewLang* already contains a set of macros that extend the basic rule-based syntax of *NewLang*,
a set of predefined keywords as in classical programming languages,
which can be adapted or supplemented to suit your own subject area.

### Constants
- *@true* - 1
- *@yes*  - 1
- *@false* - 0
- *@no* - 0

- *@this* или *@self* - Current object
- *@super* - Parent object
- *@latter* or *@last*- Result of the last operation

### Logical operators

- *@and* - Logical **AND**
- *@or* - Logical **OR**
- *@xor* - Logical **Exclusive OR**
- *@not(value)* - Logical negation


### Operators
- *@if(...)* - First conditional statement
- *@elif(...)* - Second and all subsequent conditional statements
- *@else* - Operator *otherwise*

- *@while(...)* - Loop operator with precondition
- *@dowhile(...)* - Loop operator with postcondition
- *@loop* - Infinite loop operator

- *@break $label* - Operator to break out of a named named block of code
- *@continue $label* - Jump operator to the beginning of a named block of code
- *@return( result )* - Operator for returning a value and exiting a function
- *@throw( error )* - Operator for creating an exception and returning an error

- *@match( ... )* - Expression evaluation operator
- *@case( ... )* - Pattern comparison operator
- *@default* - Selection operator

- *@iter(obj, ... )* - Creating an iterator
- *@next(obj, ... )* - Get the next element of the iterator
- *@curr(obj, ... )* - Get the current iterator element
- *@first(obj)* - Go to the first element of the iterator
- *@all(obj)* - Get all elements of the iterator at once

- *@exit(code)* - Terminate program execution and return exit code
- *@abort()* - Terminate program execution without flushing buffers to disk

- *@using(...)* - Use the listed namespaces during [name lookup](/docs/syntax/naming/#using)

## Built-in functions and checks

- *print(format:FmtChar, ...):Int32* - Equivalent to the C/C++ printf function
- *srand(init:Int32):None* - Set the initial value for the pseudo-random number generator
- *rand():Int32* - Get a pseudo-random number

- *@static_assert( ... )* - Evaluate the expression and check its truth during compilation
- *@assert( ... )* - Evaluate the expression and check its truth during execution
- *@verify( ... )* - Evaluate the expression and check its truth during execution

*If the compiler is run with the `--nlc-no-assert` flag, then runtime checks `@assert` are removed from the program text,
and computations inside `@verify` are executed, but their result is ignored.*

## Predefined macros

When the *NewLang* parser is running, several reserved macros are automatically generated,
some of which correspond to preprocessor macros in C/C++.
These predefined macros can be used as regular constants.

- \_\_FILE\_\_ or \_\_FILE_NAME\_\_ - name and full path of the current file
- \_\_LINE\_\_ or \_\_FILE_LINE\_\_ - contains the number of the current line in the file
- \_\_FILE_TIMESTAMP\_\_ - date and time of the last modification of the current file in string representation
- \_\_FILE_MD5\_\_ - md5 hash of the current file in string form
- \_\_COUNTER\_\_ - an integer counter that increases its value with each access


- \_\_DATE\_\_ - start date of the compiler launch (has the same value for all processed files)
- \_\_TIME\_\_ - start time of the compiler launch (has the same value for all processed files)
- \_\_TIMESTAMP\_\_ - date and time when the compiler started running (for example: "Fri 19 Aug 13:32:58 2016")
- \_\_TIMESTAMP_ISO\_\_ - date and time when the compiler started running in ISO format (for example: "2013-07-06T00:50:06Z")

- \_\_NLC_VER\_\_ - Older and younger version of the NewLang compiler (8-bit number, 4 bits per digit)
- \_\_NLC_DATE_BUILD\_\_ - text string with the build date of the NewLang compiler (for example, "06/23/04 20:51:39")
- \_\_NLC_SOURCE_GIT\_\_ - text string with the identifier of the NewLang git repository source files used to build the compiler (for example, "v0.3.0-fef8c371")
- \_\_NLC_SOURCE_BUILD\_\_ - build date and git identifier of NewLang source files in one text line (for example, "v0.3.0-fef8c371 06/23/04 20:51:39")