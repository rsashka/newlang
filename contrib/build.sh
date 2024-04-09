cd text2cpp
./autogen.sh
./configure --prefix=`pwd`/output
make
make install

echo Build Google UnitTest freamework ...
cd ../googletest
cmake .
make
cd ..

echo Download libtorch ...
if [ ! -e libtorch.zip ]; then
  curl https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.2.2%2Bcpu.zip --output libtorch.zip
fi

echo Unzip libtorch ...
unzip libtorch.zip -d .

