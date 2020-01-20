#! /usr/bin/gnuplot

set terminal svg noenhanced
set title "Hsm Complex Benchmark Compile Time Results"
set output "complex_benchmark_compiletime_results.svg"
set boxwidth 0.5 relative
set style fill solid 1.0
set ylabel "compile time [ms]"
set ytics nomirror
set xtics nomirror
plot "../build/complex_benchmark_compiletime_results.dat" using 2: xtic(1) with boxes notitle linecolor rgb "blue"