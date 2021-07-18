set terminal png
set output "tcp1_vs_udprate.png"
set title "UDP Rate vs TCP Throughput of Connection 1"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "TCP Throughput of Connection 1" with linespoints
20 0.0506106
30 0.131243
40 0.196775
50 0.246166
60 0.276383
70 0.29481
80 0.31025
90 0.323074
100 0.331514
e
