#!/bin/zsh
while inotifywait -r -e 'modify' .
do
  cmake . && make -j10 && test/ledger-rest_tests
done
