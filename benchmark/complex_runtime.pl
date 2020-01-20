#! /usr/bin/gnuplot

set terminal svg noenhanced
set title "Hsm Complex Benchmark Results"
set output "complex_benchmark_results.svg"
set boxwidth 0.5 relative
set style fill solid 1.0
set ylabel "runtime [ms]"
set ytics nomirror
set xtics nomirror
plot "complex_benchmark_results.dat" using 2: xtic(1) with boxes notitle linecolor rgb "blue"