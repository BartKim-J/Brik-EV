#!/bin/bash

cd /media/BRIK_USB

find . -name "BrikEVC*" |  
while read file  
do  
  echo "${file}"
  cp ${file} /brik/brik_ev_c/bin/build/BrikEVC_exe
done  

