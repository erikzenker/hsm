#! /bin/bash

RootPath="../build/"
SimpleBenchmarks="simple_hsm simple_sml simple_euml simple_sc"
ComplexBenchmarks="complex_hsm complex_sml complex_euml complex_sc"

cd $RootPath
make clean

for Name in $SimpleBenchmarks; do
    Result=$((/usr/bin/time -f "user %e" make $Name) 2>&1 | grep "user" | awk '{ print $2 }')
    echo $Name $Result
done

for Name in $ComplexBenchmarks; do
    Result=$((/usr/bin/time -f "user %e" make $Name) 2>&1 | grep "user" | awk '{ print $2 }')
    echo $Name $Result
done
