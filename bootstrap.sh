#! /bin/sh -e
os=`uname -s`
plat_cflags=""
plat_ldflags=""
plat_files=""
plat_preloadcflags="-fpic"
plat_preloadldflags="-shared"
case "$os" in
	Linux)
	plat_files="$plat_files ../src/compat/dummy.c"
	;;
	SunOS)
	plat_files="$plat_files ../src/compat/dir_mutex.c"
	plat_files="$plat_files ../src/compat/mkdirat.c"
	plat_files="$plat_files ../src/compat/readlinkat.c"
	plat_ldflags="$plat_ldflags -lsocket"
	plat_cflags="-D_REENTRANT"
	;;
	Darwin)
	plat_files="$plat_files ../src/compat/dir_mutex.c "
	plat_files="$plat_files ../src/compat/fstatat.c"
	plat_files="$plat_files ../src/compat/mkdirat.c"
	plat_files="$plat_files ../src/compat/openat.c"
	plat_files="$plat_files ../src/compat/readlinkat.c"
	plat_files="$plat_files ../src/compat/unlinkat.c"
	plat_cflags="-DAT_SYMLINK_NOFOLLOW=0x100"
	plat_preloadcflags="-DAT_FDCWD=-100 -fPIC"
	plat_preloadldflags="-dynamiclib"
	;;
esac

rm -rf build
echo "  mkdir build"
mkdir -p build
mkdir -p build/ldpreload
echo "  cd build"
cd build
for i in ../src/linux/*.c ../src/tup/*.c ../src/tup/tup/main.c ../src/tup/access_event/send_event.c ../src/tup/monitor/null.c ../src/tup/colors/colors.c ../src/tup/flock/fcntl.c ../src/tup/server/socketpair.c $plat_files; do
	echo "  bootstrap CC (unoptimized) $i"
	gcc -c $i -I../src $plat_cflags
done

echo "  bootstrap CC (unoptimized) ../src/sqlite3/sqlite3.c"
gcc -c ../src/sqlite3/sqlite3.c -DSQLITE_TEMP_STORE=2 -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION $plat_cflags

echo "  bootstrap CC (unoptimized) ../src/ldpreload/ldpreload.c [ldpreload]"
gcc -c ../src/ldpreload/ldpreload.c -o ldpreload/ldpreload.o $plat_preloadcflags $plat_cflags -I../src
echo "  bootstrap CC (unoptimized) ../src/tup/access_event/send_event.c [ldpreload]"
gcc -c ../src/tup/access_event/send_event.c -o ldpreload/send_event.o $plat_preloadcflags $plat_cflags -I../src
echo "  bootstrap CC (unoptimized) ../src/tup/flock/fcntl.c [ldpreload]"
gcc -c ../src/tup/flock/fcntl.c -o ldpreload/fcntl.o $plat_preloadcflags $plat_cflags -I../src

echo "  bootstrap LD.so tup-ldpreload.so"
gcc $plat_preloadcflags $plat_preloadldflags -o tup-ldpreload.so ldpreload/ldpreload.o ldpreload/send_event.o ldpreload/fcntl.o -ldl $plat_ldflags

echo "  bootstrap LD tup"
echo "const char *tup_version(void) {return \"bootstrap\";}" | gcc -x c -c - -o tup_version.o
gcc *.o -o tup -lpthread $plat_ldflags

cd ..
# We may be bootstrapping over an already-inited area.
./build/tup init || true
./build/tup upd
echo "Build complete. If ./tup works, you can remove the 'build' directory."
