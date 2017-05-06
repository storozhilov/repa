#!/usr/bin/env bash

set -e

echo "Testing SR"

script_dir=$(dirname $0)
if [ "${script_dir:0:1}" != "/" ] ; then
	script_dir=$(pwd)/${script_dir}
fi

set -x

${script_dir}/repa --help

location=$(mktemp -d)

#${script_dir}/repa -D multi_capture -O "${location}" &
#${script_dir}/repa -D multi_capture &
${script_dir}/repa -O "${location}" &
repa_pid=$!

sleep 1
kill -TERM ${repa_pid}

wait ${repa_pid}
repa_exit_code=$?

rm -Rf "${location}"

exit ${repa_exit_code}
