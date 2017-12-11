# Get GSL

cd ~
wget https://mirrors.ocf.berkeley.edu/gnu/gsl/gsl-2.4.tar.gz
tar -xvzf gsl-*.*.tar.gz
mkdir gsl
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


# Now get halmd
cd ~
TAR="halmd-0.2.1.tar.bz2" URL="http://code.halmd.org/tar"; \
wget "$URL/$TAR" && openssl dgst -sha512 \
  -verify <(wget -qO- "$URL/cert.pem" | openssl x509 -noout -pubkey) \
  -signature <(wget -qO- "$URL/$TAR.sig") "$TAR"

tar -xvjf halmd-*.tar.bz2

mkdir -p /tmp/halmd_prerequisites$USER && cd /tmp/halmd-prerequisites$USER
nice make -f ~/halmd/examples/packages.mk -j6 install
make -f ~/halmd/examples/packages.mk env >> ~/.bashrc
make -f ~/halmd/examples/packages.mk env >> ~/.zshrc

mkdir -p /tmp/halmd-build$USER && cd /tmp/halmd-build$USER
cmake ~/halmd -DCMAKE_INSTALL_DIRECTORY=~/opt/halmd
nice make -j6

~/halmd/halmd --version
echo "export PATH=\"${HOME}/opt/halmd/bin\${PATH+:\$PATH}\"" >> ~/.bashrc
echo "export PATH=\"${HOME}/opt/halmd/bin\${PATH+:\$PATH}\"" >> ~/.zshrc

if [ ! -f ~/.zshrc ]; then
    source ~/.bashrc
else
    source ~/.zshrc
fi
