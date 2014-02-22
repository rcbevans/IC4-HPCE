# Gnuplot script
set size 1.0, 0.6
set terminal postscript portrait enhanced mono dashed lw 1 "Helvetica" 14 
set output "./graph_scripts/results/rce10_scalability.ps"
set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title "Performance of Optimized Fast Fourier Transform against Number of Cores Employed"
set xlabel "Number of Cores Used"
set ylabel "Execution Time (S)"
plot "./graph_scripts/results/rce10_scalability" using 1:7 title 'Execution Time' with linespoints