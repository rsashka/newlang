---
slug: unusual-prog
title: Необычная концепция синтаксиса языка программирования
date: 2021-05-24
---

![КДПВ](/ru/blog/langs.jpeg)

{{% pageinfo %}}

Внимание!!!

Данная статья содержит описание синтаксиса NewLnag предыдущей версии.

Актуальную версию синтаксиса языка можно посмотреть [тут](/ru/docs/).

{{% /pageinfo %}}


Хочу представить на обсуждение читателей немного необычный концепт языка программирования, в котором отсутствует проблема, 
присущая практически всем промышленным языкам -  постоянное увеличение сложности синтаксиса языка из-за его естественного развития по мере выхода новых версий и добавления новых фич.
Эта проблема описана в статье ["Простое сложное программирование"](/ru/blog/complex-prog/) и [Какая "идеальная" цель развития у языков программирования?](/ru/blog/lang-final/)

После нескольких экспериментов с синтаксисом, хочется проверить выработанную идею на широкой аудитории [Хабра](https://habr.com/ru/companies/timeweb/articles/558738/), которая как нельзя лучше подходит для этих целей.

Особенности языка:
- Низкий порог входа и естественное ограничение уровня сложности в течение длительного времени даже при условии постоянного развития самого языка.
- Возможность писать код программы как в декларативной, так и в императивной парадигме с использованием процедурного, модульного и объектно-ориентированного подхода.

Ну и в соответствии с собственным наблюдением [Хабр — ума палата](https://habr.com/ru/post/539674/), буду рад любым комментариям и предложениям, которые помогут протестировать или улучшить предлагаемое решение.  

## Введение
Первой необычной особенностью языка является полное отсутствие зарезервированных ключевых слов. 
Точнее, планируется использовать только одно единственное ключевое слово (название языка), которое может быть как главной точкой входа в приложение, 
так и способом  доступа к настройкам языка под конкретную предметную область, например при необходимости реализации [DSL парадигмы](https://ru.wikipedia.org/wiki/Предметно-ориентированный_язык).

В настоящий момент название языка не выбрано, поэтому для примеров используется просто слово _lang_, которое в будущем будет изменено.

Данная особенность (отсутствие зарезервированных ключевых слов) реализуется за счет того, грамматика языка основана на использовании общеупотребительных символов и классических знаков препинания, 
а все остальные букво-символьные последовательности при парсинге программы рассматриваются как "токены".

Компилятор языка реализуется как [transpiler](https://ru.wikipedia.org/wiki/Транспайлер) - преобразование исходного кода программы, 
написанной на одном языке программирования, в эквивалентный исходный код на другом языке. 
Первые эксперименты с синтаксисом я начинал на Python, но после осознания необходимости разработки не только интерпретатора, 
но и компилятора непосредственно в исполняемый код, решил остановиться на C++. Хотя в принципе, язык реализации может быть любым.

Использование [транспиляции](https://ru.wikipedia.org/wiki/Транспайлер) при реализации компилятора решает сразу множество очевидных и не очень задач. 
Сразу отпадает необходимость разрабатывать низкоуровневый компилятор в исполняемый процессорный код, 
а в моем случае основным бонусом такого подхода становится возможность использования императивной парадигмы программирования за счет вставок кода непосредственно в исходный код приложения на языке реализации. 

И именно этот момент позволяет естественно разделить декларативный и императивный  способ написания программы. 
В этом случае в декларативной парадигме реализуется только одна из трех базовых управляющих конструкций, 
необходимых и достаточных при реализации [любого алгоритма](https://ru.wikipedia.org/wiki/Структурное_программирование) — _следование_. 

Другими словами, в декларативном стиле описывается только _последовательность_ операций, которые следуют одна за другой, 
но отсутствует возможность запрограммировать циклы или ветвление (переходы) по условию. 
Из-за этого в декларативном стиле можно записать только следующие типы языковых конструкций:

- комментарий
- определение переменной и присвоение ей значения
- создание объекта и назначение его свойствам значений по умолчанию
- создание функции (метода для всего класса объектов)
- вызов функции или итератора
- непосредственная вставка кода на языке реализации

Так как цель текущей публикации - протестировать общую концепцию, то начну сразу с примеров без длинного описания подробностей, а некоторые нюансы буду комментировать по ходу дела. 
Тем более, самая главная цель "низкий порог входа" само собой подразумевает возможность самостоятельно разобраться в синтаксисе. 
Тем не менее, совсем без знаний языка реализации не обойтись и начальные знания в программировании на С/С++ все равно необходимы.

## "Привет, мир!" в императивной парадигме
Определение обычной функции выполняется с помощью оператора присвоения ":=", а программный код на языке реализации (в данном случае на C++) 
заключается в фигурные скобки. Простой пример вывода строки на экран выглядит примерно так:

```cpp
    print(str="") := { printf("%s", static_cast<char *>($str)); }
```
Функция **print** с аргументом по умолчанию в виде пустой строки внутри себя вызывает обычный printf из стандартной библиотеки.

Соответственно "Привет, мир!" в императивной парадигме программирования будет выглядеть тривиально:

```cpp
    #!/bin/lang
    print(str="") := { printf("%s", static_cast<char *>($str)); };
    @print("Привет, мир!\n");
```
Из примера видно, что обращение к аргументам внутри С++ кода происходит с использованием символа **$**, который указывается вначале именованного аргумента. 
Кроме этого, для доступа к аргументам функции можно обращаться по их порядковым номерам начиная с первого ($1, $2, $3 и т. д.). 
Зарезервированный аргумент $0 содержит сам объект, метод которого вызывается или _nullptr_, если функция не принадлежит объекту.

_Обычные_ функции являются именно обычными функциями в понимании С/С++. 
Внутри них можно писать совершенно любой код, включая проверки условий, циклы, вызовы других функций и т.д. 
Технически, код такой функции парсится на предмет замены использованных аргументов, 
специальным образом декорируется её имя и добавляются специальные маркеры для идентификации содержимого. 
После этого исходный текст готов для сборки обычным С++ компилятором для превращения в динамическую библиотеку 
и после её загрузки функцию можно будет вызвать в любой момент (для вызова функции перед её именем необходимо указать символ **"@"**).

Непосредственное выполнение файла в режиме интерпретатора происходит в два этапа. На первом этапе из текста программы генерируется временный С++ файл с исходным кодом всех функций, этот файл компилируется gcc и собирается динамическая библиотека.

На втором этапе, собранная динамическая библиотека загружается средой выполнения, а исходный текст программы начинает последовательно выполнятся интерпретатором (все строки, за исключением определений функций). 

Есть еще вариант, когда вместо интерпретации кода программы, генерируется С++ файл не только для функций, но и для основной части приложения. 
Тогда на выходе компилятора будет уже обычный бинарный файл, хотя в том случае оперативно поправить текст программы уже не получится.

## Логическое программирование в декларативной парадигме
Так как самым известным языком логического программирования в декларативном стиле считается Prolog, 
поэтому приведу простой пример программы Brother (поиска братьев) на Прологе и эквивалентный ему код на новом языке.

Prolog:

```bash
    male("Tom").
    male("Tim").
    male("Jake").
    female("Janna").
    parent("Tom","Jake").
    parent("Janna","Jake").
    parent("Tom","Tim").

    brother(X,Y):- parent(Z,X),parent(Z,Y),male(X),male(Y),X\=Y.
```
Вывод: **(Jake, Tim) (Tim, Jake)**

Тот же самый пример:

```bash
    #!/bin/lang
    human:=@term(sex=,parent=);
    Tom:=@human(male);
    Janna:=@human(female);
    Jake:=@human(male, (Tom, Janna,));
    Tim:=@human(sex=male, parent=(Tom,));

    human::brother(test=human!) &&= $0!=$test, $0.sex==male, @intersec($0.parent, $test.parent);

    human.brother?
```
Вывод: **[Tim.brother(Jake), Jake.brother(Tim),]**

Надеюсь, что синтаксис интуитивно понятен, особенно при наличии эквивалентного кода на Прологе, но на всякий случай поясню отдельные моменты.

```bash
    human:=@term(sex=,parent=);
```
В этой строке создается объект с именем "human" и двумя свойствами "sex" и "parent", 
значения которых по умолчанию не определены, а родителем является системный объект "term". 
Использование символа **"@"** в начале термина обозначает вызов существующей функции с параметрами, указанными в скобках. 
В данном случае вызывается конструктор системного объекта "term", а возвращаемым результатом является новый термин "human", 
который можно воспринимать как один экземпляр класса, так и как имя целого класса, если он будет выступать в качестве родителя для других объектов.

```bash
    Tom:=@human(male);
    Janna:=@human(female);
    Jake:=@human(male, (Tom, Janna,));
    Tim:=@human(parent=(Tom,), sex=male);
```
Первые две строки создают объекты "Tom" и "Janna", у которых свойство "sex" установлено в значения "male" и "female" соответственно. 
А в последней строке при создании объекта Tim значения свойств задается с указанием их имени.

Конструкции _(Tom, Janna,)_ и _(Tom,)_, это определение константного литерала с типом словарь, которые присваиваются указанным свойствам.

Чтобы определение словаря не путать с указанием аргументов при вызове функции, в нем содержится обязательная финальная запятая перед закрывающей скобкой. 
Это правило (финальная запятая перед закрывающей скобкой) действует и при определении литерала массива, только для записи которого используются скобки не круглые, а квадратные (например **[,]** - пустой массив). 
Основным отличием словаря от массива, заключаются в способах доступа к их элементам. 
Для доступа к элементу массива используется целочисленный индекс, а в словаре может быть использован как индекс, так и имя элемента при его наличии. 
Между словарем и массивом есть еще отличия, но сейчас они не принципиальны.

И предпоследняя строка в примере:

human::brother(test=human _!_) **&&=** _$0_ != _$test_, _$0_.sex==male, @intersec(_$0_.parent, _$test_.parent);

это определение <i>простой чистой</i> функции "brother" для всех объектов, производных от "human". 
Данная функция принимает один именованный аргумент "test", значением по умолчанию которого является _итератор_ объектов класса "human".

Оператор **&&=** означает определение _простой чистой_ функции, т. е. функции без внешних зависимостей, 
которая не имеет доступа к глобальному контексту, а результат выполнения которой является логическое значение, 
которое вычисляется по схеме [логическое И](https://ru.wikipedia.org/wiki/Конъюнкция) для всех условий, указанных в теле функции через запятую.

Выполнение и вывод результата выполнения программы происходит в строке 
`human.brother?`

Восклицательный и вопросительный знаки обозначают [итератор](https://ru.wikipedia.org/wiki/Итератор). Итератор "**!**" возвращает один текущий элемент из коллекции и передвигает указатель на следующий, 
а итератор "**?**" возвращает сразу всю коллекцию объектов целиком.

Другими словами при выполнении строки программы _human.brother **?**_ происходит последовательный перебор всей коллекции объектов типа "human", 
где у каждого объекта выполняется функция "brother" с аргументами по умолчанию. 
А так как в качестве аргумента по умолчанию указан итератор _human **!**_, то в качестве аргумента последовательно предается каждый из элементов указанного класса. 
По сути, при выполнении этой строки происходит полный перебор всех возможных комбинаций объектов класса "human" каждый с каждым.

> Более подробное описание работы итератора и причина выбора такого синтаксиса  опубликовано в отдельной статье <a href="https://habr.com/ru/company/timeweb/blog/559916/">Лаконичный итератор для декларативного синтаксиса</a>

Результат выполнения будет состоять из тех пар объектов, для которых функция "brother" вернет истину, т.е. результат выполнения будет массив из двух пар объектов **[Tim.brother(Jake), Jake.brother(Tim),]**.


## Формулировка проверяемой концепции
Тестируемая концепция заключается в следующем. Синтаксис языка программирования состоит как бы из двух частей, 
каждая из которых является относительно независимой и в тоже время они связаны между собой.

Первая часть сложная  — предназначена для описания алгоритмов в _императивном стиле_ на обычном языке программирования, 
поэтому внутри функций разработчику доступны все возможности языка реализации.

Вторая часть простая (по сравнению с первой) — предназначена только для логического описания задачи в _декларативном стиле_. 
Именно эта часть используется для начального уровня использования, а естественное ограничение только на последовательное выполнение операторов 
(т.е. отсутствие циклов и ветвлений) не доставляет чрезмерных сложностей в понимании исходного текста программы даже в будущем по мере развития языка.

Связь между двумя вариантами синтаксиса прозрачна и реализуется за счет совместного использования переменных и функций, 
декларация и обращение к которым происходит в едином стиле в обоих случаях за счет унификации синтаксиса на уровне использования знаков препинания.

Сейчас я специально не привожу полного синтаксиса и подробного описания остальных возможностей языка, т.к. компилятор пока не готов для публичной презентации. 
Но буду крайне благодарен за любой отклик или дельные предложения в отношении предложенной концепции. 

Особенно это касается возможных конфликтов по синтаксису в представленных примерах.
