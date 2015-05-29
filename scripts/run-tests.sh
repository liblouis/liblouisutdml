#!/bin/bash
set -ev
if [ "${LIBLOUIS_VERSION}" = "master" ]; then
    wget https://github.com/liblouis/liblouis/archive/master.tar.gz
else
    wget https://github.com/liblouis/liblouis/releases/download/v${LIBLOUIS_VERSION}/liblouis-${LIBLOUIS_VERSION}.tar.gz
fi
tar -xf liblouis-*.tar.gz
cd liblouis-* && ./configure && make && sudo make install && cd ..
sudo ldconfig
./autogen.sh && ./configure && make && make check
