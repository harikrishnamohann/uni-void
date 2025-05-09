#! /usr/bin/env bash

echo -n "Compiler: "
echo -en "\e[32m"
if command -v clang > ./tmp
then
  echo "clang"
  CC=clang
elif command -v gcc > ./tmp
then
  echo "gcc"
  CC=gcc
else
  echo -e "\e[31mNot found\e[0m"
  exit -1
fi
echo -en "\e[0m"

echo -n "ncurses development library: "
if ! whereis ncurses | grep '/' > ./tmp
then
  echo -e "\e[31mNot found\e[0m"
  exit -1
fi

echo -e "\e[32mfound\e[0m"

mkdir -p target
mkdir -p game_files

CFLAGS="-std=c23 -Wall -Werror -lncurses"
RELEASE="target/uni-void"
DEBUG="target/debug"

compile_debug() {
  echo -e "compiling in \e[32mdebug mode\e[0m..."
  $CC $CFLAGS -g src/main.c -o $DEBUG
}

compile_release() {
  echo -e "compiling in \e[32mrelease mode\e[0m..."
  $CC $CFLAGS -O3 src/main.c -o $RELEASE
}

case $1 in
  "debug")
    compile_debug
    BIN=$DEBUG
    ;;
  "release")
    BIN=$RELEASE
    compile_release
    ;;
  *)
    compile_debug
    exit 1
    ;;
esac

case $2 in 
  "run")
    echo "running..."
    ./$BIN
    ;;
esac

rm ./tmp
