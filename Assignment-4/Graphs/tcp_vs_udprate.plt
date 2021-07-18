set terminal png
set output "tcp_vs_udprate.png"
set title "UDP Rate vs Throughput(TCP)"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "TCP Throughput" with linespoints
20 5.35104
30 5.8143
40 6.08482
50 6.26174
60 6.37769
70 6.45425
80 6.49562
90 6.56466
100 6.59993
e
