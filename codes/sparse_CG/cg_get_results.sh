#!/bin/bash

DIR=$1

echo "Threads;GFLOPS;#ITERS;Testcase;Run;Hostname;bin;Date;MatVec[s];SOLVE[s];TOTAL[s];Tasks"
for i in `ls ${DIR}/*log`
do
	BASENAME=`basename $i`
	if `echo $i|grep -q shell`; then 
		continue
	fi
	if grep -q DEBUG $i; then
		echo "WARNING: Debug output"
	fi
	if grep -q "NUMBER OF REMOTE COMPUTING" $i; then 
		echo "WARNING: TASK_DIST included"
	fi
	cat $i|grep -E "Threads.*:"|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep "MatVec GFLOP"|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep Iterations|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep Matrix|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	echo -n ${BASENAME/.log}|awk 'BEGIN {FS="_Run"} {printf "%s;",$2}'
	cat $i|grep Hostname|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep Version|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep Build|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep "MatVec time"|cut -d ":" -f 2|tr -s " "|tr "\n" ";"	
	cat $i|grep Solve|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep Total|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	cat $i|grep Tasks|cut -d ":" -f 2|tr -s " "|tr "\n" ";"
	echo
done
         
