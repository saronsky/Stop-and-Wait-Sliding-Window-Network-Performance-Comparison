set terminal postscript landscape
set nolabel
set xlabel "window"
set xrange [0:30]
set ylabel "usec"
set yrange [0:8000000]
set output "udp.ps"
plot "1gbps.dat" title "1gbps slinding window" with linespoints, 2031813 title "1gbps stopNwait" with line
