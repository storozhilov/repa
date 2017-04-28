#!/usr/bin/env bash

set -e

echo "Building sr"

script_dir=$(dirname $0)
if [ "${script_dir:0:1}" != "/" ] ; then
	script_dir=$(pwd)/${script_dir}
fi

sources="$(ls ${script_dir}/src/*.cpp)"

set -x

g++ ${sources} -o ${script_dir}/sr