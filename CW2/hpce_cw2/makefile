SRC_URL = http://cas.ee.ic.ac.uk/people/dt10/teaching/2013/hpce/cw2

packages/sox-14.4.1.tar.gz :
	curl $(SRC_URL)/sox-14.4.1.tar.gz -o packages/sox-14.4.1.tar.gz

build/sox-14.4.1/configure : packages/sox-14.4.1.tar.gz
	cd build && tar -xzf ../packages/sox-14.4.1.tar.gz

build/sox-14.4.1/make : build/sox-14.4.1/configure
	cd build/sox-14.4.1 && ./configure --prefix=`pwd`/../../local

build/sox-14.4.1/src/sox : build/sox-14.4.1/make
	cd build/sox-14.4.1 && make -j 12

local/bin/sox : build/sox-14.4.1/src/sox
	cd build/sox-14.4.1 && make install

packages/lame-3.99.5.tar.gz :
	curl $(SRC_URL)/lame-3.99.5.tar.gz -o packages/lame-3.99.5.tar.gz

build/lame-3.99.5/configure : packages/lame-3.99.5.tar.gz
	cd build && tar -xzf ../packages/lame-3.99.5.tar.gz

build/lame-3.99.5/make : build/lame-3.99.5/configure
	cd build/lame-3.99.5 && ./configure --prefix=`pwd`/../../local

build/lame-3.99.5/src/lame : build/lame-3.99.5/make
	cd build/lame-3.99.5 && make -j 12

local/bin/lame : build/lame-3.99.5/src/lame
	cd build/lame-3.99.5 && make install

audiomake :
	cd audio && make tools


clean : 
	rm packages/*
	rm -rf build/*
	rm -rf local/*

tools : local/bin/lame local/bin/sox audiomake