---
title: Типы данных
#description: > 
#    Типы данных
# linkTitle: Docs
# menu: {main: {weight: 20}}
weight: 2
simple_list: true
---

*NewLang* является языком с динамической типизацией и явное указание типа переменной не обязательно 
и на прямую не влияет на размер памяти, котрую она занимает. 
Тип переменных используется при проверке их совместимости, когда существующему объекту присваивается другое значение. 

Присвоение нового значения переменной возможно только тогда, когда типы данных между собой совместимы или допускают автоматическое приведение. 
Это справедливо как во время компиляции исходного теста, так и во время выполнения.

*NewLang* имеет закрытую иерархическую систему типов, т.е. любой новый тип данных, которые создает пользователь,
обязательно должен быть наследником от уже существующего.
Это напоминает реализуцию ООП в яызке Java, где каждый класс имеет в своем составе наследников *Object*.

За счет закрытой иерархии типов реализуется и парадигма [обобщённого программирования (generics)](/docs/types/generics/).

Каждый тип данных имеет *краткое имя* и функцию - конструктор.

*Краткое имя* типа всегда начинается с символа двоеточия "**:**" 
и определяется в [глобальной области видимости](/docs/syntax/namespace/).

Функция - конструктора типа, имеет тоже самое *краткое имя* типа, но без символа двоеточия "**:**",
и как и любая функция может располагаться в пользовательском [пространстве имен](/docs/syntax/namespace/).

Создать новый экземпляр типа нужно вызвать функцию - конструтор типа.

Создать новый экземпляр типа данных можно записав *краткое имя типа* как вызов функции, 
либо вызвать непосредственно функцию - конструтор типа, с указанием её полного пространства имен.

Типы данных могут быть синонимами, т.е. два типа данных могут иметь одинаковое *краткое имя*,
(соответствено, и одитнаковые функции конструкторы),
но сами функции должны располагаться в разных областях [пространства имен](/docs/syntax/namespace/).

В этом случае, создание объекта - синонима типа, требует вызова функции конструктора с указанием её полного пространства имен,
в противном случае будет вызвана функция согласно алгоритму [разрешения имен (name lookup). ](/docs/syntax/naming/).


Возможно явное приведение одного типа данных к дргуому. 
Для этого применяется *краткое имя* (которое начинается с символа двоеточия "**:**"), как вызов функции, 
в параметрах которой передается один или несколько объектов, тип которых нужно привести к требуемому.


Типы данных разделяются на простые и сложные (составные).

Простые типы данных не могут  Поля у простых типов данных не могут быть расширены 


Создание типов происходит согласно [правилам синтаксиса](/docs/ops/create/).
Имя типа всегда начинается с символа двоеточия "**:**" и определяется в [глобальной области видимости](/docs/syntax/namespace/).
Создание сложных [типов (классов)](/docs/types/class/), может располагаться в пользовательском [пространстве имен](/docs/syntax/namespace/).

, в том числе и [конструктор классов](/docs/types/class/), 
---


