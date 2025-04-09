---
title: Многопоточное и асинхронное программирование
description: 
    # Операторы возврата из функций, прерывания выполнения циклов, создание инсключений при возникновении ошибок.
weight: 16
---

Для многопоточноо и асинхронного выполнения кода используются отдельные типы данных,
которые принимают одним из аргументов обычно [анонимную функцию](/ru/docs/types/funcs/#anonymous) 
или [короутину](/ru/docs/types/funcs/#coro) и выполняют её параллельно в отдельном потоке или асинхронно.

Класс :Thread() применяется для создания отдельного потока
и класс :Task() для создания короутины для асинхронного выполнения кода, 
который может выполняться как в рамках своего текущего потока, 
так и в отдельном потоке или даже пуле потоков.

Тело потока или задачи (функция или короутина) передается в качестве аргумента в конструктор соответствующего класса :Thread() или :Task(),
или же базовый класс может быть расширен. Использование базового класса (вместо вызова низкоуровневых функций),
позволяет более просто реализовать локальные данных для каждого потока (вместо thread_local перемнных)
и для уменьшения возможных потенциальных ошибок при обработке исключений в каждом отдельном потоке или корутине,
так как не обработанные исключения перехватываются родительским классом, чтобы не произошло краха всего приложения.


### Отдельный поток приложения {#thread}

Любую функцию можно выполнить как отдельный поток приложения, выполнение которого будет происходить не зависимо от основонго потока. 
Переключения потоков реализуется средствами ОС и происходит неявно в произвольные моменты времени.

```cpp
    func() ::= { slepp(1) };
    pure() ::- { slepp(1) };

    *thread_func ::= :Thread( &func );
    *thread_pure ::= :Thread( &pure );
    *thread_anon ::= :Thread( _() := { sleep(1) } );

    thread_func.join();
    thread_pure.join();
    thread_anon.join();
```




### Пул потоков приложения {#threadpool}
Создание и удалние потоков операционной системы, это относительно тяжелые (медленные) операции.
Чтобы ускорить выполнение большого количества маленьких задач в параллельных потоках приложения
можно использовать пул потоков. Данный класс заранее создает заданное количество потоков ОС
и динамически назначает один из свободных для выполнения новой задачи по мере их добавления.

```python

    print('Main thread %d',  :Thread::get_id()); 

    * pool ::= :ThreadPool(4); # Maximum 4 threads

    * i :Integer = 0;
    @while(i < 5) {
        # Enqueue tasks for execution
        pool.enqueue( 
            _(task: Integer) := { 
                print('Task %d is running on thread %d',  $task, :Thread::get_id()); 
                sleep(1);
            } 
        );
        i += 1;
    };
    pool.join();
```

```python
    Main thread 140178994147480
    Task 0 is running on thread 140178994148928
    Task 1 is running on thread 140178985756224
    Task 2 is running on thread 140179010934336
    Task 3 is running on thread 140179002541632
    Task 4 is running on thread 140178994148928
```



### Асинхронное выполнение {#async}

[Лямбда функцию](/ru/docs/types/funcs/#lambda) или [сопрограмму (короутину)](/ru/docs/types/funcs/#coro),
можно выполнить как асинхронную задачу, относительно текущего потока выполнения и не зависимо от него. 

Но в отличии от потоков операционной системы, переключение между асинхронными задачами происходит явно,
методами языка программирования при вызове Awaitable функций,
что не требует ресурсов процессора и операционной системы на переключение контекста.

Awaitable функции это обычные функции которые возвращают Awaitable объект,
и в зависимости от его значения выполпнение корутины продолжается, если данные были возвращаны, 
либо выполнение корутины приостанавливается до получения новых данных,
а поток управления получает другая корутина в этом же потоке, которая выполняется одновремненно с текущей.

!!!!!!!!!!!!!    
Недостатки корутин в C++ https://habr.com/ru/companies/ruvds/articles/755246/   
!!!!!!!!!!!!!    

Так как асинхронное выполнение корутин происходит в рамках одного потока операционной системы, что автоматически 
исключает какие либо [гонки между потоками](https://en.wikipedia.org/wiki/Race_condition#In_software), 
однако не исключает [запуска асинхронных задач в пуле потоков](#asynctask), 
что может привести к неконсистентности данных при реализации логики работы приложения.

Поэтому доступ к данным внтури корутин крайне желательно делать с использованием объектов [межпотоковой синхронизации](/ru/docs/syntax/memory/#sync).


```python
    
    async_read(stream): <:String,>:Awaitable ::= { // <:AwaitState, :String,>: Awaitable
        # Coroutine as an asynchronous task
        @if(poll(stream)){
            # Read and return data
            @return <read(stream),>:Awaitable; # Return data
            //@return :Awaitable:Awaitable(true, read(stream) );
        } @else {
            # No data to read 
            # Switch to another async task
            @return <,>:Awaitable(); # suspend
        }
    };

    @@ co_await @@ ::= @@ ** @@;
    async_task(stream) ::= []() { // Coroutine
        str ::= co_await async_read(stream); # ** async_read(stream);
        print(str);
    };

    @while( @true ){
        :Async( &async_func( Open( Input ) ) );
    }
```


### Пул асинхронных задач {#asynctask}
Асинхронные задачи выполянются в одном и том же потоке приложения, но при наличии нескольких физических ядер у процессора, 
асинхронные задачи целесообразно распредять сразу по нескольким потокам, выполняющимся на разных физическия ядрах CPU. 

Для этих целей служат два класса :AsyncTask - запуск асинхронных задач в отдельном потоке операционной системы
и класс :AsyncPoll - который заранее создает заданное количество потоков ОС
и динамически назначает один из них для выполнения новой асинхронной задачи по мере их добавления.


```python

    print('Main thread %d',  :Thread::get_id()); 

    * task ::= :AsyncTask(4); # Maximum 4 async task
    * i: Integer = 0;
    @while(i < 5) {
        # Enqueue async tasks for execution
        task.enqueue( 
            [](task: Integer) { 
                print('AsyncTask %d is running on thread %d',  $task, :Thread::get_id()); 

                * counter:Int64 ::= 0;
                * start_time:Int64 ::= time::microseconds();

                @while(time::microseconds() - start_time < 1_000_000){
                    usleep(1);
                    @co_yield; # -+;
                    counter += 1;
                }

                print('AsyncTask %d in thread %d done and was called %d times!', $task, :Thread::get_id(), $counter); 
                # @co_return; # ++;
            } 
        );
        i += 1;
    };
    task.join();
```

```python
    Main thread 140178994147480
    AsyncTask 0 is running on thread 140178994148924
    AsyncTask 1 is running on thread 140178994148924
    AsyncTask 2 is running on thread 140178994148924
    AsyncTask 3 is running on thread 140178994148924
    AsyncTask 4 is running on thread 140178994148924
```


```python
    * pool ::= :AsyncPool(2, 5); # Maximum 2 threads and 5 task in each

    @while(@true) {
        pool.enqueue( &async_func( Open( Input ) )
    };
    pool.join();
```

