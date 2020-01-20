#! /bin/bash

RootPath="../build/"
SimpleBenchmarks="simple_hsm simple_euml simple_sc"
ComplexBenchmarks="complex_euml complex_hsm complex_sc complex_switch"

cd $RootPath
make clean

rm simple_benchmark_compiletime_results.dat
for Name in $SimpleBenchmarks; do
    Result=$((/usr/bin/time -f "user %e" make $Name) 2>&1 | grep "user" | awk '{ print $2 }')
    echo $Name $Result >> simple_benchmark_compiletime_results.dat
done
./simple_compiletime.pl

rm complex_benchmark_compiletime_results.dat
for Name in $ComplexBenchmarks; do
    Result=$((/usr/bin/time -f "user %e" make $Name) 2>&1 | grep "user" | awk '{ print $2 }')
    echo $Name $Result >> complex_benchmark_compiletime_results.dat
done
./complex_compiletime.pl
