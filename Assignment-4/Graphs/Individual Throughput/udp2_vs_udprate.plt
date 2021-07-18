set terminal png
set output "udp2_vs_udprate.png"
set title "UDP Rate vs UDP Throughput of Connection 2"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "UDP Throughput of Connection 2" with linespoints
20 19.2387
30 22.4734
40 26.5195
50 30.8908
60 35.424
70 40.0497
80 44.7337
90 49.4561
100 53.8778
e
