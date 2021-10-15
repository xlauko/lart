#!/bin/bash

export LART_INSTALL_DIR=./install
./scripts/build.sh

rm -rf submission/
./scripts/svcomp/setup-submission.sh /workspaces/lart/install/ submission

rm lart.zip
cp -r submission lart
rm -rf rm -rf ./lart/utils/__pycache__/
zip -r lart.zip lart
rm -rf lart
