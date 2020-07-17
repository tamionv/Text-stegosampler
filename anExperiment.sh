#!/bin/bash

size=$1
model=$2
ctx=$3
filename="results"

echo Trying size $size and model size $model and ctxlen $ctx

for i in {1..10}
do
     echo Trial $i
     name=sampler_j"$4"_m"$size"_c"$ctx"_s500_t7
     thing=$(/usr/bin/time -f "%e" sh -c "./$name 1>/dev/null 2>/dev/null" 2>&1)
     echo $size, $model, $ctx, $thing
     echo $size, $model, $ctx, $thing >> results
done
