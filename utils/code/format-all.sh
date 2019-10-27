#!/bin/sh
if [ "$#" -ne 1 ]; then
    echo "Please pass the root firmware directory as a parameter"
fi

find $1/fline/app \( -name \*.c -or -name \*.cpp -or -name \*.cc -or -name \*.h \) | xargs -n12 clang-format -i
find $1/fline/bootloader \( -name \*.c -or -name \*.cpp -or -name \*.cc -or -name \*.h \) | xargs -n12 clang-format -i
find $1/fline/common \( -name \*.c -or -name \*.cpp -or -name \*.cc -or -name \*.h \) | xargs -n12 clang-format -i
