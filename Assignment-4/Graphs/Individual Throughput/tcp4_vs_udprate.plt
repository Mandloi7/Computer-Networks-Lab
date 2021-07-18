set terminal png
set output "tcp4_vs_udprate.png"
set title "UDP Rate vs TCP Throughput of Connection 4"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "TCP Throughput of Connection 4" with linespoints
20 2.62814
30 2.77609
40 2.84477
50 2.88605
60 2.91361
70 2.93331
80 2.93912
90 2.96001
100 2.96914
e
