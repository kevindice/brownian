wget https://mirrors.ocf.berkeley.edu/gnu/gsl/gsl-2.4.tar.gz
tar -zxvf gsl-*.*.tar.gz
cd gsl-2.4
./configure --prefix=/homes/$(whoami)/gsl
make
make check
make install
