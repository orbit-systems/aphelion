#!/usr/bin/env bash

set -euo pipefail

LUNA="./bin/luna"

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

fails=0
skips=0
passes=0
tests=0

maxname=0
for src in tests/*.s; do
    name=${src##*/}
    name=${name%.s}

    ((tests++))

    if (( ${#name} > maxname )); then
        maxname=${#name}
    fi
done

for src in tests/*.s; do
    name=${src##*/}
    name=${name%.s}

    expected="tests/${name}.out"
    out="$TMPDIR/${name}.out"

    if [[ ! -f "$expected" ]]; then
        ((skips++))
	printf "Skip: %-*s (missing %s)\n" "$maxname" "$name" "$expected"
        continue
    fi

    if ! "$LUNA" "$src" -o "$out" 2>/dev/null; then
        ((fails++))
	printf "Fail: %-*s (runtime error)\n" "$maxname" "$name"
        continue
    fi

    if cmp -s "$expected" "$out"; then
        ((passes++))
        echo "Pass: $name"
    else
        ((fails++))
	printf "Fail: %-*s (mismatch)\n" "$maxname" "$name"
    fi
done

echo
printf "tests failed:  %3d/%-3d\n" "$fails"  "$tests"
printf "tests skipped: %3d/%-3d\n" "$skips"  "$tests"
printf "tests passed:  %3d/%-3d\n" "$passes" "$tests"

# if all tests pass return 0, else exit with error
((tests == passes))
