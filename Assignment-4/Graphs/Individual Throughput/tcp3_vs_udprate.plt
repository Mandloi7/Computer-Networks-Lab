set terminal png
set output "tcp3_vs_udprate.png"
set title "UDP Rate vs TCP Throughput of Connection 3"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "TCP Throughput of Connection 3" with linespoints
20 2.62165
30 2.77553
40 2.84434
50 2.88571
60 2.91332
70 2.93305
80 2.93749
90 2.95982
100 2.96896
e
