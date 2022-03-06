#!/bin/bash
dir1="./simulation_result/CDF/metric1/"
dir2="./simulation_result/CDF/metric2/"
dir3="./simulation_result/CDF/metric3/"

for mou in `ls $dir1`
	do
		rm -rf $dir1$mou
	done

for mou in `ls $dir2`
	do
		rm -rf $dir2$mou
	done

for mou in `ls $dir3`
	do
		rm -rf $dir3$mou
	done
