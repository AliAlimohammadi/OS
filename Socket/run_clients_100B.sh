#!/bin/bash

for i in $(seq 1 10)
do
    ./client -h 127.0.0.1 -p 8080 dvuqnttzxflchasaizhxchneylwsgmujrcpkhrkfdposuorrwtbklfmpafcxsvypmlvrxbkxevrgenrwpfuiwpoghttbyoqpjzag &
done

wait
