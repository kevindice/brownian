for y in tiny small big; do
  for bin in ../bin/*; do
  x="$(echo "$bin" | cut --complement -c 1-7)"
      echo "bash perf-test.sh $y $x >> output_log.txt"
  done
done
