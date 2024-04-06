for instance in instances/instance-*.txt; do
    time program < $instance | check
    echo "................"
done
