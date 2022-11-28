#!/bin/bash

makebatch()
{
	alpha=$1
	cuts=$2
	name="phe_${cuts}_${alpha}"
	name=${name//[\[,\]]/}
	name=${name//[=,\,,:]/_}
	fname=batch/${name}.sh
	echo "#!/bin/bash"							>  $fname
	echo "#PBS -N ${name}"							>> $fname
	echo "#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_${name}.out"	>> $fname
	echo "#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_${name}.err"	>> $fname
	echo "#PBS -l nodes=1"							>> $fname
	echo "#PBS -q long"							>> $fname
	echo "#PBS -l walltime=100:00:00"					>> $fname
	echo ""									>> $fname
	echo "cd /home/itep/alekseev/igor/PlasticEvolution"			>> $fname
	echo "date"								>> $fname
	echo "./phe_hist ${name}.root 2000 121999 100 -$alpha \"$cuts\""	>> $fname
	echo "date"								>> $fname
	echo "exit 0"								>> $fname
	qsub $fname
}

for alpha in 0.56 ; do
	for cuts in "none" "dist=[0:20]" "dist=[20:40]" "dist=[40:60]" "dist=[60:80]" "dist=[80:100]" \
		"xy=[5:19]" "xy=[5:19],z=[0:9]" "xy=[5:19],z=[10:19]" "xy=[5:19],z=[20:29]" \
		"xy=[5:19],z=[30:39]" "xy=[5:19],z=[40:49]" "xy=[5:19],side=[0:0]"\
		"xy=[5:19],side=[1:1]"; do
		makebatch $alpha $cuts
	done
done
