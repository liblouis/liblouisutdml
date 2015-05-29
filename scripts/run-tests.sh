#!/bin/bash
set -ev
if [ "${LIBLOUIS_VERSION}" = "master" ]; then
    LIBLOUIS_DOWNLOAD_URL=https://github.com/liblouis/liblouis/archive/master.tar.gz
else
    LIBLOUIS_DOWNLOAD_URL=https://github.com/liblouis/liblouis/releases/download/v${LIBLOUIS_VERSION}/liblouis-${LIBLOUIS_VERSION}.tar.gz
fi
wget -O liblouis-release.tar.gz ${LIBLOUIS_DOWNLOAD_URL}
tar -xf liblouis-release.tar.gz
cd liblouis-* && ./configure && make && sudo make install && cd ..
sudo ldconfig
./autogen.sh && ./configure && make && make check
