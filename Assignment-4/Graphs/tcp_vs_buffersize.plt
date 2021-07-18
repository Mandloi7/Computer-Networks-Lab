set terminal png
set output "tcp_vs_buffersize.png"
set title "Buffer Size vs Throughput(TCP)"
set xlabel "Buffer Size(# of packets)"
set ylabel "Throughput(Mbps)"

set xrange [0:800]
plot "-"  title "TCP Throughput" with linespoints
10 6.59993
35 22.1996
60 38.1136
85 44.4042
110 44.6173
135 44.689
160 44.7377
185 44.7991
210 44.8606
235 44.9221
260 44.996
350 45.2172
440 45.4259
530 45.6471
620 45.8717
710 46.0928
800 46.312
e
