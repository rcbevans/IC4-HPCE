#!/bin/bash

echo "" > ./graph_scripts/results/rce10_scalability

for i in 1 2 3 4 5 6 7 8
do
	echo -n $i " " >> ./graph_scripts/results/rce10_scalability
	./bin/time_fourier_transform hpce.rce10.fast_fourier_transform_opt $i | grep -w $1 | sed 's/,//g' >> ./graph_scripts/results/rce10_scalability
done

gnuplot ./graph_scripts/gnuplot/rce10_scalability.p
ps2pdf ./graph_scripts/results/rce10_scalability.ps ./rce10_scalability.pdf
rm ./graph_scripts/results/rce10_scalability.ps ./graph_scripts/results/rce10_scalability