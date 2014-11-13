#!/bin/bash

ROCCC_BINARY_DIR="<edit_me>/"
ROCCC_SOURCE_DIR="<edit_me>/roccc-compiler/"
GCC_INSTALL_DIR="<edit_me>/gcc-4.0.2-install/"
LLVM_GCC_INSTALL_DIR="<edit_me>/llvm-gcc4.2-2.3-x86_64-linux-CentOS/"

if [ -d "$ROCCC_BINARY_DIR" ];
then
  if [ -d "$GCC_INSTALL_DIR" ];
  then
    cp -f $GCC_INSTALL_DIR/bin/* $ROCCC_BINARY_DIR/bin
    cp -f $GCC_INSTALL_DIR/libexec/gcc/x86_64-unknown-linux-gnu/4.0.2/cc1 $ROCCC_BINARY_DIR/bin
    cp -f $GCC_INSTALL_DIR/libexec/gcc/x86_64-unknown-linux-gnu/4.0.2/cc1plus $ROCCC_BINARY_DIR/bin
    cp -f $GCC_INSTALL_DIR/libexec/gcc/x86_64-unknown-linux-gnu/4.0.2/collect2 $ROCCC_BINARY_DIR/bin
    cp -f $GCC_INSTALL_DIR/lib/gcc/x86_64-unknown-linux-gnu/4.0.2/libgcc.a $ROCCC_BINARY_DIR/lib
    cp -f $GCC_INSTALL_DIR/lib/gcc/x86_64-unknown-linux-gnu/4.0.2/libgcc_eh.a $ROCCC_BINARY_DIR/lib
    cp -f $GCC_INSTALL_DIR/lib/gcc/x86_64-unknown-linux-gnu/4.0.2/libgcov.a $ROCCC_BINARY_DIR/lib
    cp -f $GCC_INSTALL_DIR/lib/libiberty.a $ROCCC_BINARY_DIR/lib
  else
    echo "Error: Couldn't find GCC_INSTALL_DIR at '$GCC_INSTALL_DIR'"
  fi


  if [ -d "$ROCCC_SOURCE_DIR" ];
  then
    cp -f $ROCCC_SOURCE_DIR/bin/parser $ROCCC_BINARY_DIR/bin/
    cp -f $ROCCC_SOURCE_DIR/bin/suifdriver $ROCCC_BINARY_DIR/bin/
    cp -f $ROCCC_SOURCE_DIR/src/llvm-2.3/sqlite-3.6.11/.libs/libsqlite3.so* $ROCCC_BINARY_DIR/lib/
  else
    echo "Error: Couldn't find ROCCC_SOURCE_DIR at '$ROCCC_SOURCE_DIR'"
  fi


  if [ -d "$LLVM_GCC_INSTALL_DIR" ];
  then
    cp -f $LLVM_GCC_INSTALL_DIR/libexec/gcc/x86_64-unknown-linux-gnu/4.2.1/cc1 $ROCCC_BINARY_DIR/bin/llvmGcc/
    cp -f $LLVM_GCC_INSTALL_DIR/libexec/gcc/x86_64-unknown-linux-gnu/4.2.1/collect2 $ROCCC_BINARY_DIR/bin/llvmGcc/
    cp -f $LLVM_GCC_INSTALL_DIR/bin/llvm-gcc $ROCCC_BINARY_DIR/bin/llvmGcc/
  else
    echo "Error: Couldn't find LLVM_GCC_INSTALL_DIR at '$LLVM_GCC_INSTALL_DIR'"
  fi



else
  echo "Error: Couldn't find ROCCC_BINARY_DIR at '$ROCCC_BINARY_DIR'"
fi
