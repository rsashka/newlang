---
title: Data type conversion
slug: convert
weight: 100
tags: [types, convert]
---

Despite the dynamic typing of the language, if the type of a variable is specified explicitly, then automatic type casting is not performed.
and to assign a value of an incompatible type to a variable, an explicit conversion is required.

Since symbolic type names are implementation details, an explicit conversion to a specific data type is performed
by calling a function with a type name, i.e. :Bool(), :StrWide(), :Int64(), etc.

Examples:

## Strings and Tensors
```python
# Create a tensor from a wide character string with automatic output of size and data type
# (type :Int32 will be on Linux systems, and on Windows the tensor type will be :Int16)
> tstr := :Tensor[...]("Тест"); 
[1058, 1077, 1089, 1090,]:Int32

# Same thing, but a tensor of a given dimension
> t2 := :Tensor[2,2]("Test");
[
   [1058, 1077,], [1089, 1090,],
]:Int32

# Create a character string from a tensor
> :StrWide(tstr) 
Тест

# Change tensor data type without changing dimension
> :Float64[...](t2)
[
   [1058, 1077,], [1089, 1090,],
]:Float64

# Change tensor dimension and type
# (in this case with partial data loss)
> t3 := :Int8[4]( t2 ) 
[34, 53, 65, 66,]:Int8

# Boolean value - scalar
>:Bool( [0, 53, 65, 66,] );
1:Bool

# Or like this (zero-dimensional tensor)
>:Bool[0]( [0, 53, 65, 66,] );
1:Bool

# Boolean tensor without resizing
>:Bool[_]( [0, 53, 65, 66,] ); 
[0, 1, 1, 1,]:Bool

```

## Dictionaries
```python
# Create a tensor from a dictionary with automatic size output
>:Tensor[...]( (1,2,3,) );
[1, 2, 3,]:Int8

# Convert a byte string to a one-dimensional tensor
>:Tensor[_]( 'first second' )
[102, 105, 114, 115, 116, 32, 115, 101, 99, 111, 110, 100,]:Int8

# Get a tensor from a dictionary with the same data
>:Tensor[...]( ('first', space=32, second='second',) )
[102, 105, 114, 115, 116, 32, 115, 101, 99, 111, 110, 100,]:Int8

# Boolean value - scalar
>:Bool( (0,1,2,3,) ); 
1:Bool

# Boolean tensor
>:Bool[...]( (0,1,2,3,) );
[0, 1, 1, 1]:Bool


# Differences between "..." and "_"

# Automatic output of measurement sizes
>:Tensor[...]( ((1,2,3,), (4,5,6,), (7,8,9,),) );
[
     [1, 2, 3,], [4, 5, 6,], [7, 8, 9,]
]:Int8

# One-dimensional tensor with arbitrary size
>:Tensor[_]( ((1,2,3,), (4,5,6,), (7,8,9,),) ); 
[ 1, 2, 3, 4, 5, 6, 7, 8, 9,]:Int8

```
