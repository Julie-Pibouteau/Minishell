#!/bin/sh

minishell=${MINISHELL:=./minishell}

# sh colors codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

usage(){
    echo "Usage : $0 testfile1.ms [ testfile2.ms ... ]" >&2
}

finish_with_success(){
    echo "Result: ${GREEN}TEST HAS SUCCEEDED${NC}."
    echo ""
}

finish_with_error(){
    echo "Result: ${RED}TEST HAS FAILED${NC}."
    echo ""
}

# test que minishell est bien disponible
if [ ! -f "$minishell" ]
then
    echo "Error: cannot find minishell executable file '$minishell'." >&2
    exit 1
fi

# test que le nombre d'arguments est correct (au moins un fichier test)
if [ $# -lt 1 ]
then
    usage
    exit 1
fi

# create a temporaryfile for output
output_file=$(mktemp -t output_file_XXXX)

for t in $*
do
    echo "#############################"
    echo "EXECUTING TEST: $t."
    echo "#############################"
    echo "Test file:"
    nl -s': ' "$t"

    $minishell < $t > $output_file
    retval=$?

    echo "Test results:"
    solfile="$t.sol"
    if [ -f $solfile ]; then
	expected_retval=$(head -n 1 $solfile)

	# testing return value
	if [ "$expected_retval" -eq "$retval" ]
	then
	    echo "- Return value: minishell has returned the expected return value (${expected_retval})"
	else
	    echo "- Return value: minishell has returned $retval instead of the expected value ${expected_retval}"
	    rm -f "$output_file"
	    finish_with_error
	fi

	# checking that the output is equal to the expected output
	expected_output=$(mktemp -t expected_output_XXXX)
	tail +2 $solfile > $expected_output
	echo "- Expected output file:"
	nl -s': ' $expected_output
	echo "- Actual output file:"
	nl -s': ' $output_file
	echo "- Diff between the expected and the actual file:"
	diff --color -y -s $expected_output $output_file
	diffret=$?
	echo ""
	if [ "$diffret" -eq 0 ]
	then
	    echo "- Expected output: is correct."
	    rm -f "$expected_output" "$output_file"
	    finish_with_success
	else
	    echo "- Expected output is incorrect."
	    rm -f "$expected_output" "$output_file"
	    finish_with_error
	fi
    fi
done
