#! /bin/sh -e

. ./tup.sh
tmkdir a
tmkdir a/a2
cp ../testTupfile.tup a/a2/Tupfile

echo "int main(void) {return 0;}" > a/a2/foo.c
tup touch a/a2/foo.c a/a2/Tupfile
update
tup_object_exist . a
tup_object_exist a a2
tup_object_exist a/a2 foo.c foo.o prog
sym_check a/a2/foo.o main
sym_check a/a2/prog main

rm -rf a
tup rm a
update
tup_object_no_exist . a
tup_object_no_exist a a2
tup_object_no_exist a/a2 foo.c foo.o prog

eotup
