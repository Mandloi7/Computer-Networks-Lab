set terminal png
set output "udp_vs_udprate.png"
set title "UDP Rate vs Throughput(UDP)"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "UDP Throughput" with linespoints
20 27.7611
30 31.1618
40 35.2869
50 39.7087
60 44.2811
70 48.927
80 53.6266
90 58.3617
100 62.7928
e
