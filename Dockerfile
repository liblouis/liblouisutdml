FROM liblouis/liblouis

# Fetch build dependencies
RUN apt-get update && apt-get install -y \
    libxml2-dev \
   && rm -rf /var/lib/apt/lists/*

# compile and test liblouisutdml
ADD . /usr/src/liblouisutdml
WORKDIR /usr/src/liblouisutdml
RUN ./autogen.sh \
    && ./configure --disable-java-bindings \
    && make check \
    && make install \
    && ldconfig

