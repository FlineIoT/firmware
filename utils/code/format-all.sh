#!/bin/sh
if [ "$#" -ne 1 ]; then
    echo "Please pass the root firmware directory as a parameter"
fi

find $1/motion/app \( -name \*.c -or -name \*.cpp -or -name \*.cc -or -name \*.h \) | xargs -n12 clang-format -i
find $1/motion/bootloader_secure_ble \( -name \*.c -or -name \*.cpp -or -name \*.cc -or -name \*.h \) | xargs -n12 clang-format -i
find $1/motion/common \( -name \*.c -or -name \*.cpp -or -name \*.cc -or -name \*.h \) | xargs -n12 clang-format -i
# find ../balios/dtm \( -name \*.c -or -name \*.cpp -or -name \*.cc -or -name \*.h \) | xargs -n12 ./formater.sh -i
