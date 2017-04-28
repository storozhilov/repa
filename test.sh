#!/usr/bin/env bash

set -e

echo "Testing sr"

script_dir=$(dirname $0)
if [ "${script_dir:0:1}" != "/" ] ; then
	script_dir=$(pwd)/${script_dir}
fi

set -x

${script_dir}/sr
