---
slug: lang-final
title: Какая «идеальная» цель развития у языков программирования?
date: 2021-04-07
---

![КДПВ](lang-final.jpeg)

С постоянной периодичностью появляется информация о выходе новой версии того или иного языка программирования. 
И с каждой новой версией расширяются его возможности, добавляются новые синтаксические конструкции или иные улучшения.

И это очень сильно напоминает развитие технологий, как и в любой другой области техники. 
Когда с очередным этапом совершенствуются создаваемые творения. 
Быстрее, выше, сильнее … и одновременно значительно сложнее.

Об этой проблеме меня заставила задуматься первоапрельская статья [«Доказательное программирование»](https://habr.com/ru/post/550270/).

Понятно, что дата публикации статьи говорит сама за себя. 
Тем не менее, новые стандарты С++, постоянно выходящие спецификации Java или новый синтаксис у PHP 8, 
невольно заставляют задуматься, а в нужную ли сторону идет развитие языков программирования? 
Ведь большинство нововведений добавляют сложность в основной рабочий инструмент и решая одни проблемы, неявно добавляя множество других.

А что должно быть в конце прогресса у развития такой дисциплины как программирование? 
Или хотя бы у одного конкретного языка? Ради достижения какой конечной «идеальной» цели разрабатываются новые стандарты языков программирования?


Если фантазировать об идеальной конечной цели развития, например транспорта, 
то это будет мгновенное перемещение на любое расстояние с произвольной полезной нагрузкой и нулевым потреблением энергии.

Или, например, какая может быть идеальная цель у медицины? ~~Бедные не болели, а богатые не выздоравливали,~~ вероятно, лечение любых болезней и биологическое бессмертие.

Конечно, «идеальная» цель это очень упрощенное понятие. Фактически, «идеальная» это синоним «недостижимая», 
т.к. она всегда будет упираться необходимость соблюдать компромисс между различными взаимно-исключающими граничными условиями.

Но сравнивать разработку средств программирования с процессом развития в других технических дисциплинах напрямую нельзя. 
Ведь при создание конечного продукта в любой технической области, все сложные производственные операции, 
которые требуют непосредственного участия человека, практически всегда можно разделить на отдельные, более простые части или этапы.  

Это делается, в том числе и для того, чтобы сложность одной выполняемой операции не была запредельной для исполнителя. 
Но как это можно сделать при разработке программного обеспечения?

В данном случае, я имею ввиду конечную ограниченность возможностей одного конкретного человека в противовес возможностям разделения технологических процессов на отдельные этапы, 
каждый из которых может выполняться совершенно разными людьми (как пример, это обычный конвеер с его разделением труда на элементарные операции 
или узкая специализация врачей-специалистов в одной конкретной области).

Ведь даже сложно себе представить фантастическую организацию труда программистов в виде конвеера: 
- Первый разработчик пишет только интерфейсы функций и их вызовы, после чего передает код второму сотруднику. 
- Второй пишет в тексте программы только условия проверки и безусловные переходы и передает текст третьему. 
- Третий отвечает за написание циклов и общее форматирование кода и т.д. 
и результате ожидаемо получится полный бред.

Из-за этого индустрия разработки ПО вынуждена идти по экстенсивному пути развития (т.е. за счет увеличения используемых в производстве ресурсов). 
Современные промышленные языки программирования имеют очень богатые возможности по разделению кода приложения на отдельные функции/модули/компоненты, 
что позволяет вести разработку сложного программного продукта сразу множеству сотрудников одновременно.

Но и такое развитие имеет естественное ограничение. И этим ограничением является сам человек, 
так как каждый разработчик должен знать и уметь пользоваться своим рабочим инструментом, т.е. языком программирования.

Если взять приведенную выше аналогию с конвеером, то в ней каждый рабочий должен был бы досконально знать б**о**льшую часть используемых станков 
и инструментов на всем заводе, независимо от того, какую **одну** конкретную операцию он выполняет на своем рабочем месте.

Ведь парадокс развития языков программирования заключается в том, что добавляя новые возможности и синтаксические конструкции, 
мы усложняем рабочий инструмент, предназначенный как для совместного, так и для индивидуального использования! 

И получается, что одновременно с процессом постоянного наращивания возможностей средств разработки идет 
и обратный процесс — увеличение сложности разработки кода отдельно взятым разработчиком. 
Фактически, это и есть то самое взаимоисключающее непреодолимое противоречие.

Может быть именно поэтому невозможно найти «серебряную пулю», которая бы повышала производительность труда одного программиста? 
Ведь внимание и возможности человека не безграничны. 
И любые новшества и улучшения рабочего инструмента программиста вынужденно толкают всю IT индустрию на экстенсивный путь развития.

Возможно следует разрабатывать принципиально новые подходы, а не гнаться за синтаксическим сахаром в языках программирования из прошлого века? 
Или и вовсе не париться, продолжать пользоваться тем, что есть, а теория дедушки Дарвина все сама расставит по своим местам?

[Первоначальная публикация](https://habr.com/ru/companies/timeweb/articles/551224/)

