#! /bin/bash

RootPath="../build/benchmark"
SimpleBenchmarks="simple_hsm simple_sml simple_euml simple_sc "
ComplexBenchmarks="complex_hsm complex_sml complex_euml complex_sc"

for Name in $SimpleBenchmarks; do
    Benchmark="$RootPath/simple/$Name"
    Result=$((eval $Benchmark) 2>&1 | grep "execution speed" | awk '{ print $3 }')
    echo $Name $Result
done

for Name in $ComplexBenchmarks; do
    Benchmark="$RootPath/complex/$Name"
    Result=$((eval $Benchmark) 2>&1 | grep "execution speed" | awk '{ print $3 }')
    echo $Name $Result
done
