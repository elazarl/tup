#! /bin/sh -e

# Basic foreach rule

. ./tup.sh
cat > Tupfile << HERE
: foreach *.c |> echo gcc -c %f -o %B.o |>
HERE
tup touch foo.c bar.c Tupfile
update
tup_object_exist . foo.c bar.c
tup_object_exist . "echo gcc -c foo.c -o foo.o"
tup_object_exist . "echo gcc -c bar.c -o bar.o"

eotup
