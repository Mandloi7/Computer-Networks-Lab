set terminal png
set output "tcp2_vs_udprate.png"
set title "UDP Rate vs TCP Throughput of Connection 2"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "TCP Throughput of Connection 2" with linespoints
20 0.0506471
30 0.131441
40 0.198935
50 0.243812
60 0.274385
70 0.293082
80 0.308768
90 0.321761
100 0.330323
e
