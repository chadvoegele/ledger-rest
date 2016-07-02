FROM ubuntu:xenial
RUN \
      apt-get update && \
      apt-get install -y git cmake make g++ libgnutls-dev libboost-date-time-dev libboost-filesystem-dev libboost-dev libboost-iostreams-dev libboost-regex-dev libboost-test-dev libcurl4-gnutls-dev libgtest-dev coreutils libmicrohttpd-dev libmpfr-dev
WORKDIR /usr/src/gtest
RUN \
      cmake . && make && \
      install -m 644 libgtest.a /usr/lib/libgtest.a && \
      install -m 644 libgtest_main.a /usr/lib/libgtest_main.a
WORKDIR /tmp/ledger_build
RUN \
      git clone https://github.com/ledger/ledger.git && \
      cd ledger && \
      git checkout v3.1.1 && \
      install -m 644 lib/utfcpp/v2_0/source/utf8.h /usr/include/utf8.h && \
      install -d /usr/include/utf8 && \
      find lib/utfcpp/v2_0/source/utf8 -type f -exec install -m 644 -D {} /usr/include/utf8/ \; && \
      cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_DEBUG=0 . && \
      make -j$(nproc) install
WORKDIR /tmp/ledger-rest_build
RUN \
      git clone https://github.com/chadvoegele/ledger-rest.git && \
      cd ledger-rest && \
      cmake -DCMAKE_INSTALL_PREFIX=/usr . && \
      make -j$(nproc) install
WORKDIR /
RUN \
      rm -r /tmp/ledger_build && \
      rm -r /tmp/ledger-rest_build
ENTRYPOINT ["/usr/bin/ledger-rest"]
