#! /usr/bin/gnuplot

set terminal svg noenhanced
set title "Hsm Simple Benchmark Compile Time Results"
set output "simple_benchmark_compiletime_results.svg"
set boxwidth 0.5 relative
set style fill solid 1.0
set ylabel "compile time [s]"
set ytics nomirror
set xtics nomirror
plot "../build/simple_benchmark_compiletime_results.dat" using 2: xtic(1) with boxes notitle linecolor rgb "orange"