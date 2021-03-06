#! /bin/sh -e

# Try extra-outputs with a !-macro. The shell script mimics a Windows linker.

. ./tup.sh

echo 'out=$1; base=`basename $out .dll`; shift; cat $* > $out; touch $base.lib $base.exp' > ok.sh
chmod +x ok.sh

cat > Tupfile << HERE
!cc = |> gcc -c %f -o %o |> %B.o
!ld = |> ./ok.sh %o %f |> | %O.lib %O.exp

: foreach *.c |> !cc |> {objs}
: {objs} |> !ld |> out.dll
HERE
tup touch Tupfile foo.c bar.c ok.sh
update

eotup
