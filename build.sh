#!/usr/bin/env bash

set -e

echo "Building Repa"

script_dir=$(dirname $0)
if [ "${script_dir:0:1}" != "/" ] ; then
	script_dir=$(pwd)/${script_dir}
fi

sources="$(ls ${script_dir}/src/*.cpp)"
libraries="-lasound -lboost_program_options -lboost_system -lboost_thread"

set -x

g++ ${sources} ${libraries} -o ${script_dir}/repa
