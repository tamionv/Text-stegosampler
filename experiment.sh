#!/bin/bash

for jj in {1..20}; do
    j=$((jj*131562))
    head -c "$j" corpus.txt > illiad"$jj".txt
done

(for jj in {1..20}; do
    for i in 25 50; do
        for x in 4 5; do
            mm=$(wc -w "illiad""$jj"".txt" | cut -d' ' -f1)
            echo $i $mm $x $jj
        done
    done
done) | xargs -n 4 -P 3 ./anExperiment.sh
