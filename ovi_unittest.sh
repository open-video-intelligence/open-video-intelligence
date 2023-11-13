#!/bin/bash

BUILD_DIR=build
TEST_DIR=tests/unittest

TESTNAME="*"
TESTCASE="*"

if [ $# -eq 1 ]; then
	TESTNAME=$1
elif [ $# -eq 2 ]; then
	TESTNAME=$1
	TESTCASE=$2
fi

#./ovi_build.sh

cd ${BUILD_DIR} || exit

# pre-condition
for filename in ../${TEST_DIR}/resource/*; do
echo ${filename##*/}
cp ${filename} .
done;

cp fd_hide.mp4 movie_noaccess.mp4
chmod 000 movie_noaccess.mp4

# run test
./${TEST_DIR}/ovi_ut --gtest_filter=${TESTNAME}.${TESTCASE} --gtest_output=xml

# post
for filename in ../${TEST_DIR}/resource/*; do
rm -f ${filename##*/}
done;
chmod 644 movie_noaccess.mp4
rm -f movie_noaccess.mp4
cd - || exit
