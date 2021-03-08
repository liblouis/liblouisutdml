#!/bin/bash
set -ev
# get and build liblouis
if [ "${LIBLOUIS_VERSION}" = "master" ]; then
    wget https://github.com/liblouis/liblouis/archive/master.tar.gz
    tar -xf master.tar.gz
    ( cd liblouis-* && ./autogen.sh && ./configure && make && sudo make install )
else
    wget https://github.com/liblouis/liblouis/releases/download/v${LIBLOUIS_VERSION}/liblouis-${LIBLOUIS_VERSION}.tar.gz
    tar -xf liblouis-*.tar.gz
    ( cd liblouis-${LIBLOUIS_VERSION} && ./configure && make && sudo make install )
fi
sudo ldconfig

# build liblouisutdml
./autogen.sh && ./configure && make && make check
