#!/usr/bin/env zsh
### Job name
#BSUB -J terminal
#BSUB -W 02:00
#BSUB -m c144m1024
#BSUB -n 1
#BSUB -x
#BSUB -a openmp
#BSUB -P jara0001
#BSUB -M 120000
#BSUB -XF
 
xterm
