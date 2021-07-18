set terminal png
set output "udp_vs_buffersize.png"
set title "Buffer Size vs Throughput(UDP)"
set xlabel "Buffer Size(# of packets)"
set ylabel "Throughput(Mbps)"

set xrange [0:800]
plot "-"  title "UDP Throughput" with linespoints
10 62.7928
35 62.7336
60 62.6721
85 62.6217
110 62.6199
135 62.622
160 62.6206
185 62.6206
210 62.6206
235 62.6206
260 62.622
350 62.622
440 62.6206
530 62.6206
620 62.6205
710 62.6205
800 62.6195
e
