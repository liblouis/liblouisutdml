#!/bin/bash

# run_test
#
# Copyright (C) 2013,2016,2019 by Swiss Library for the Blind, Visually Impaired and Print Disabled
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

# Create a temporary directory $tmp_dir in $TMPDIR (default /tmp).
: ${TMPDIR=/tmp}
{
    tmp_dir=`(umask 077 && mktemp -d "$TMPDIR/fooXXXXXX") 2>/dev/null` && test -d "$tmp_dir"
} || exit $?

find-up () {
    current=$(pwd)
    path=$current
    while [[ "$path" != "/" && ! -e "$path/$1" ]]; do
	path=$(dirname -- "$path")
    done
    if [ "$path" != "/" ]; then
	echo "$path/$1"
    else
	>&2 echo "No file $1 found in parent directories of $current"
	echo ""
    fi
}

run_test () {
    test_dir=$1
    cd "$test_dir"
    README=$(find-up README)
    input=$(find-up input.xml)
    styles=$(find-up styles.cfg)
    expected=$(find-up expected.txt)

    if [[ $README == "" || $input == "" || $styles == "" || $expected == "" ]]; then
	exit 1
    fi

    file2brl -w $tmp_dir -f $styles $input $tmp_dir/output.txt 2> /dev/null
    if [ $? -ne 0 ]; then
	exit 99
    else
	diff -q $expected $tmp_dir/output.txt >/dev/null
	if [ $? -ne 0 ]; then
	    if [ -n "$VERBOSE" ]; then
		cat $README
		echo "Diff: " >&2
		diff -u $expected $tmp_dir/output.txt
	    fi
	    exit 1
	else
	    rm $tmp_dir/output.txt
	    exit 0
	fi
    fi
    rm -f file2brl.temp
    rm -rf $tmp
}

run_test $1
