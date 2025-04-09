---
title: Операторы и управляющие конструкции
weight: 3
simple_list: false
---

Слева показан синтаксис основных операторов при использовании [DSL макросов](/ru/docs/syntax/dsl/), 
а справа аналогичные операторы без использования ключевых слов.


[Условный оператор](/ru/docs/ops/if/)
<table>
<tr>
<td>

```python
    @if( condition ) {
        code
    } @elif( call(value) ) {
        code
    } @else {
        code
    };
```
</td>
<td>

```python
    [ condition ] --> {
        code
    } , [ call(value) ] --> {
        code
    } , [...] --> {
        code
    };
```

</td>
</tr>
</table>


[Операторы циклов](/ru/docs/ops/while/)
<table>
<tr>
<td>

```python
    @while( condition ) {
        code
    };
```
```python
    @while( condition ) {
        code
    } @else {
        code
    };
```
```python
    @do {
        code
    } @while( condition );
```
</td>
<td>

```python
    [ condition ] <-> {
        code
    };
```
```python
    [ condition ] <-> {
        code
    } , [...] --> {
        code
    };
```
```python
    {
        code
    } <-> [ condition ];
```

</td>
</tr>
</table>


[Операторы оценки выражения](/ru/docs/ops/match/)
<table>
<tr>
<td>

```python
    @match( $var ) ==> {
        @case( 1 ) { code };
        @case( 1, 2 ) { code };
        @default { code default };
    };
```
</td>
<td>

```python
    [ $var ] ==> {
        [1] --> { code };
        [1, 2] --> { code };
        [...] --> { code default };
    };
```
</td>
</tr>
</table>



[Оператор менеджера контекста](/ru/docs/ops/with/)
<table>
<tr>
<td>

```python
    @with( val1 = *ref1, val2 = *ref2 ){
        code
    };
```
```python
    @with( val = *ref ){
        code
    } @else {
        code
    };
```
</td>
<td>

```python
    **( val1 = *ref1, val2 = *ref2 ){
        code
    };
```
```python
    **( val = *ref ){
        code
    } , [...] --> {
        code
    };
```

</td>
</tr>
</table>



### Описание синтаксиса подробно: