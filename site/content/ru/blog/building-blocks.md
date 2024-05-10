---
slug: building-blocks
title: Building blocks in programming languages
date: 2024-05-03
---

В результате обратной связи по статье с [размышлениями о структурном программировании](https://habr.com/ru/articles/784238/), было много отзывов и споров в комментариях, за которые я хочу сказать всем большое спасибо. 

И под впечатлениями от обсуждения предыдущей статьи я задался вопросом, а существует ли минимальный набор лексем, операторов или синтаксических конструкций, с помощью которых можно построить произвольную грамматику для современного языка программирования общего назначения?


## Ведение
Практически все языки программирования строятся либо по принципу подобия (сделать как вот этот, только со своим блекждеком), либо для реализации какой-то новой концепции (модульность, чистота функциональных вычислений и т.д.). Либо и то и другое одновременно. 

Но при любом раскладе, создатель нового языка программирования не берет свои идеи случайным образом из воздуха. Они все равно базируются на основе его предыдущего опыта, одержимости новой концепцией и прочими начальными установками и ограничениями.

Сразу признаюсь, что не могу однозначно перечислить минимальный набор базовых операторов и конструкций, который будет достаточен для *современного* языка программирования. Более того, я не уверен, что такой набор вообще возможен, ведь многие конструкции можно представить с помощью других, более низкого уровня (например, условного/безусловного перехода).  Я помню про машину Тьюринга, но меня интересуют реальные языки программирования, а не машинные инструкции у абстрактного исполнителя.

Поэтому, в качестве базовых кирпичиков языков программирования можно смело принять те возможности, которые были придуманы и реализованы разработчиками мейнстримовых языков. И начать наверно лучше с критики отдельных и всем хорошо известных фундаментальных концепций. И нет, это не оператор goto! 

### Странные инкремент и декремент (++ и --)

По моему мнению самые не однозначные операторы, это операторы для инкремента и декремента, т.е. арифметическое увеличение или уменьшение на единицу значения переменной. Они вносят серьезную путаницу в строгую  грамматику языка, которая, по моему мнению, просто обязана быть максимально прозрачной и *однозначной*.

Основная проблема у этих операторов заключается в том, что являясь арифметическими операторами, они *модифицируют* значение переменной, тогда как все остальные арифметические операторы оперируют *копиями* значений без изменения самой переменной непосредственно.

Мне могут возразить, что операторы +=, -=, \*= или \= тоже меняют значение переменной, но хочу заметить, что это только упрощенная запись комбинации двух операторов, один из которых как раз и предназначен для присваивания нового значения переменной, поэтому возражения не принимаются. :-)

А если еще вспомнить, что операторы инкремента и декремента могут быть префиксными и постфиксными, то в комбинациях с адресной арифметикой (\*val++ или не дай бог какой нибудь ++\*val++), взрыв мозга с возможными ошибками просто гарантирован.


### Мало операторов присвоения значения
Да, вы читаете все правильно! Я действительно критикую оператор присвоения значения из одного значка равно "**=**", так как считаю, что он является не совсем полноценным. Но в отличие от инкремента и декремента, без которых лексика языка может легко обойтись, без оператора присвоения обойтись никак не получится!

Но моя критика направлена не на сам оператор, а на его незавершенность и создание дополнительной путаницы в некоторых языках программирования. Например, в том же самом Python невозможно понять, происходит ли создание переменной (т.е. первое использование переменной) или же это присвоение значения переменной, которая уже существует (или программист допустил опечатку в имени переменной). 

Если вспомнить королевское "критикуешь, предлагай", то мне кажется, что было бы правильным сделать два разных оператора: оператор *присвоения значения* и оператор *создания переменной* (в С/С++ логика создания переменной выполняется за счет указания типа переменой при ее первом использовании). 

Другими словами, вместо одного оператора "создания и/или присвоения значения" лучше использовать два или даже три оператора: создания новой переменной (**::=**), только присвоения значения уже существующей переменной (**=**) и создания/присвоения не зависимо от наличия переменной (**:=**) - т.е. аналога текущего оператора **=**.

И в этом случае компилятор уже на уровне исходного синтаксиса мог бы контролировать создание или повторное использование ранее созданной перемененной согласно намерениям программиста.

А еще, я бы добавил оператор "обмен значений", какой нибудь **:=:**. По сути, это аналог std::swap() в С++, только на уровне синтаксиса языка.


### Всегда лишний тип данных

Во всех массовых языках программирования как правило присутствуют числа с разной разрядностью. Эта вынужденная необходимость, так как разрядность вычислений определяется аппаратным уровнем и разработчики языков не могут это не учитывать.

Другое дело, булевый (логический) тип данных. В описании одного языка я встретил даже вот такое:
```
Bool        1 Byte truth value
(Bool16)    2 Byte truth value
(Bool32)    4 Byte truth value
(Bool64)    8 Byte truth value
```  
А когда копаешь чуть глубже, все скатывается к одному единственному биту, которым можно представить два противоположных состояния ДА/НЕТ, true/false, 1/0...

Но позвольте, если это 1 или 0, то почему бы сразу не определить, что логический тип, это число с одним разрядом? (передаю пламенный привет LLVM!).

Ведь нет работы хуже, чем бессмысленная работа по преобразованию чисел в логические значения и наоборот:

> В Java существуют довольно строгие ограничения по отношению к типу boolean: значения типа boolean нельзя преобразовать ни в какой другой тип данных, и наоборот. В частности, boolean не является целым типом, а целые значения нельзя применять вместо булевых.

А еще, в некоторых языках программирования, которые поддерживают значение "Пусто/None", булевый тип данных может вообще превращаться в трибул, например в случае параметров функций по умолчанию, когда у булевого аргумента добавляется состояние "не установлен". Но с точки зрения использования не инициализированных переменных, это хотя бы понятно и логически объяснимо.

### Нулевой указатель
Во всех массовых языках программирования так или иначе присутствует тип данных под названием *ссылка*. А в некоторых языках типы ссылок могут быть сразу нескольких видов. 

Однако, наличие ссылочных типов данных добавляет сразу несколько неопределенностей, таких как управление памятью и разделяемыми ресурсами (подробнее в статье [Управление памятью и разделяемыми ресурсами без ошибок](https://habr.com/ru/articles/784184/)). Кроме этого, при наличии адресной арифметики (явной или не явной), сразу становится необходимым использование специального зарезервированного значения под названием "нулевой указатель", **NULL**, **nil**, **nullptr** и т.д. в зависимости от языка.

Наличие подобного значения принуждает разработчиков языков идти на значительное усложнение синтаксиса и логики работы с указателями за счет контроля явной/не явной возможности хранить нулевой указатель в ссылочной переменной.

Но в том случае, если компилятор языка будет сам управлять и контролировать ссылочные типы данных и разделяемые ресурсы, то само понятие как "нулевой указатель" становится не нужным и будет скрыто от программиста в деталях реализации.

### Результат выполнения последней операции

Бывают ситуации, когда не хватает системной переменной со значением результата выполнения последней операции. Какой нибудь аналог  `$?` в bash скриптах, но на уровне исходного кода Python или C/C++. 

Но я имею ввиду не конкретную физическую переменную, а некий обобщенный идентификатор с результатом выполнения последней операции. Псевдо-переменную, которой управляет компилятор языка. Другими словами, чтобы тип этой псевдо-переменной менялся в зависимости от того, какая операция была последней.

Это могло бы упростить решение часто возникающих задач, например, получить последнее значение после выхода из цикла.

Или с помощью такой псевдо-переменной можно значительно упростить синтаксис обработки исключений, где перехват реализуется на основе типов. Но одновременно с типом перехватываемого исключения приходится определять и переменную, даже если она в дальнейшем никак не используется.

###  Чистые функции

Еще, мне бы иногда хотелось иметь возможность создавать чистые функции на С/С++ или Python, чтобы сам компилятор контролировал запрет на обращение к глобальным переменным или не чистым функциям на уровне синтаксиса языка, и это бы проверялось во время компиляции.

###  Пустое имя переменной

И напоследок хочется сказать, что в С++ очень сильно не хватало пустой переменой "**_**" (как в Python). Но в последних предложениях стандарта ее кажется завезли, поэтому начиная с C++26 будет нам счастье :-)


### Итого

При написании данной статьи я постарался абстрагироваться и подойти не предвзято к своему более чем тридцатилетнему опыту разработки, но не уверен, что это у меня получилось, поэтому буду рад любым замечаниям и возражениям в комментариях.

Если вам не сложно, [напишите в комментариях](https://habr.com/ru/articles/785856/), какие возможности в современных языках программирования по вашему мнению больше мешают, чем помогают, или наоборот, каких  операторов/синтаксических конструкций вам не хватает. 

Всегда интересно узнать, что же ты пропустил, забыл или не учел.
