---
title: Опции и настройки компилятора
# description:  Правила синтаксиса языка
# date: 2017-01-05
weight: 110
---


## Прагмы


### Предварительное объевления объектов
@\_\_PRAGMA_DECLARE\_\_
```
    @__PRAGMA_DECLARE__( func(arg) );

```

### Импорт нативной функции / переменной 

@\_\_PRAGMA_NATIVE\_\_
```
    // import native int printf(char *format, ...)
    @__PRAGMA_NATIVE__( printf(format:FmtChar, ...):Int32 );

```

### Управление диагностикой компилятора
@\_\_PRAGMA_DIAG\_\_ -         
```
        @__PRAGMA_DIAG__(push)
        @__PRAGMA_DIAG__(ignored, "-Wundef")
        @__PRAGMA_DIAG__(warning, "-Wformat" , "-Wundef", "-Wuninitialized")
        @__PRAGMA_DIAG__(error, "-Wuninitialized")
        @__PRAGMA_DIAG__(pop)
```

@\_\_PRAGMA_MESSAGE\_\_
```
    #pragma message "Compiling " __FILE__ "..."
    @__PRAGMA_MESSAGE__("Compiling ", __FILE__, "...")


    #define DO_PRAGMA(x) _Pragma (#x)
    #define TODO(x) DO_PRAGMA(message ("TODO - " #x))

    @@TODO( ... )@@ := @__PRAGMA_MESSAGE__("TODO - ", @#...)

    @TODO(Remember to fix this)  # note: TODO - Remember to fix this

```

@\_\_PRAGMA_WARNING\_\_
@\_\_PRAGMA_ERROR\_\_

- @\_\_PRAGMA_LOCATION\_\_
```
        // #line 303 "location.hh"
        // Prototype - @__PRAGMA_LOCATION__( pop )
        // Prototype - @__PRAGMA_LOCATION__( push ) or @__PRAGMA_LOCATION__( push, 'filename') or @__PRAGMA_LOCATION__( push, 'filename', line)
        // Prototype - @__PRAGMA_LOCATION__( line ) or @__PRAGMA_LOCATION__( line, 'filename')

```


## Управление макросами
@\_\_PRAGMA_EXPECTED\_\_ - Следующий символ должен быть одним из списка
```
    @__PRAGMA_EXPECTED__( @\ {, @\ {*, @\ {+ ) 
        { }

    @__PRAGMA_EXPECTED__( @\ {, @\ {*, @\ {+) 
        {+ +}

```

@\_\_PRAGMA_UNEXPECTED\_\_ - Запретить использование перечисленных символов
```
    @__PRAGMA_UNEXPECTED__( @\{, @\{*, @\{+) 
        {- -}
    @__PRAGMA_UNEXPECTED__( @\{*, @\{+)
        {   }

```

@\_\_PRAGMA_NO_MACRO\_\_ - Следующий символ не макрос


### Аннотации

@\_\_ANNOTATION_SET\_\_
```
    @__ANNOTATION_SET__(name)
    @__ANNOTATION_SET__(name, "value")
    @__ANNOTATION_SET__(name, 1)
```

@\_\_ANNOTATION_IIF\_\_


### Не используется (заготовки на будущее)

 @\_\_PRAGMA_IGNORE\_\_
```
        static const char * ignore_space = "space";
        static const char * ignore_indent = "indent";
        static const char * ignore_comment = "comment";
        static const char * ignore_crlf = "crlf";
```

- @\_\_PRAGMA_INDENT_BLOCK\_\_

- @\_\_PRAGMA_MACRO\_\_ - 


- @\_\_PRAGMA_MACRO_COND\_\_

- @\_\_PRAGMA_FINALIZE\_\_
  