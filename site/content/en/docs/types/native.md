---
title: Native Data Types
# linkTitle: Docs
# menu: {main: {weight: 20}}
weight: 70
tags: [типы данных, системые типы, коллекции]
---

Native (machine) data can be variables or functions, and the names of native objects start with the percent symbol "**%**".

In the case of variables, these are binary data located in a single continuous area of machine memory at a specific address and have a strictly defined format.

To use existing libraries, you need to import the native object, thereby creating an object NewLang, 
but with an implementation in another language, for example, in C/C++.

The syntax for importing native objects is the same as for [creating](/docs/ops/create/) regular language objects, 
only the name of the native object needs to be specified as the right operand in the creation operator.

When importing a native object, it is necessary to always specify the variable types, 
as well as the types of arguments and return values for functions.

If the data types of the created object and the native object match (i.e., no type conversion is required), 
then the native object name can be specified with an ellipsis, 
which means that the data types will replicate those specified at the left operand.

```python
:FILE ::= :Plain;

fopen(filename:StrChar, modes:StrChar):FILE ::= %fopen...;
fclose(f:FILE):Int32 ::= %fclose...;
fflush(f:FILE):Int32 ::= %fflush...;
fprintf(f:FILE, format:FmtChar, ...):Int32 ::= %fprintf...;

fremove(filename:String):Int32 ::= %remove...;
frename(old:String, new:String):Int32 ::= %rename...;
```


{{% pageinfo %}}

The following description is under development.

{{% /pageinfo %}}

