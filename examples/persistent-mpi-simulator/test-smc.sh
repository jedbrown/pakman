#!/bin/bash
set -uxo pipefail

touch smc.out smc.err

test_failed()
{
    echo "Test $0 in $(pwd) failed!"
    rm smc.out smc.err
    exit 1
}

# Accept and exit without error
mpirun -n 4 pakman smc persistent-mpi-simulator-accept-no-error.smc 10 \
    --mpi-simulation --persistent 1> smc.out 2> smc.err

# Check exit status
[ $? -ne 0 ] && test_failed

# Check whether standard output is as expected
diff smc.out - << EOF || test_failed
p
1
1
1
1
1
1
1
1
1
1
EOF

# Reject and exit without error
mpirun -n 4 pakman smc persistent-mpi-simulator-reject-no-error.smc 10 \
    --mpi-simulation --persistent 1> smc.out 2> smc.err &

# This job should never end, else the test has failed
# Sleep for 5 seconds then check whether it is still running
sleep 5
[ $(jobs -r | wc -l) -ne 1 ] && test_failed

# Kill job
kill $!

# Accept and exit with error
# This should return with nonzero error code, else the test has failed
mpirun -n 4 pakman smc persistent-mpi-simulator-accept-error.smc 10 \
    --mpi-simulation --persistent 1> smc.out 2> smc.err

# Check exit status
[ $? -eq 0 ] && test_failed

# Reject and exit with error
mpirun -n 4 pakman smc persistent-mpi-simulator-reject-error.smc 10 \
    --mpi-simulation --persistent 1> smc.out 2> smc.err && test_failed

# Check exit status
[ $? -eq 0 ] && test_failed

# Clean up files
rm smc.out smc.err