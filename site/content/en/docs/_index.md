---
title: Documentation
linkTitle: Documentation
menu: {main: {weight: 20}}
weight: 20
---

Основные материалы по языку программирования NewLang.

Описание акутально для текущей версии 4.0


clang-17 не поддерживает:
|  Feature          |      Proposal     | Available |
|-------------------|-------------------|---------------------------------------|
| Coroutines        |       P0912R5     |*Partial*   Fully supported on all targets except Windows, which still has some stability and ABI issues. 
| Extended floating-point types and standard names  | P1467R9 	| No
| Concepts                                          | P0848R3   | Clang 16 (Partial)
| Modules            |                              | No        |


### Начиная с C++ should adopt the same characters for C++26.
- Add **@**, **$**, and ` to the basic character set          P2558R2 	Yes