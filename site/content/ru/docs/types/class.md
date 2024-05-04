---
title: Классы
# linkTitle: Docs
# menu: {main: {weight: 20}}
weight: 35
#categories: [типыExamples, Placeholders]
tags: [типы данных, ООП, коллекции]
---


Тип данных *:Class* аналогичен словарю, но все его поля обязаны иметь имена (хотя доступ к свойствам класса по индексу так же возможен).
При создании экземпляра класса, создается новый тип, для которого копируются свойства и методы всех родителей. 

Создание нового сложного типа (класса), происходит согласно [правилам синтаксиса](/ru/docs/ops/create/) как создание нового типа 
как функции в пользовательском [пространстве имен](/ru/docs/syntax/namespace/). 
Имя типа является именем нового класса, а его вызов как функции будет вызовом конструктора.
Области видимости [тут](/ru/docs/arch/visibility.)

Синтаксис создание класса выглядит следующим образом:
```python
# Новый тип (класс) :NewClass
ns::NewClass() := :Class() { # Родительские класс или несколько классов через запятую

    # Конструктор класса - весь блок кода с определением полей и методов

    @::static := 1; # Поле класса (у всех объектов одно значение)
    static := 1; # Поле класса (у всех объектов одно значение)
    .field := 1; # Поле объекта
    func() := {}; # Метод класса всегда статический
    .method() := {}; # Метод объекта (у каждого объекта свой)

    ~NewClass() ::= { # Деструктор класса

    };
};
obj := ns::NewClass(); # Экземпляр класса
```

Так как *NewLang* реализует полный набор вариантов проверок при создании объектов, 
то переопределения наследуемых функций не требует никаких ключевых слов:
```python
NewClass2() := NewClass { # Новый класс на базе существующего
    .field ::= 2;    # Будет ошибка, т.к. поле field уже есть в базовом классе
    method() = {};  # Аналог override, т.к. method должен существовать в базовом классе
};
```

```python

@@ self_required() @@ ::= @if( @not(@self) ) { @throw :ErrorRuntime( 'Fail function call without object!' ); };

:File(filename: String, mode:String) ::= Class(){
    :Handler ::= :Pointer;
    _fopen(file:StrChar, mode:StrChar):Handler ::= %fopen64 ...;
    _fclose(hfile:Handler):None ::= %fclose ...;
    _ftmpfile():Handler ::= %ftmpfile ...;
    
    ._haldler:Handler ::= 0; # FILE handler

    ~File() ::= { # Destructor
        @self_required();
        [ $0._haldler ] --> {
            _fclose($0._haldler);
            $0._haldler = 0;
        };
    };

    # Constructor body
    [$filename] --> {
        ._haldler = _fopen($filename, $mode);
    },[...] ={
        ._haldler = _ftmpfile();
    };
    [ ._haldler == -1 ] --> {
        @throw( "Fail open file $name"(name=$filename) );
    }

};

File::remove(filename:String):Int32; ::= %remove ...;
File::frename(old:String, new:String):Int32 ::= %rename...;
File::_fflush(stream:FileHandler):Int32 ::= fflush...;
File::flush(hfile:Handler = -1):Int32 ::= {
    @self_required();
    [ hfile != -1 ] --> {
        @return _fflush(stream);
    }, [ $0 ] {
        @return _fflush($0._haldler);
    },[...] ={
        @throw( "Fail flush without file handler or object!" );
    };
};

File::_fileno(handler:Handler) ::= fileno ...;
File::fileno():Int32 ::= {
    [ $0 ] {
        @return _fileno($0._haldler);
    },[...] ={
        @throw( "Fail fileno without object!" );
    };
};

File::_fputc(c:Int32, stream:Handler):Int32 ::= fputc ...;
File::_fputs(string:String, stream:Handler):Int32 ::= fputs...;

File::_fread(c:Int32, stream:Handler):Int32 ::= fputc ...;
File::_fwrite(c:Int32, stream:Handler):Int32 ::= fputc ...;
size_t fwrite(const void *buf, size_t size, size_t count, FILE *stream)

File::_fread(buf:Pointer, size:Integer, count:Integer, stream:Handler):Integer ::= fread ...;
File::read(&buf:Integer, size:Integer = -1):Integer ::= {
    [ @not($0) ] {
        @throw( "Fail read without object!" );
    };

    [ size == -1 ] {
        size = tell();
        buf = Tensor[size](buf);
    };
    
    @return read(&buf, buf.size(), 1, $0._handler);
};


int flushall(void)
int fcloseall(void)


File::filelegth() ::= {
    $size: Int64 ::= _;
    @if( @.embed-enabled ){%
        size_t save = ftell($0._handler); // get current file pointer
        fseek($0._handler, 0, SEEK_END); // seek to end of file
        $size = ftell($0._handler); // get current file pointer
        fseek($0._handler, save, SEEK_SET); // seek back to beginning of file
    %} @else {
        save = _ftell($0._handler);
        fseek(f, 0, SEEK_END); // seek to end of file
        $size = ftell($0._handler); // get current file pointer
        fseek(f, save, SEEK_SET); // seek back to beginning of file
    }
    @return $size;
}



```

:FileHandler ::= :Pointer;

fopen(filename:String, modes:String):FileHandler ::= :Pointer("fopen(filename:StrChar, modes:StrChar):FileHandler");;
fopen64(filename:String, modes:String):FileHandler ::= :Pointer("fopen(filename:StrChar, modes:StrChar):FileHandler");;

freopen(filename:String, modes:String):FileHandler ::= :Pointer("freopen(filename:StrChar, modes:StrChar, stream:FileHandler):FileHandler");;
fclose(stream:FileHandler):Int32 ::= :Pointer("fclose(stream:FileHandler):Int32");;
fflush(stream:FileHandler):Int32 ::= :Pointer("fflush(stream:FileHandler):Int32");;
fremove(filename:String):Int32 ::= :Pointer("remove(filename:StrChar):Int32");;
frename(old:String, new:String):Int32 ::= :Pointer("rename(old:StrChar, new:StrChar):Int32");;
ftmpfile():FileHandler ::= :Pointer("tmpfile():FileHandler");;

fprintf(stream:FileHandler, format:FmtChar, ...):Int32 ::= :Pointer("fprintf(stream:FileHandler, format:FmtChar, ...):Int32");;
fscanf(stream:FileHandler, format:FmtChar, ...):Int32 ::= :Pointer("fscanf(stream:FileHandler, format:FmtChar, ...):Int32");;       
fgetc(stream:FileHandler):Int32 ::= :Pointer("fgetc(stream:FileHandler):Int32");;
fungetc(c:Int32, stream:FileHandler):Int32 ::= :Pointer("ungetc(c:Int32, stream:FileHandler):Int32");;
fputc(c:Int32, stream:FileHandler):Int32 ::= :Pointer("fputc(c:Int32, stream:FileHandler):Int32");;
fputs(string:String, stream:FileHandler):Int32 ::= :Pointer("fputs(c:StrChar, stream:FileHandler):Int32");;

SEEK ::= :Enum(SET=0, CUR=1, END=2);
fseek(stream:FileHandler, offset:Int64, whence:Int32):Int32 ::= :Pointer("fseek(stream:FileHandler, offset:Int64, whence:Int32):Int32");;





## Импорт нативных классов  <КОСЯКИ!!!!!>
Так же новый класс можно создать на базе нативного класса C++ с определенными ограничениями:
- У нативных классов можно импортировть только *публичные не статические методы* (не виртуальные???)
- Импорт перегруженных методы пока под вопросом?
- Импортировать и использовать поля нельзя из-за их определения как смещения в объекте, а не адресом в памяти, как тело функции.
- Статические поля класса и статические методы использовать нельзя из-за отсутсвия на них ссылки в глобальной таблице имен.
- У виртуальных функций есть адрес в глобальной таблице имен, но как их вызывать, пока не понятно.
- Импортируемые методы могут удаляться из приложения линкером, если на них нет ссылки (если они не используются).
- Используется только один нативный констурктор, который указывается при определении функции контруктора класса перед фигурными скобками.
 
При импорте нативного класса, он указывается в качестве родителя с указанием полного пространства имен.
Вызов нативного конструктора происходит автоматически перед выполенением конструктора объекта.
Вызов нативнного деструктора происходит автоматически при удалении объекта.

Определение нативного класса С++:
```cpp
namespace ns {
    class NativeClassCPP {
        int day;
        int month;
        int year;
    public:

        NativeClassCPP(): NativeClassCPP(0, 0, 0) {
        }

        NativeClassCPP(int day, int month, int year ) {
            setDate(day, month, year);
        }

        std::string message()  {
            return std::string("\nNativeClass C++ message\n");
        }

        void setDate(int date_day, int date_month, int date_year) {
            day   = date_day;
            month = date_month;
            year  = date_year;
        }

        void getDate() {
            cout << "date: " << day << "." << month << "." << year << endl;
        }
    };
};
```

Импорт нативного класса:
```python
:NativeClass := %ns::NativeClassCPP {

    # Конструктор класса
    NativeClass(day:Int32=0, month:Int32=0, year:Int32=0) ::= %(day, month, year) { 
    # Будет вызываться  NativeClassCPP(int day, int month, int year )

    }

    getDate():None ::= %getDate ...;
    Message():StdString  ::= %message ...;
    setDate(day:Int32, month:Int32, int year:Int32):None ::= %setDate...;

    ~NativeClass() ::= {
        # Деструктор для нативного объекта вызывается автоматически
    };
};
obj := :NativeClass(); # Экземпляр класса
obj.setDate(26, 03, 2024);
obj.Message();

```




## Интерфейсы, именование методов классов и пространства имен
Для создания уникальных идентификаторов на основе имен методов классов, *NewLang* использует подход, похожий на применяемый в языке Python. 
При создании метода класса, создается глобальная функция с именем класса и именем метода, объединенные через разделитель области имен. 
Например, в классе `NewClass2` при создании метода `method` будет создана функция с именем `::NewClass2::method`.

Такая схема наименований методов полностью соответствует именованию функций в пространствах имен, 
и тем самым позволяет определять классы c чистыми виртуальными функциями (методами без реализации), 
а в последствии определять их в пространстве имен или указав полное имя в явном виде.

```python
ns:: {
    NewClass3 := NewClass() { # Новый класс на базе существующего
        virtual() := _; # Виртуальный метод
    };
};

obj := ns::NewClass3(); # объект создать нельзя, будет ошибка

# Определить функцию для виртуального метода
ns::NewClass3::virtual() := {}; 

obj := ns::NewClass3(); # ОК
```

## Конструкторы, деструкторы и финализаторы у объектов
```python
ns::NewClass() :=  Class() {    # Новый класс на базе существующего

    # Блок кода функции - конструктор объекта

    __NewClass__() = {...}; # Метод с системным именем типа - деструктор объекта (вызывается сразу после удаления)

    _NewClass() := {...}; # Скрытый метод с именем типа - защищенный конструктор объекта ?????????????????
    __NewClass() := {...}; # Скрытый метод с именем типа - приватный конструктор объекта ?????????????????

    :NewClass(type) ::= {...}:NewClass; # Функция для приведения типа объекта $type к типу :NewClass
    ~NewClass() = {...}; # Скрытый метод - финализатор (вызывается перед освобождением памяти)

    __equals__(obj):Bool = { __compare__($obj) == 0 };
    __compare__(obj):Int8 =  { :: __compare__(@this, $obj) };


    _() := {...}; # ?????????????????????????????????????
    
};
```

extension methods !

> Пишу на С++ последние лет 20, до этого ещё Delphi лет 10. С годами полностью отошёл от ООП в сторону data-driven design. 
> Классы — практически структуры, из методов как правило только декоративные геттеры. 
> Всё остальное — это просто функции с понятными названиями, сигнатурами и операндами-объектами таких вот классов-носителей данных и состояния. 
> Получается, очень легко дышится и чистенько — состояния изолированы в структуры, логика изолирована в функции. 
> Конечно, в C++ всё довольно печально с ООП как таковым, поскольку нет механизма **extension methods — это когда вы собираете методы в класс из разных единиц трансляции**
> из-за этого, обычные функции C++ значительно удобнее методов класса




> Основная мысль, которая почти всегда теряется при обсуждении ООП, это то, зачем он нужен и в чем его суть. Все сводится к инструментам, их правильном и неправильном использовании, практичных архитектурах и оверинженеринге.
> 
> Мне кажется самая главная мысль очень проста. Сложная программа это сложное состояние.
> Проблема в том, что сложно следить за всей возможной суперпозицией всех деталей состояния.
> Небольшое состояние гораздо проще валидировать и постоянно поддерживать целостным.
> Если ты собираешь сложное состояние из простых целостных состояний, то его тоже проще поддерживать целостным (потому что нужно следить только за высокоуровневым состоянием, но не за всеми деталями)
> 
> Соответственно ООП - это способ описания программы как набора иерархии изолированных состояний, где операции по изменению состояний максимально приближены к состоянию.
> 
> 
> Точно так же как сложную функцию можно представить как последовательность простых, так и сложное состояние можно представить комбинацией простых состояний.
> Изоляция и контроль состояний и есть основная идея ООП.
> 
> ФП подходит с другой стороны - максимально старается избежать состояния и работать только с текущим контекстом. Подход не лишен логики, но любая программа - это прежде всего состояние.
> 
> Все остальное - лишь инструменты и особенности реализации. ООП может быть реализовано почти на любом языке самыми разными инструментами и не обязательно требует интерфейсов, классов, сообщений, инкапсуляции и т.д. Но разумеется современные ООП языки хорошо адаптированы для такого способа описания. В конечном итоге если подсказки или области видимости действительно не дают тебе менять состояние объекта - это и есть прямая польза на этапе понимания и доработки твоего кода. Тебе просто не нужно думать и знать о состоянии объекта, когда ты с ним не работаешь.




> Главное в ООП - это то, что есть данные, и есть методы которые напрямую работают с этими данными. Это то, что можно засунуть в один объект. Другой объект, который хочет получить данные из первого объекта, обязан пользоваться публичными методами.
> 
> Это упрощает поддержку обратной совместимости, упрощает изоляцию объектов, упрощает версионирование в случае многократного использования одними объектами других объектов.
> 
> И опять таки, суть не в том, что это чем-то напоминает рест-апи или библиотеки, а в том, что это парадигма программирования, которая упрощает организацию разработки сложных проектов, в которых задействовано много разработчиков.
> 
> Все остальные штуки - наследование, интерфейсы и все другое - это не суть ООП, а различные варианты решения или оптимизации различных кейсов