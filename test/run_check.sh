
function run_tests {
	ftests=0
	if [ ! -r "$1" ]
	then
		echo -e "\tFAILED -- cannot find test cases: $1"
		return 1
	fi

	while read line
	do
		$line > /dev/null
		if [ "$?" -ne 0 ]
		then
			ftests=$(($ftests+1))
		fi
	done < "$1"
	return $ftests
}

prog="radixtree"
fail=0
tests=1

echo -e "\n------------------------------\n"
echo "$prog: Running unit tests...:"

exe="rt_unit_test"
if [ ! -x "$exe" ]
then
	echo -e "\tFAILED -- Could not find unit test script: $exe"
	fail=$(($fail+1))
else
	"./$exe"
	if [ "$?" -eq 0 ]
	then
		echo -e "\tSUCCESS -- all unit tests passed"
	else
		fail=$(($fail+1))
		echo -e "\tFAILED -- one or more unit tests failed"
	fi
fi

echo "$prog: Running functionality tests...:"
t="tests-good.txt tests-bad.txt"
for tst in $t
do
	tests=$(($tests+1))
	if [[ $(run_tests $tst) -gt 0 ]]
	then
		echo "FAILED $tst"
		fail=$(($fail+1))
	fi
done

echo "$prog: Passed $(($tests-$fail)) of $tests tests"
if [ "$fail" -gt 0 ]
then
	exit 1
else
	exit 0
fi

# vim:sw=8:ts=8:noet:
