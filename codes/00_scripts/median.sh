#!/bin/bash
#set -x
PREFIX="$3"
PROGRAM_CMD="$4"
#echo "Program that should be executed: $PROGRAM_CMD"

#: << END
FILE_OUT=output_${PREFIX}.txt 
FILE_CUR_RES=curres_${PREFIX}.txt

FILE_ABS=results_${PREFIX}_abs.txt
FILE_AVG=results_${PREFIX}_avg.txt
FILE_TIMES=results_${PREFIX}_times.txt

TMP_TIMES_OVERALL=results_times_${3}_execution.txt

TMP_TIMES_NUMA=results_times_tmp_numa.txt
TMP_TIMES_STEAL=results_times_tmp_steal.txt
TMP_TIMES_REMOVE=results_times_tmp_remove.txt

TMP_TIMES_NUMA_SUM=results_times_tmp_numa_sum.txt
TMP_TIMES_STEAL_SUM=results_times_tmp_steal_sum.txt
TMP_TIMES_REMOVE_SUM=results_times_tmp_remove_sum.txt

NUM_ITER=$1

export KMP_TASK_STEALING_CONSTRAINT=0
export OMP_PLACES=cores 
export OMP_PROC_BIND=spread 
export KMP_A_DEBUG=30 
export OMP_NUM_THREADS=$2

# hack for CG
export CG_NUM_TASKS=$(($OMP_NUM_THREADS * 24))

#rm -f ${TMP_TIMES_OVERALL}

rm -f ${TMP_TIMES_NUMA}
rm -f ${TMP_TIMES_STEAL}
rm -f ${TMP_TIMES_REMOVE}

rm -f ${TMP_TIMES_NUMA_SUM}
rm -f ${TMP_TIMES_STEAL_SUM}
rm -f ${TMP_TIMES_REMOVE_SUM}


# add number of threads in front of measurements
echo -e -n "${2}\t" >> ${TMP_TIMES_OVERALL}

for i in `seq 1 ${NUM_ITER}`; do
# run version
eval "${PROGRAM_CMD}" &> ${FILE_OUT}

#extract times
grep "TASK AFFINITY: __kmp_reap_thread:" ${FILE_OUT} > ${FILE_CUR_RES}

grep "Elapsed time for program" ${FILE_OUT} | cut -d$'\t' -f2 | tr '\n' '\t' >> ${TMP_TIMES_OVERALL}

# ===== extract sums
#rm -f bla
#cat ${FILE_CUR_RES} | grep "find_numa" | cut -f3 > blub
#while IFS='' read -r line || [[ -n "$line" ]]; do	
#	if [[ "$line" != -nan* ]] && [[ "$line" != nan* ]]; then 
#		echo $line >> bla
#	fi 
#done < blub
#awk '{ total += $1; count++ } END { print total/count }' bla >> ${TMP_TIMES_NUMA_SUM}
#rm -f bla
#cat ${FILE_CUR_RES} | grep "steal_search" | cut -f3 > blub
#while IFS='' read -r line || [[ -n "$line" ]]; do	
#	if [[ "$line" != -nan* ]] && [[ "$line" != nan* ]]; then 
#		echo $line >> bla
#	fi 
#done < blub
#awk '{ total += $1; count++ } END { print total/count }' bla >> ${TMP_TIMES_STEAL_SUM}
#rm -f bla
#cat ${FILE_CUR_RES} | grep "remove_my_task" | cut -f3 > blub
#while IFS='' read -r line || [[ -n "$line" ]]; do	
#	if [[ "$line" != -nan* ]] && [[ "$line" != nan* ]]; then 
#		echo $line >> bla
#	fi 
#done < blub
#awk '{ total += $1; count++ } END { print total/count }' bla >> ${TMP_TIMES_REMOVE_SUM}

# ===== extract avg
#rm -f bla
#cat ${FILE_CUR_RES} | grep "find_numa" | cut -f5 > blub
#while IFS='' read -r line || [[ -n "$line" ]]; do	
#	if [[ "$line" != -nan* ]] && [[ "$line" != nan* ]]; then 
#		echo $line >> bla
#	fi 
#done < blub
#awk '{ total += $1; count++ } END { print total/count }' bla >> ${TMP_TIMES_NUMA}
#rm -f bla
#cat ${FILE_CUR_RES} | grep "steal_search" | cut -f5 > blub
#while IFS='' read -r line || [[ -n "$line" ]]; do	
#	if [[ "$line" != -nan* ]] && [[ "$line" != nan* ]]; then 
#		echo $line >> bla
#	fi 
#done < blub
#awk '{ total += $1; count++ } END { print total/count }' bla >> ${TMP_TIMES_STEAL}
#rm -f bla
#cat ${FILE_CUR_RES} | grep "remove_my_task" | cut -f5 > blub
#while IFS='' read -r line || [[ -n "$line" ]]; do	
#	if [[ "$line" != -nan* ]] && [[ "$line" != nan* ]]; then 
#		echo $line >> bla
#	fi 
#done < blub
#awk '{ total += $1; count++ } END { print total/count }' bla >> ${TMP_TIMES_REMOVE}

rm -f blub
rm -f bla

done

echo -e -n "${OMP_NUM_THREADS}\t" >> ${FILE_TIMES}
# cut off first line because equal to number of threads and last empty line
tail -n 1 ${TMP_TIMES_OVERALL} | tr '\t' '\n' | tail -n +2 | sort -n | awk '{ a[i++]=$1; } END { printf "%f\t",a[int(i/2)]; }' | sed -e "s/\./,/g" >> ${FILE_TIMES}
echo -e -n "\n" >> ${FILE_TIMES}
echo -e -n "\n" >> ${TMP_TIMES_OVERALL}


#echo -e -n "${OMP_NUM_THREADS}\t" >> ${FILE_ABS}
#sort -n ${TMP_TIMES_NUMA_SUM} | awk '{ a[i++]=$1; } END { printf "%f\t",a[int(i/2)]; }' | sed -e "s/\./,/g" >> ${FILE_ABS}
#sort -n ${TMP_TIMES_STEAL_SUM} | awk '{ a[i++]=$1; } END { printf "%f\t",a[int(i/2)]; }' | sed -e "s/\./,/g" >> ${FILE_ABS}
#sort -n ${TMP_TIMES_REMOVE_SUM} | awk '{ a[i++]=$1; } END { printf "%f\t",a[int(i/2)]; }' | sed -e "s/\./,/g" >> ${FILE_ABS}
#echo -e -n "\n" >> ${FILE_ABS}

#echo -e -n "${OMP_NUM_THREADS}\t" >> ${FILE_AVG}
#sort -n ${TMP_TIMES_NUMA} | awk '{ a[i++]=$1; } END { printf "%f\t",a[int(i/2)]; }' | sed -e "s/\./,/g" >> ${FILE_AVG}
#sort -n ${TMP_TIMES_STEAL} | awk '{ a[i++]=$1; } END { printf "%f\t",a[int(i/2)]; }' | sed -e "s/\./,/g" >> ${FILE_AVG}
#sort -n ${TMP_TIMES_REMOVE} | awk '{ a[i++]=$1; } END { printf "%f\t",a[int(i/2)]; }' | sed -e "s/\./,/g" >> ${FILE_AVG}
#echo -e -n "\n" >> ${FILE_AVG}


#rm -f ${TMP_TIMES_OVERALL}

rm -f ${TMP_TIMES_NUMA}
rm -f ${TMP_TIMES_STEAL}
rm -f ${TMP_TIMES_REMOVE}

rm -f ${TMP_TIMES_NUMA_SUM}
rm -f ${TMP_TIMES_STEAL_SUM}
rm -f ${TMP_TIMES_REMOVE_SUM}

rm -f ${FILE_CUR_RES}
rm -f ${FILE_OUT}

#END
