cd z:\NewLang\newlang\contrib\libffi
./configure CC="msvcc.sh -m64" CXX="msvcc.sh -m64" LD=link CPP="cl -nologo -EP" CPPFLAGS="-DFFI_BUILDING_DLL"

rem PATH="C:\Program^ Files\Microsoft^ Visual^ Studio\2022\Community\VC\Tools\MSVC\14.32.31326\bin\Hostx64\x64;%PATH%"

rem /z/NewLang/newlang/contrib/libffi/configure CC=/z/NewLang/newlang/contrib/libffi/msvcc.sh CXX=/z/NewLang/newlang/contrib/libffi/msvcc.sh LD=link CPP="cl -nologo -EP" CPPFLAGS="-DFFI_BUILDING_DLL"

rem C:\msys64\usr\bin\bash -lc "cd /z/NewLang/newlang/contrib/libffi && ./configure CC='./msvcc.sh -m64' -- --prefix=`pwd`/vs --disable-docs"


rem C:\msys64\usr\bin\bash -lc "cd libffi && ./configure --build=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --prefix=`pwd`/win64 --disable-docs && make && make install && .."



