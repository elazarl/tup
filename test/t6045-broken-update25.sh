#! /bin/sh -e

# I stumbled across this circular dependency while trying to come up with
# cases that might break with the addition of dependency inheritance. The
# problem here is if we swap the commands, the link from 'foo' still points
# to the command that writes to bar (since we find commands based on their
# outputs, the command IDs actually swap). This link is demoted from a
# normal+sticky link to just a normal link, but since it is no longer sticky
# we bypass the check for it in the delete_tree.
#
# The fix is to have the link demotion also check if the link is in the
# delete_tree, and remove it entirely if it is. This problem ultimately came
# about as a result of trying to preserve as much of the database as possible
# when renaming commands and such (I used to wipe out the command entirely,
# which would avoid this issue, but also produced a lot of unnecessary db
# churn).

. ./tup.sh

cat > ok1.sh << HERE
cat foo
HERE

chmod +x ok1.sh

cat > Tupfile << HERE
: |> echo blah > %o |> foo
: foo |> ./ok1.sh > %o |> bar
HERE
update

# Note that ./ok1.sh still reads from 'foo', but that's ok since a command is
# allowed to try to read from a file and still write to it (writing takes
# precedence)
cat > Tupfile << HERE
: |> echo blah > %o |> bar
: bar |> ./ok1.sh > %o |> foo
HERE

tup touch Tupfile
update

eotup
