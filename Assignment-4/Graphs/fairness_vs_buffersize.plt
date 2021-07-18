set terminal png
set output "fairness_vs_buffersize.png"
set title "Buffer Size vs Fairness plot"
set xlabel "Buffer Size(# of packets)"
set ylabel "Fairness"

set xrange [0:800]
plot "-"  title "Fairness" with linespoints
10 0.269903
35 0.386456
60 0.487859
85 0.523123
110 0.524746
135 0.525126
160 0.525269
185 0.525536
210 0.525803
235 0.52607
260 0.526459
350 0.527413
440 0.528236
530 0.529176
620 0.530136
710 0.531062
800 0.531967
e
