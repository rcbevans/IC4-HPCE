#!/bin/sh
T1=`mktemp ../tmp/XXXXXXX`;
./fir_filter coeffs/f500.csv > $T1;

T2=`mktemp ../tmp/XXXXXXX`;
cat $T1 | ./fir_filter coeffs/f600.csv > $T2;

T3=`mktemp ../tmp/XXXXXXX`;
cat $T2 | ./fir_filter coeffs/f800.csv > $T3;

T4=`mktemp ../tmp/XXXXXXX`;
cat $T3 | ./fir_filter coeffs/f1000.csv > $T4;

T5=`mktemp ../tmp/XXXXXXX`;
cat $T4 | ./fir_filter coeffs/f1200.csv > $T5;

T6=`mktemp ../tmp/XXXXXXX`;
cat $T5 | ./fir_filter coeffs/f1400.csv > $T6;






