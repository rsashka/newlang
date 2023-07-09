#ifndef __LIST2CPP_newlang_dsl
#define __LIST2CPP_newlang_dsl
inline constexpr unsigned long newlang_dsl_size = 48;

inline const char *newlang_dsl_arr[newlang_dsl_size] = {
"#!../output/nlc --eval",
"",
"@@true 1:Bool@@@",
"@@yes 1:Bool@@@",
"",
"@@false 0:Bool@@@",
"@\\no 0:Bool@@@",
"",
"@@this $0@@@  ",
"",
"@@parent $$@@@ ",
"",
"@@args @$*@@@",
"@@sys @@@@@",
"@@current @$@@@",
"@@cmd @*@@@",
"",
"@@run(...) @@(@$*)@@@",
"@@ifmain [@$.$0]-->@@@",
"@@ifload [@$.__load__]-->@@@",
"",
"@@match(cond) [@$cond]@@@",
"@@if(...) [@$*]-->@@@",
"@@elif(...) ,[@$*]-->@@@",
"@@else ,[_]-->@@@",
"",
"@@while(cond) [@$cond] <->@@@",
"@@dowhile(cond) <->[@$cond]@@@",
"",
"@@iter(obj, ... ) @$obj ? (@$*) @@@",
"@\\next(obj, ... ) @$obj ! (@$*) @@@",
"@@curr(obj, ... ) @$obj !? (@$*) @@@",
"@@first(obj) @$obj !! @@@",
"@@all(obj)   @$obj ?? @@@",
"",
"",
"@@break+  ++:Break++@@@",
"@@break-  --:Break--@@@",
"@@continue+  ++:Continue++@@@",
"@@continue-  --:Continue--@@@",
"",
"@@return ++@@@",
"@@return(value) ++@$value++@@@",
"",
"@@error --@@@",
"@@error(value) --@$value--@@@",
"",
"'OK'"
};


#endif

