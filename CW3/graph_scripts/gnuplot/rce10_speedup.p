# Gnuplot script
set size 1.0, 0.6
set terminal postscript portrait enhanced mono dashed lw 1 "Helvetica" 10 
set output "./graph_scripts/results/rce10_speedup.ps"
set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtics nomirror rotate by -45 font ",8"
set boxwidth 0.5
set style fill solid
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
unset key
set title "Performance of different Fourier Transforms"
set ylabel "Performance Relative to Original Direct Fourier Transform"
plot "./graph_scripts/results/rce10_speedup" using 1:3:xtic(2) with boxes,\
	"./graph_scripts/results/rce10_speedup" using 1:($3+1000):3 with labels font ",8"