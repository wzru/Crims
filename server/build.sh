#!/bin/bash

make -C parser static
cd build && cmake .. && make clean && make -j8