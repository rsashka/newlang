---
title: Оценка выражения
# description: Виды перемнных и правила их наименования
weight: 200
---


Синтаксическая конструкция с помощью которой реализуется примерный аналог оператора *switch* или *match* выглядит следующим образом:
```python
    [ $var ] ==> {
        [1] --> { code }; # Выполнится проверка условия $var == 1
        [1, 2] --> { code }; # Выполнится проверка условия ($var == 1 || $var == 2)
        [...] --> { code default }; # Ветка условия иначе
    };
```
Или тоже самое, но с использованием [макросов из модуля dsl](/docs/syntax/dsl/):
```python
    @match( $var ) ==> {
        @case( 1 ) { code };
        @case( 1, 2 ) { code };
        @default { code default };
    };
```

Этот оператор очень похож на *Pattern Matching*, но все же не является сопоставлением с образцом, 
а скорее более краткая запись множественного оператора сравнения, 
так как в качестве оператора для оценки могут быть использован любые имеющиеся операторы сравнения на равенство:
- **==>** — проверка на равенство с приведением типов;
- **===>** — проверка на точное равенство;
- **~>** — проверка типа (имени класса);
- **~~>** — утиная типизация;
- **~~~>** — строгая утиная типизация.

Но если в качестве оператора сравнения использовать оператор утиной типизации, то оценка выражения превращается в классический *Pattern Matching*:
```python
    $value := (f1=1, f2="2",);
    @match( $value ) ~~> {
        @case((f1=_, ), (f1=_, f2=0, )) { ... code ... }; # Поле f2 отсутствует или число 
        @case((f1=_, f2="",), (f1=_, f2='',)) { ... code ... }; # Поле f2 строка
        @default { ... code default ... }; # Код по умолчанию
    };
```

