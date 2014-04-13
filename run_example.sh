#!/bin/bash
LLVM_DIR=/Users/barry/code/llvm/build  #the location of your llvm dir
function ergodic(){
for file in `find $1 -name "*.c" -o -name "*.cxx" -o -name "*.cpp" -o -name "*.S" -o -name "*.inl"`
do
if [ -d $1"/"$file ]
then
ergodic $1"/"$file
else
local path=$1"/"$file
local name=$file
echo $name
fi
done
return
}

IFS=$'\n'                      #这个必须要，否则会在文件名中有空格时出错
INIT_PATH="/Users/barry/code/eCos_code_base/packages_noifdef";
c=$(ergodic $INIT_PATH);

$LLVM_DIR/Debug+Asserts/bin/example $c -- -Iinclude -I/Users/barry/Documents/eCos_Sources/eCos_install/include -target arm-none-eabi