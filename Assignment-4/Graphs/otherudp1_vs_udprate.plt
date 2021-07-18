set terminal png
set output "otherudp1_vs_udprate.png"
set title "UDP Rate vs Throughput of remaining UDP connection"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "UDP Throughput of Remaining 2 UDPs" with linespoints
20 8.52244
30 8.68845
40 8.76739
50 8.81796
60 8.85708
70 8.87729
80 8.89283
90 8.9056
100 8.91497
e
