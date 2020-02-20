#!/bin/bash

mkdir stegotexts

(for i in `seq 72 81`; do
    echo $i
done) | xargs -n 1 -P 5 ./one_stegotext_build.sh
