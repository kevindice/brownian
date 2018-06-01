for i in `seq 1 5`;
do
    bash test-all.sh | bash 2>&1 | tee -a timing_data.txt
done
