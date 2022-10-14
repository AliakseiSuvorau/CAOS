#!/bin/bash
IFS=","

function change_places {
	#поменять местами 2 строки
	if [[ $i -eq $(($n-1)) ]]
	then
		echo "The system of equations has infinitely many solutions"
		return
	fi
	declare tmp_arr
	for ((t=0;t<$m;t++))
	do
		tmp_arr[$t]=${array[$i, $t]}
		array[$i, $t]=${array[$(($i+1)), $t]}
		array[$(($i+1)), $t]=${tmp_arr[$t]}
	done
}

declare -A array
n=0
flag=0
while read -a line
do
	for ((i=0;i<${#line[@]};i++))
	do
		array[$n, $i]=${line[$i]}
	done
	((n++))
done < input.csv

m=$(($n+1))

for ((i=0;i<n;i++)) # прямой ход алгоритма Гаусса
do
	for ((j=$(($i+1));j<n;j++))
	do
		if (($(echo "${array[$i, $i]}" == ".00000000000000000000" | bc -l )))
		then
			change_places
		fi
		ratio=$(echo "${array[$j, $i]}/${array[$i, $i]}" | bc -l)
		for ((k=$i;k<$m;k++))
		do
			tmp=$(echo "$ratio*${array[$i, $k]}" | bc -l)
			array[$j, $k]=$(echo "${array[$j, $k]}-($tmp)" | bc -l)
		done
	done
done

declare -a sol

for ((i=$(($n-1));i>=0;i--)) # обратный ход алгоритма Гаусса
do
	sum=0
	for ((j=$(($i+1));j<$n;j++))
	do
		sum_component=$(echo "${array[$i, $j]}*${sol[$j]}" | bc -l)
		sum=$(echo "$sum+$sum_component" | bc -l)
	done
	numerator=$(echo "${array[$i, $(($m-1))]}-($sum)" | bc -l)
	if (($(echo "${array[$i, $i]}" == ".00000000000000000000" | bc -l ))) && (($(echo "$numerator" != ".00000000000000000000" | bc -l )))
	then
		echo "The system of equations has no solutions"
		((flag++))
		break
	elif (($(echo "${array[$i, $i]}" == ".00000000000000000000" | bc -l )))
	then
		echo "The system of equations has infinitely many solutions"
		((flag++))
		break
	fi
	sol[$i]=$(echo "$numerator/${array[$i, $i]}" | bc -l)
done

if [[ $flag -eq 0 ]]
then
	for ((i=0;i<$n;i++))
	do
		echo $(echo ${sol[$i]}/1 | bc)
	done
fi
