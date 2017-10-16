cd ~
wget https://mirrors.ocf.berkeley.edu/gnu/gsl/gsl-2.4.tar.gz
tar -zxvf gsl-*.*.tar.gz
mkdir gsl
cd gsl-2.4
./configure --prefix=/homes/$(whoami)/gsl
make
make check
make install
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/homes/$(whoami)/gsl/lib/" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/homes/$(whoami)/gsl/lib/" >> ~/.zshrc
if [ ! -f ~/.zshrc ]; then
    source ~/.bashrc
else
    source ~/.zshrc
fi

