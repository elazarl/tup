#! /bin/sh -e

. ./tup.sh

for i in db object shared tri vardict; do
	if [ ! -f ".tup/$i" ]; then
		echo ".tup/$i not created!" 1>&2
		exit 1
	fi
done

eotup
