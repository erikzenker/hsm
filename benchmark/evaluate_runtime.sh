#! /bin/bash

RootPath="../build/benchmark"
SimpleBenchmarks="simple_hsm simple_euml simple_sc"
ComplexBenchmarks="complex_euml complex_hsm complex_sc complex_switch"

rm simple_benchmark_results.dat
for Name in $SimpleBenchmarks; do
    Benchmark="$RootPath/simple/$Name"
    Result=$((eval $Benchmark) 2>&1 | grep "execution speed" | awk '{ print $3 }')
    echo $Name $Result >> simple_benchmark_results.dat
done
./simple_runtime.pl

rm complex_benchmark_results.dat
for Name in $ComplexBenchmarks; do
    Benchmark="$RootPath/complex/$Name"
    Result=$((eval $Benchmark) 2>&1 | grep "execution speed" | awk '{ print $3 }')
    echo $Name $Result >> complex_benchmark_results.dat
done
./complex_runtime.pl