#!/bin/bash

#ToDo: make makefile

gcc52 myfind.c -o myfind -Wall -Werror -std=gnu99 -g -Og &&\
./test-find.sh