#!/bin/bash
nEndDevices='200 600 1000'
delays='120 65 35 14'

for delay in $delays
do
  for number in $nEndDevices
  do
    start=`date +%s`
    echo '========================================================='
    echo 'In progress: nDevices = '$number', interTransmissionDelay = '$delay
    mkdir -p logs
    ./waf --run "scenario1 --nDevices=$number --delay=$delay" > logs/scen1_${delay}_${number}.txt 2>&1
    end=`date +%s`
    echo 'Finished: nDevices = '$number', interTransmissionDelay = '$delay
    date +%d.%m", "%X
    echo "Execution time: "$((end-start))" s"
    echo -e '=========================================================\n\n'
  done
done

echo "Batch simulation for scenario 1 complete."
date +%d.%m", "%X
