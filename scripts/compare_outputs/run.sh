#!/bin/bash
PYTHON_SCRIPT="run.py"
CPP_EXECUTABLE="../../build/examples/run"

PYTHON_OUTPUT="python_output.txt"
CPP_OUTPUT="cpp_output.txt"
DIFF_OUTPUT="diff_output.txt"

# Clean outputs
rm *_output.txt

# Generate outputs
python3 $PYTHON_SCRIPT $1 $2 > $PYTHON_OUTPUT
$CPP_EXECUTABLE $1 "$2" > $CPP_OUTPUT

# Generate diff
diff $PYTHON_OUTPUT $CPP_OUTPUT > $DIFF_OUTPUT
if [ -s $DIFF_OUTPUT ]; then
    echo "Outputs are different. Check $DIFF_OUTPUT for details."
    exit 1
else
    echo "Outputs are identical."
    exit 0
fi