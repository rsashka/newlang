---
slug: building-blocks
title: Building blocks in programming languages
date: 2024-01-12
---

Practically all programming languages are built either on the principle of similarity (to make like this one, only with its own blackjack) or to realize some new concept (modularity, purity of functional calculations, etc.). Or both at the same time. 

But in any case, the creator of a new programming language doesn't take his ideas randomly out of thin air. They are still based on his previous experience, obsession with the new concept and other initial settings and constraints.

Is there a minimal set of lexemes, operators, or syntactic constructs that can be used to construct an arbitrary grammar for a modern general-purpose programming language?

## Introduction
I confess at once that I cannot unambiguously list a minimal set of basic operators and constructs that would be sufficient for a *modern* programming language. Moreover, I'm not sure that such a set is even possible, since many constructs can be represented using other, lower-level constructs (e.g. conditional/unconditional transition).  I remember about the Turing machine, but I'm interested in real programming languages, not machine instructions at an abstract executor.

Therefore, as the basic building blocks of programming languages we can safely accept those features that were invented and implemented by developers of mainstream languages. And it's probably better to start with criticizing separate and well-known fundamental concepts. And no, it's not the goto operator! 

### Strange increment and decrement (++ and --).

In my opinion, the most unambiguous operators are the operators for increment and decrement, i.e. arithmetic increase or decrease of a variable value by one. They cause serious confusion in the strict grammar of the language, which, in my opinion, should be as transparent and *ambiguous* as possible.

The main problem with these operators is that, as arithmetic operators, they *modify* the value of a variable, whereas all other arithmetic operators operate on *copies* of values without modifying the variable itself directly.

I may object that the operators +=, -=,\*= or \= also change the value of a variable, but I would like to point out that this is only a simplified notation of a combination of two operators, one of which is intended to assign a new value to a variable, so no objections are accepted. :-)

And if we remember that increment and decrement operators can be prefix and postfix, then in combinations with address arithmetic (\*val++ or some ++\*val++), brain explosion with possible errors is simply guaranteed.

### Few value assignment operators
Yes, you read that right! I do criticize the one-value assignment operator “**=**” because I think it is not quite complete. But unlike increment and decrement, which the language lexicon can easily do without, there is no way to do without the assignment operator!

But my criticism is directed not at the operator itself, but at its incompleteness and creation of additional confusion in some programming languages. For example, in the same Python it is impossible to understand whether a variable is being created (i.e. the first use of a variable) or whether it is assigning a value to a variable that already exists (or whether the programmer has made a typo in the variable name). 

If we remember “if you criticize, suggest”, it would be correct to make two different operators: the *assign value* operator and the *create variable* operator (in C/C++, the logic of creating a variable is performed by specifying the type of the variable when using it for the first time). 

In other words, instead of one “create and/or assign value” operator, it is better to use two or even three operators: creating a new variable (**::=**), only assigning a value to an already existing variable (**=**) and creating/assigning regardless of the variable's existence (**:=**) - i.e. an analog of the current **=** operator.

And in this case, the compiler could control the creation or reuse of a previously created variable according to the programmer's intentions already at the level of the initial syntax.

You can also add a “value exchange” operator, some **:=:**. In essence, it is an analog of std::swap() in C++, only at the level of language syntax.

### Always an extra data type

All mass programming languages usually contain numbers with different digit capacity. This is a compulsory necessity because the digit capacity of calculations is determined by the hardware level and language developers cannot ignore it.

Another thing is a Boolean (logical) data type. In the description of one language I even met this:
```
Bool 1 Byte truth value
(Bool16) 2 Byte truth value
(Bool32) 4 Byte truth value
(Bool64) 8 Byte truth value
```  
And when you dig a little deeper, everything comes down to one single bit, which can be used to represent two opposite states YES/NO, true/false, 1/0....

But let me tell you, if it's a 1 or a 0, why not immediately define that a logical type is a number with one digit? (as it is done in LLVM!).

After all, there is no worse job than the pointless work of converting numbers to logical values and vice versa:

> Java has some pretty strict restrictions on the boolean type: boolean values cannot be converted to any other data type, and vice versa. In particular, boolean is not an integer type, and integer values cannot be used in place of boolean values.

And also, in some programming languages that support Empty/None, a boolean data type can turn into a tribulus at all, for example in the case of default function parameters, when the boolean argument has the state “not set” added to it. But from the point of view of using non-initialized variables, it is at least understandable and logically explainable.


### Null pointer
In one way or another, all mainstream programming languages contain a data type called *reference*. And in some languages, reference types can be of several kinds at once. 

However, the presence of reference data types adds several uncertainties at once, such as memory and shared resource management. Besides, if address arithmetic (explicit or not) is present, it immediately becomes necessary to use a special reserved value called “null pointer”, **NULL**, **nil**, **nullptr**, etc. depending on the language.

The presence of such a value forces language developers to considerably complicate the syntax and logic of working with pointers by controlling the explicit/implicit possibility of storing a null pointer in a reference variable.

But if the language compiler will manage and control reference data types and shared resources itself, the very concept of “null pointer” becomes unnecessary and will be hidden from the programmer in the implementation details.
### Last operation result

There are situations when a system variable with the value of the result of the last operation is missing. Something analogous to `$?` in bash scripts, but at the level of Python or C/C++ source code. 

But I don't mean a specific physical variable, but some generalized identifier with the result of the last operation. A pseudo-variable that is managed by the language compiler. In other words, so that the type of this pseudo-variable changes depending on which operation was the last one.

This could simplify the solution of frequently occurring tasks, for example, to get the last value after exiting a loop.

Or such a pseudo-variable could simplify the syntax of exception handling, where interception is implemented on the basis of types. But at the same time with the type of the exception to be intercepted you have to define a variable, even if it is not used in any further way.

### Clean functions

Also, I would sometimes like to be able to create pure functions in C/C++ or Python, so that the compiler itself would control the prohibition of accessing global variables or non-pure functions at the language syntax level, and this would be checked at compile time.

### Empty variable name

And lastly, I would like to say that C++ lacked the empty variable “**_**” (as in Python) very much. But it seems to have been introduced in the last proposals of the standard, so we will be happy starting from C++26 :-))


#### In conclusion

When writing this article, I tried to abstract and approach my more than thirty years of development experience without bias, but I'm not sure that I succeeded, so I'll be glad to receive any remarks and objections in comments.

If you don't mind, [write in the comments](https://habr.com/articles/812117/comments/) what features in modern programming languages you think hinder more than help, or vice versa, what operators/syntactic constructs you miss. 

It's always interesting to find out what you missed or forgot.
