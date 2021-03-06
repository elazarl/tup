#! /bin/sh -e

# Check that static binning with a varsed rule works.

. ./tup.sh
check_no_windows varsed
cat > Tupfile << HERE
: foo.txt |> tup varsed %f %o |> out.txt {txt}
: foreach {txt} |> cp %f %o |> %B.copied
HERE
echo "hey @FOO@ yo" > foo.txt
tup touch foo.txt Tupfile
varsetall FOO=sup
update

echo "hey sup yo" | diff out.copied -
check_not_exist foo.copied

eotup
