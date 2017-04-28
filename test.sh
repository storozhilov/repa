#!/usr/bin/env bash

set -e

echo "Testing SR"

script_dir=$(dirname $0)
if [ "${script_dir:0:1}" != "/" ] ; then
	script_dir=$(pwd)/${script_dir}
fi

set -x

${script_dir}/sr &
sr_pid=$!

sleep 1
kill -TERM ${sr_pid}

wait ${sr_pid}
sr_exit_code=$?

exit ${sr_exit_code}
