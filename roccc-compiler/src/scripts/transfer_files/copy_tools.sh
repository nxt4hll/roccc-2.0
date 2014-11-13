#!/bin/bash

ROCCC_BINARY_DIR="<edit_me>/"
ROCCC_SOURCE_DIR="<edit_me>/roccc-compiler/"

if [ -d "$ROCCC_BINARY_DIR" ];
then
  if [ -d "$ROCCC_SOURCE_DIR" ];
  then
    cp -f $ROCCC_SOURCE_DIR/src/tools/createScript $ROCCC_BINARY_DIR/bin
    cp -f $ROCCC_SOURCE_DIR/src/tools/initializeFP $ROCCC_BINARY_DIR/bin
  else
    echo "Error: Couldn't find ROCCC_SOURCE_DIR at '$ROCCC_SOURCE_DIR'"
  fi
else
  echo "Error: Couldn't find ROCCC_BINARY_DIR at '$ROCCC_BINARY_DIR'"
fi
