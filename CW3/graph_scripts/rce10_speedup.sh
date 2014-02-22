#!/bin/bash

echo "" > ./graph_scripts/results/rce10_speedup

count=0

basespeed=`./bin/time_fourier_transform hpce.direct_fourier_transform | grep -w $1 | sed 's/,//g' | sed -e 's/^.* //'`

echo $basespeed

for i in 'hpce.direct_fourier_transform' 'hpce.rce10.direct_fourier_transform_parfor' \
	'hpce.fast_fourier_transform' 'hpce.rce10.fast_fourier_transform_taskgroup' \
	'hpce.rce10.fast_fourier_transform_parfor' 'hpce.rce10.fast_fourier_transform_combined' \
	'hpce.rce10.fast_fourier_transform_opt'

do
	echo -n $count " " >> ./graph_scripts/results/rce10_speedup
	speed=`./bin/time_fourier_transform $i | grep -w $1 | sed 's/,//g'  | sed -e 's/^.* //'`
	echo $i $(bc <<< "scale=5; $basespeed / $speed" | xargs printf "%1.0f") >> ./graph_scripts/results/rce10_speedup
	let count++
done

gnuplot ./graph_scripts/gnuplot/rce10_speedup.p
ps2pdf ./graph_scripts/results/rce10_speedup.ps ./rce10_speedup.pdf
rm ./graph_scripts/results/rce10_speedup.ps ./graph_scripts/results/rce10_speedup