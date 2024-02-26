---
title: Playground and example code
weight: 4
description: >
    Примеры кода и простые программы для демонстрации возможностей языка

simple_list: false
categories: [Examples, Placeholders]
tags: [test, docs]
---

Примеры программ на NewLang


<style>
    .rownr {width: 5%; overflow-y: hidden; background-color: rgb(105,105,105); color: white; 
           text-align: right; vertical-align:top; resize: none;}
    .txt {width: 90%; overflow-x: scroll; resize: none; font-family: monospace;}
    .out {width: 95%; font-family: monospace;}
    .error {background-color: rgb(255, 180, 180); }
</style>

<div>
<select class="c10" style="width: 95%;" onchange="SelectExample(this);" id="example_list"> 
    <option selected="selected" value="" id="example_start">Select an exmaple...</option>
    <option>Hello, world!</option>
    <option>Rational numbers without limitation of precision</option>
    <option>3</option>
    <option>4</option>
    <option>5</option>
</select>
<div>
    <textarea class="rownr" rows="20" 
    cols="3" value="1" readonly></textarea>
    <span>
        <textarea 
        class="txt" 
        rows="20" 
        id="playground"
        cols="150" 
        nowrap="nowrap" 
        wrap="off"
        autocomplete="off" 
        autocorrect="off" 
        autocapitalize="off" 
        spellcheck="false"
        onclick="selectionchanged(this)" 
        onkeyup="keyup(this,event)" 
        oninput="input_changed(this)" 
        onscroll="scroll_changed(this)"></textarea><br/><br/>
<!--            <label>Current position: 
        </label><input id="sel_in" style="border-style:none" readonly>  -->
    </span>
</div>

<div>
    <button onclick="run_playground()">Playground run</button>
    <label class="pg_out" style="display:none" >Playground version:</label>
    <input id="pg_version" class="pg_out" style="border-style:none; display:none" size="50" readonly>
</div>

<div id="pg_out_div" class="pg_out" style="display:none">
    <label>Output:</label>
    <textarea class="out" rows="5" value="" id="playground_out" style="font-family: monospace; resize: none" readonly ></textarea>
</div>
</div>


<script>

locations =[ "",
    "{{< source "hello.src" >}}",
    "{{< source "rational.src" >}}",

    /*option 3*/                 
    " This is template 3 that  will appear in a textarea keeping its formatting as  is. Donec tortor lorem,  ornare vitae commodo nec,  sagittis et nunc. Maecenas sagittis quam ",

    /*option 4*/                 
    "#!../output/nlc --eval-file\n\nprintf('Hello, world!');\n",

    /*option 5*/                 
    "etc...", ];

function SelectExample(sel){   

    srcLocation = locations[sel.selectedIndex];
    if (srcLocation != undefined && srcLocation != "") {
        obj = document.getElementById('playground');
        obj.value= locations    [sel.selectedIndex];
        input_changed(obj);
    } 
}



function populate_rownr(obj_rownr, cntline){
    obj_rownr.value = '';
    for (let i = 1; i <= cntline; i++) {
      obj_rownr.value += i;
      obj_rownr.value += '\n';
    }
}

function input_changed(obj_txt)
    {
        obj_rownr = obj_txt.parentElement.parentElement.getElementsByTagName('textarea')[0];
        cntline = obj_txt.value.split('\n').length
        if(cntline == 0) cntline = 1;
        tmp_arr = obj_rownr.value.split('\n');
        cntline_old = parseInt(tmp_arr[tmp_arr.length - 1], 10);
        // if there was a change in line count
        if(cntline != cntline_old)
        {
            obj_rownr.cols = cntline.toString().length; // new width of txt_rownr
            populate_rownr(obj_rownr, cntline);
            scroll_changed(obj_txt);
        }
        selectionchanged(obj_txt);
    }

function scroll_changed(obj_txt)
    {
        obj_rownr = obj_txt.parentElement.parentElement.getElementsByTagName('textarea')[0];
        scrollsync(obj_txt,obj_rownr);
    }
    
function scrollsync(obj1, obj2)
    {
        // scroll text in object id1 the same as object id2
        obj2.scrollTop = obj1.scrollTop;
    }


function selectionchanged(obj)
{
    /*
    var substr = obj.value.substring(0,obj.selectionStart).split('\n');
    var row = substr.length;
    var col = substr[substr.length-1].length;
    var tmpstr = '(' + row.toString() + ',' + col.toString() + ')';
    // if selection spans over
    if(obj.selectionStart != obj.selectionEnd)
    {
        substr = obj.value.substring(obj.selectionStart, obj.selectionEnd).split('\n');
        row += substr.length - 1;
        col = substr[substr.length-1].length;
        tmpstr += ' - (' + row.toString() + ',' + col.toString() + ')';
    }
    obj.parentElement.getElementsByTagName('input')[0].value = tmpstr;
    */
}

function keyup(obj, e)
{
    if(e.keyCode == 13 || (e.keyCode >= 33 && e.keyCode <= 40)){
        selectionchanged(obj, e.keyCode);
    }
        document.getElementById('example_list').value = "";
}





function run_playground(){   

    //document.getElementById('pg_out_div').style.display = 'none';

    for (let el of document.querySelectorAll('.pg_out')) {
        el.style.display="none";
    }

    // 1. Создаём новый XMLHttpRequest-объект
    let xhr = new XMLHttpRequest();
    xhr.timeout = 2000;
    xhr.responseType = 'json';

    let url =  new URL('/cgi-bin/playground.cgi?'+escape(document.getElementById('playground').value), 'http://localhost');
    //url.searchParams.set('q', document.getElementById('playground').value);

    // 2. Настраиваем его: GET-запрос по URL /article/.../load
    xhr.open('GET', url);

    // 3. Отсылаем запрос
    xhr.send();

    // 4. Этот код сработает после того, как мы получим ответ сервера
    xhr.onload = function() {
        out = document.getElementById('playground_out');
        out.value = unescape(xhr.response.out);
        //document.getElementById('playground_out').value = unescape(xhr.response.out);
        document.getElementById('pg_version').value = unescape(xhr.response.version);
        document.getElementById('pg_out_div').style.display = 'block';

        for (let el of document.querySelectorAll('.pg_out')) {
            el.style.display="";
        }

        if (xhr.status != 200) { // анализируем HTTP-статус ответа, если статус не 200, то произошла ошибка
              //alert(`Ошибка ${xhr.status}: ${xhr.statusText}`); // Например, 404: Not Found
            out.classList.add("error");
        } else { // если всё прошло гладко, выводим результат
              //alert(`Готово, получили ${xhr.response.length} байт`); // response -- это ответ сервера
            out.classList.remove("error");
        }
    };

    xhr.onprogress = function(event) {
      if (event.lengthComputable) {
        //alert(`Получено ${event.loaded} из ${event.total} байт`);
      } else {
        //alert(`Получено ${event.loaded} байт`); // если в ответе нет заголовка Content-Length
      }

    };

    xhr.onerror = function() {
        document.getElementById('playground_out').classList.add("error");
        
      //alert("Запрос не удался");
    };
}
</script>     


sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss