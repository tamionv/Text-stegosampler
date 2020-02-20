#!/bin/bash

for x in results/*
do
    msg=`echo $x | cut -d '_' -f 2`
    model=`echo $x | cut -d '_' -f 4`
    for y in `cat $x`
    do
        echo $msg, $model, $y
    done
done
