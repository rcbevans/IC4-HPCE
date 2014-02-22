#!/bin/bash

T1=`mktemp -p ../tmp T1_\`date +%T\`.XXX`;
./fir_filter coeffs/f500.csv > $T1;
T2=`mktemp -p ../tmp T2_\`date +%T\`.XXX`;
cat $T1 | ./fir_filter coeffs/f600.csv > $T2;
T3=`mktemp -p ../tmp T3_\`date +%T\`.XXX`;
cat $T2 | ./fir_filter coeffs/f800.csv > $T3;
T4=`mktemp -p ../tmp T4_\`date +%T\`.XXX`;
cat $T3 | ./fir_filter coeffs/f1000.csv > $T4;
T5=`mktemp -p ../tmp T5_\`date +%T\`.XXX`;
cat $T4 | ./fir_filter coeffs/f1200.csv > $T5;
cat $T5 | ./fir_filter coeffs/f1400.csv;
rm $T1 $T2 $T3 $T4 $T5