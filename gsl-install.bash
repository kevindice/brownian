#!/bin/bash

#Get GSL

cd /tmp
mkdir setup
cd setup

wget https://mirrors.ocf.berkeley.edu/gnu/gsl/gsl-2.4.tar.gz
tar -xvzf gsl-*.*.tar.gz
mkdir ~/gsl
cd gsl-2.4
./configure --prefix=$HOME/gsl
make -j6
make check
make install
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$HOME/gsl/lib/" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$HOME/gsl/lib/" >> ~/.zshrc
if [ ! -f ~/.zshrc ]; then
    source ~/.bashrc
else
    source ~/.zshrc
fi
