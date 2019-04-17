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
    local current=$(pwd)
    local path=$current
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

cleanup_and_exit () {
    local status=$1
    local message=$2
    if [[ $message != "" ]]; then
	>&2 echo "$message"
    fi
    rm -rf $tmp_dir
    exit $status
}

run_test () {
    local test_dir=$1
    cd "$test_dir"
    local README=$(find-up README)
    local input=$(find-up input.xml)
    local styles=$(find-up styles.cfg)
    local expected=$(find-up expected.txt)
    local ini_file=$(find-up liblouisutdml.ini)

    if [[ $README == "" || $input == "" || $styles == "" || $expected == "" || $ini_file == "" ]]; then
	cleanup_and_exit 1 "Couldn't find README, input.xml, styles.cfg, expected.txt or liblouisutdml.ini"
    fi

    if [[ $(dirname $styles) != $(dirname $ini_file) ]]; then
	cleanup_and_exit 2 "$styles and $ini_file need to be in the same directory for the test to work."
    fi

    # the liblouisutdml.ini file contains paths which are relative to
    # that file, so we need to start file2brl from there to make the
    # relative paths work
    (cd $(dirname $ini_file);
     file2brl -w $tmp_dir -f $styles $input $tmp_dir/output.txt)
    if [ $? -ne 0 ]; then
	cleanup_and_exit 99 "Invocation of file2brl failed with error code $?"
    else
	diff -q $expected $tmp_dir/output.txt >/dev/null
	if [ $? -ne 0 ]; then
	    if [ -n "$VERBOSE" ]; then
		cat $README
		echo "Diff: " >&2
		diff -u $expected $tmp_dir/output.txt
	    fi
	    cleanup_and_exit 1
	else
	    cleanup_and_exit 0
	fi
    fi
    # shouldn't really reach this code
    cleanup_and_exit 1
}

run_test $1
