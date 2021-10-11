#!/bin/bash

help()
{
    echo "call from lart root directory"
    echo "syntax: setup.sh <lart-install-dir> <final-package-dir>"
}

if [[ $# -ne 2 ]]; then
    echo "wrong arguments supplied"
    help
    exit 1
fi

if [[ -L ./lart ]]; then
  echo "lart is a symlink!"
  exit 1
fi

LART=$1
DEST=$2

if [[ -d $DEST ]]; then
    echo "$DEST already exists!"
    exit 1
fi

SVCOMP=./scripts/svcomp

mkdir -p $DEST
mkdir -p $DEST/utils
mkdir -p $DEST/bin

cp -t $DEST LICENSE $SVCOMP/README $SVCOMP/lart $SVCOMP/__init__.py
cp -r $SVCOMP/utils $DEST
cp -r $LART/bin $DEST
cp -r $LART/include $DEST
cp -r $LART/lib $DEST
