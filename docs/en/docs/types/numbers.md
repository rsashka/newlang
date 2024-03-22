---
title: Numbers
weight: 10
---

*NewLang* is a language with dynamic typing, and explicit type declaration does not affect the memory size occupied by a variable. 
Type information is used when checking their compatibility, when assigning a value of a different type to an existing object. 
This operation is possible only when the types are compatible and allow for automatic conversion. 
This is true both during parsing/compilation of the source text and during execution in interpreter and/or compiled file modes.

### Arithmetic types

All numbers (except rational) in *NewLang* are tensors, i.e. arrays of one type with an arbitrary number of dimensions 
and the same column size in each. A unit number is also a tensor of zero size.

Only signed integers are supported, as there is no special need for unsigned numbers, 
and there are many problems with them that can be found very easily. 

> *Issues with unsigned numbers (from the internet):*
> First, subtracting two unsigned numbers, for example 3 and 5. 
> 3 minus 5 equals 4294967294 because -2 cannot be represented as an unsigned number.
> Second, unexpected behavior may occur when mixing signed and unsigned integer values. 
> C++ can freely convert signed and unsigned numbers, 
> but does not check the range to ensure that you are not overflowing your data type.

The names of the built-in arithmetic types: :Int8, :Int16, :Int32, :Int64, :Float16, :Float32, :Float64, :Complex16, 
:Complex32, :Complex64 speak for themselves. 
And although among them there are names that are inherent to unsigned numbers (:Byte, :Word, :DWord, etc.), 
they are synonyms and are used for interaction with [native C++ code](/docs/types/native/).

#### Logical type {#bool}
A separate type is the logical type: Bool, which can only take values 0 or 1 (*false*/*true* respectively), 
and depending on the operation being performed, it can also be classified as integer types, 
or not included in their composition (this approach to interpreting the logical data type was taken from the Torch library).
>  ```
>  // Treat bool as a distinct "category," to be consistent with type promotion
>  // rules (e.g. `bool_tensor + 5 -> int64_tensor`). If `5` was in the same
>  // category as `bool_tensor`, we would not promote. Differing categories
>  // implies `bool_tensor += 5` is disallowed.
>  //
>  // NB: numpy distinguishes "unsigned" as a category to get the desired
>  // `bool_tensor + 5 -> int64_tensor` behavior. We don't, because:
>  // * We don't want the performance hit of checking the runtime sign of Scalars.
>  // * `uint8_tensor + 5 -> int64_tensor` would be undesirable.
>  ```

### Tensor Indexing {#indexing}
Access to tensor elements is done by an integer index starting from 0. 
For a multidimensional tensor, element indices are listed in square brackets separated by commas. 
Access to elements through a negative index is supported, which is handled in the same way as 
in Python (-1 is the last element, -2 is the second to last, and so on).

Ranges can be used as tensor indices, which are handled the same way as in Python, 
as well as the value `:None` and ellipsis `...`. The value `:None`, meaning absence of an index, 
signifies an arbitrary size of the tensor in *one* specific dimension, while the ellipsis `...` denotes 
an arbitrary dimension in *any* number of dimensions (hence, it can appear in the tensor index only once).

The tensor literal in the program text is written in square brackets with a mandatory closing comma, i.e. `[1, 2,]` - this is 
a literal one-dimensional tensor of two numbers. After the closing bracket, the tensor type can be explicitly specified. 
If the type is not specified, it is automatically output based on the specified data and the minimum possible byte size that allows 
all values to be saved without loss of accuracy is selected.

Examples of creating tensors and transforming their dimensions can be found [here](/docs/types/convert/).

### Rational Numbers {#rational}

For calculations with unlimited precision in *NewLang*, a separate type is used - rational numbers. 
They are written in the form of a common fraction, in which the numerator must be an *integer*, 
and the denominator *natural* (an integer without zero). 

A backslash is used as the fraction separator, i.e. `1\1` - rational number 1, `-5\1` - rational number -5, etc.

