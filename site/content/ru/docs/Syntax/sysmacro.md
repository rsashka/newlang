---
title: Системные макросы
# description:  Правила синтаксиса языка
# date: 2017-01-05
weight: 60
---


## DSL

### Особенности ассоциативной памяти
Синтаксис языка *NewLang* основан на строгих правилах без использования ключевых слов, 
и как бы он не выглядел логичным, ассоциация по ключевым словам вспоминается гораздо легче, например **if**,
чем комбинация *минус минус правая угловая скобка* **-->**. 
Из-за этого имеет смысл использовать не "чистый" базовый синтаксис, а более привычный диалект с использованием ключевых слов. 

### Синтаксис на ключевых словах
В файле dsl.nlp находится набор макросов, которые расширяют базовый синтаксис *NewLang*, основанный на правилах,
набором предопределенных ключевых слов как в классических языках программирования. 
А при необходимости, их можно адаптировать или дополнить под собственную предметную область.


## Константы
### Логические
- **true** - 1
- **yes**  - 1
- **false** - 0
- **no** - 0

### Системные имена
- **this** - Текущий объект (**$0**)
- **parent** - Родительский объект (**$$**)
- **last** - Результат выполнения последнего оператора (**$^**)

!!!!!!!!- **args** - Все аргументы функции (**$\***)

!!!!!!!!- **sys** - Системный контекст запущенной программы (**@@**)
!!!!!!!!- **current** - Текущий модуль (**@$**)
!!!!!!!!- **cmd** - Все аргументы выполняющегося приложения из командной строки (**@\***)

!!!!!!!!!!1### Типовые функции и проверки 
!!!!!!!!!!- **run('filename')** - Выполнить указанный файл (**@@('filename')**)
!!!!!!!!!!- **ifmain** - Проверка, если текущий модуль основной (**[@$.$0] -->** { ... })
!!!!!!!!!!- **ifload** - Проверка, если текущий модуль загружается впервые (**[@$.__load__] -->** { ... })

### Операторы
- **match(cond)** - Оператор [оценки выражения](https://newlang.net/ops.html#оценка-выражения)
- **if(...)** - Первый условный оператор (**[\$*]-->** { ... })
- **elif(...)** - Второй и все последующие (**,[\$*]-->** { ... })
- **else** - Оператор *иначе* (**,[_]-->** { ... })

- **while(cond)** - Оператор цикла с предусловием (**[@$cond] <->** { ... } )
- **dowhile(cond)** - Оператор цикла с постусловием ({ ... } **<->[@$cond]**)

- **iter(obj, ... )** - Создание итератора ( **@$obj** ? (@$*) )
- **next(obj, ... )** - Следующий элемент итератора ( **@$obj** ! (\$*) )
- **curr(obj, ... )** - Текущий элемент итератора ( **@$obj** !? (\$*) )
- **first(obj)** - Перейти на первый элемент итератора ( **@$obj** !! )
- **all(obj)** - Получить сразу все элементы итератора ( **@$obj** ??)


### Прерывания
- **return** - Положительное прерывание (**++**)
- **return(...)**- Положительное прерывание с возвратом значения (**++** @$value **++**) 

- **error** - Отрицательное прерывание (**-\-**)
- **error(...)** - Отрицательное прерывание с возвратом значения (**-\-** @$value **-\-**)

- **break+** и **break-** - Выход из цикла:
```
    while(...) {+
        ++ :Break ++   
    +}
    while(...) {-
        -- :Break --
    -}
```
- **continue+** и **continue-** - В начало цикла:
```
    while(...) {+
        ++ :Continue ++   
    +}
    while(...) {-
        -- :Continue --
    -}
```





obj {
    .field = 1; 
    .prop();
};

#Python
# 1) without using with statement
file = open('file_path', 'w')
file.write('hello world !')
file.close()


 
# 2) without using with statement
file = open('file_path', 'w')
try:
    file.write('hello world')
finally:
    file.close()

open('file_path', 'w') {
    .write('hello world !');
    .close();
}

  
Python3

# using with statement
with open('file_path', 'w') as file:
    file.write('hello world !')


# a simple file writer object
 
class MessageWriter(object):
    def __init__(self, file_name):
        self.file_name = file_name
     
    def __enter__(self):
        self.file = open(self.file_name, 'w')
        return self.file
 
    def __exit__(self, *args):
        self.file.close()
 
# using with statement with MessageWriter
 
with MessageWriter('my_file.txt') as xfile:
    xfile.write('hello world')





# a simple file writer object
:MessageWriter {
    def __init__(self, file_name):
        self.file_name = file_name
     
    def __enter__(self):
        self.file = open(self.file_name, 'w')
        return self.file
 
    def __exit__(self, *args):
        self.file.close()
}

# using with statement with MessageWriter
 
MessageWriter('my_file.txt') {
    .write('hello world');
};







from contextlib import contextmanager
 
 
class MessageWriter(object):
    def __init__(self, filename):
        self.file_name = filename
 
    @contextmanager
    def open_file(self):
        try:
            file = open(self.file_name, 'w')
            yield file
        finally:
            file.close()
 
 
# usage
message_writer = MessageWriter('hello.txt')
with message_writer.open_file() as my_file:
    my_file.write('hello world')




:MessageWriter(object) {
    def __init__(self, filename):
        self.file_name = filename
 
    @contextmanager
    def open_file(self):
        try:
            file = open(self.file_name, 'w')
            yield file
        finally:
            file.close()
 
}

# usage
message_writer = MessageWriter('hello.txt')
with message_writer.open_file() as my_file:
    my_file.write('hello world')





# Приложения


## Предопределенные макросы

При работе парсера *NewLang* автоматически формируются несколько зарезервированных макросов, часть из которых соответствуют макросам препроцессора у С/С++.
Данные предопределенные макросы можно использовать как обычные константы. 

- \_\_FILE\_\_ или \_\_FILE_NAME\_\_ - имя и полный путь текущего файла
- \_\_LINE\_\_ или \_\_FILE_LINE\_\_ - содержит номер текущей строки в файле 
- \_\_FILE_TIMESTAMP\_\_ - дату и время последней модификации текущего файла в строковом представлении
- \_\_FILE_MD5\_\_ - md5 хеш текущего файла в строковом виде
- \_\_COUNTER\_\_ - целочисленный счетчик, который увеличивает свое значение при каждом обращении


- \_\_DATE\_\_ - дата начала запуска компилятора (имеет одно и тоже значение для всех обрабатываемых файлов)
- \_\_TIME\_\_ - время начала запуска компилятора (имеет одно и тоже значение для всех обрабатываемых файлов)
- \_\_TIMESTAMP\_\_ - дату и время начала запуска компилятора (например: "Fri 19 Aug 13:32:58 2016") 
- \_\_TIMESTAMP_ISO\_\_ - дату и время начала запуска компилятора в формате ISO (например: "2013-07-06T00:50:06Z")


- \_\_NLC_VER\_\_ - Страшая и младшая версия компилятора NewLang (8 битное число, по 4 бита на цифру)
- \_\_NLC_DATE_BUILD\_\_ - текстовая строка с датой сборки компилятора NewLang (например, "23.06.04 20:51:39")
- \_\_NLC_SOURCE_GIT\_\_ - текстовая строка с идентификатором исходных файлов NewLang git репозитория, использованных при сборке компилятора (например, "v0.3.0-fef8c371")
- \_\_NLC_SOURCE_BUILD\_\_ - дата сборки и git идентификатор исходных файлов NewLang одной текстовой строкой (например, "v0.3.0-fef8c371 23.06.04 20:51:39")
