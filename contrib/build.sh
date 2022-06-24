cd libffi
./autogen.sh
./configure --prefix=`pwd`/output
make
make install
cd ..
