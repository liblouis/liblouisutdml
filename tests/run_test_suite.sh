#!/bin/sh

# run_test_suite
#
# Copyright (C) 2013 by Swiss Library for the Blind, Visually Impaired and Print Disabled 
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the Lesser or Library GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.

# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Library
# GNU General Public License for more details.

# You should have received a copy of the Library GNU General Public
# License along with this program; see the file COPYING.  If not, write
# to the Free Software Foundation, 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

verbose=1
colors=1

echo "--------------------------------------------------------------------------------"
echo "TESTS"
echo "--------------------------------------------------------------------------------"

tests=0
error=0
pass=0
fail=0
xfail=0
xpass=0

TESTS_DIR=$(cd $(dirname "$0"); pwd)
cd "$TESTS_DIR"

for test_dir in test_suite/test_* ; do
	tests=$(( tests + 1 ))
	cd "$TESTS_DIR/$test_dir"
	echo "Running $(basename $test_dir)..."
	if [ -e README -a $verbose -ne 0 ]; then
		echo "$(cat README | fold -sw 77 | sed 's/.*/   &/' )"
	fi
	file2brl -f ./styles.cfg input.xml output.txt 2>/dev/null
	if [ $? -ne 0 ]; then
		error=$(( error + 1 ))
		if [ $colors -ne 0 ]; then
			echo "   \033[1m\033[91mERROR\033[0m\033[0m"
		else
			echo "   ERROR"
		fi
	else
		diff -q expected.txt output.txt >/dev/null
		if [ $? -ne 0 ]; then
			if [ -e xfail ]; then
				xfail=$(( xfail + 1 ))
				if [ $colors -ne 0 ]; then
					echo "   \033[1m\033[93mXFAIL\033[0m\033[0m"
				else
					echo "   XFAIL"
				fi
			else
				fail=$(( fail + 1 ))
				if [ $colors -ne 0 ]; then
					echo "   \033[1m\033[91mFAIL\033[0m\033[0m"
				else
					echo "   FAIL"
				fi
			fi
		else
			if [ -e xfail ]; then
				xpass=$(( xpass + 1 ))
				if [ $colors -ne 0 ]; then
					echo "   \033[1m\033[91mXPASS\033[0m\033[0m"
				else
					echo "   XPASS"
				fi
			else
				pass=$(( pass + 1 ))
				rm output.txt
				if [ $colors -ne 0 ]; then
					echo "   \033[1m\033[92mPASS\033[0m\033[0m"
				else
					echo "   PASS"
				fi
			fi
		fi
	fi
	rm -f file2brl.temp
done

echo ""
echo "Summary:"
echo "--------"
echo "Tests run: $tests, Failures: $((fail + xpass)), Expected failures: $xfail, Errors: $error"
echo ""

[ $(( error + fail + xpass )) -gt 0 ] && exit 1
exit 0
