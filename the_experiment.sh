mkdir stegotexts

for i in `seq 1 1000`; do
    echo $i
    ./sampler > stegotexts/$i
done
