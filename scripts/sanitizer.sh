#!/bin/bash

binary_dir=${2:-bin}
checker_bin=./"$binary_dir"/find-type"$1"
failed=0

if [ ! -e "$checker_bin" ] ; then
  echo "Abort. Executable for sanitizer check not found at: " "$checker_bin"
  exit 1
fi

for test in ./test/collection/*.cpp ; do
  "$checker_bin" "$test" -- > /dev/null
  if [ ! $? -eq 0 ] ; then
    ((failed++))
  fi
done

if [ $failed -ge 1 ] ; then
  echo "# of failed tests:" $failed && exit 1
fi
