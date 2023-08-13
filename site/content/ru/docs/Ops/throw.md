---
title: Операторы прерывания выполнения (оператор возврата)
# description: >    Виды перемнных и правила их наименования
weight: 500
---

Операторы прерывания выполнения (оператор возврата)

### Прерывания, возврат и обработка ошибок
Изменена, а точнее полностью переделана идеология возвратов из функций и обработки ошибок. 
Теперь она чем-то похожа на подход, примененный в Ruby. 
Любая последовательность команд заключенные в фигурные скобки (в том числе тело функции), 
рассматривается как блок кода у которого нет специального оператора аналога **return**, который возвращает какое либо значение. 
Просто любой блок кода всегда возвращает последнее вычисленное значение (это чем то похоже на оператор «запятая» в языках C/C++).

Для того, чтобы прервать выполнение кода используются две разные команды - прерывания, которые 
условно можно назвать *положительным* и *отрицательным* результатом, что примерно соответствует семантике их записи. 
"Отрицательное" прерывание записывается в виде двух минусов, а "положительное" прерывание в виде двух плюсов, т.е. **-\-** или **++**.

По умолчанию данные операции возвращают пустое значение. Чтобы прерывание вернуло результат, возвращаемые данные нужно записывать между 
парой соответствующих символов, т.е. **-\-** 100 **-\-**, что является близким аналогом оператора `return 100;` в других языках программирования, 
а **++**«Строка»**++** - аналогом `return «Строка»;`.

Хотя более точным аналогом этих операторов будет все таки не **return**, а **throw**, т.к. эти команды не только прерывают 
выполнение последовательности команд в блоке, но их еще можно «ловить». 
Для этого используется блок кода с соответствующей семантикой, **{+** … **+}** - блок кода, который перехватывает 
положительные прерывания и **{-** … **-}** - блок кода, который перехватывает прерывания, созданные операторами **--**. 

Подобная концепция (в явном виде не разделять возвраты из функций и обработку исключений), и оставить конкретную реализацю 
на усмотрение компилятора, хоть и выглядит немного необычной, но позволяет реализовывать несколько очень полезных финтов, 
которые сложно реализуемые в обычных языках программирования.

Например, возврат из нескольких вложенных функций без необходимости обрабатывать результат возврата каждой из них. 
В этом примере функция *Test* перехватывает "положительные" прерывания из вложенных функций:
```bash
    Test0(arg) := { if($arg==0) return("DONE - 0"); «FAIL» };
    Test1(arg) := { if($arg==1) return("DONE - 1"); Test0($arg); };
    Test(arg) := {+ if($arg >= 0) Test1($arg); $arg; +};

    Test(0); # Вернет «DONE — 0» возврат из вложенной функции Test0
    Test(1); # Вернет «DONE — 1» возврат из вложенной функции Test1
    Test(2); # Вернет «FAIL» возврат из вложенной функции Test0
    Test(-2); # Вернет -2 — возврат из функции Test
```
Есть еще блок `{* ... *}`, который перехватывает оба типа прерываний. 
Такой блок кода поддерживает типизацию возвращаемого значения, что позволяет в явном виде указывать типы данных, которые нужно перехватывать. 
Например, `{* ... *} :Type1` — будет перехвачено прерывание вида `++ :Type1 ++` или `--:Type1--`, 
что позволяет очень гибко формировать логику работы программы.

Блоки кода с перехватом исключений также поддерживают оператор иначе (*else*) который, 
по аналогии с оператором *else* в циклах, выполняется только тогда, если прерывания не произошло.

Можно указать сразу несколько типов, которые нужно перехватывать:
```bash
    {* 
        ....
    *} <:Type1, :Type2, :Type3>;
```


