file=$(basename $0 .sh)
make coverage > $file.log 2>&1
echo "**** $file ****" >> summary.log
tail -1 $file.log >> summary.log
echo "" >> summary.log
