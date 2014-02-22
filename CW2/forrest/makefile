SRC_URL = http://cas.ee.ic.ac.uk/people/dt10/teaching/2013/hpce/cw2

tools: local/bin/sox local/bin/lame
clean: clean/sox clean/lame

# SOX MAKE
packages/sox-14.4.1.tar.gz:
	curl $(SRC_URL)/sox-14.4.1.tar.gz -o packages/sox-14.4.1.tar.gz

build/sox-14.4.1/configure: packages/sox-14.4.1.tar.gz
	cd build && tar -xzf ../packages/sox-14.4.1.tar.gz

build/sox-14.4.1/Makefile: build/sox-14.4.1/configure
	cd build/sox-14.4.1 && ./configure --prefix="`pwd`/../../local"

build/sox-14.4.1/src/sox: build/sox-14.4.1/Makefile
	cd build/sox-14.4.1 && $(MAKE)

local/bin/sox: build/sox-14.4.1/src/sox
	cd build/sox-14.4.1 && $(MAKE) install

clean/sox/uninstall:
	-cd build/sox-14.4.1 && $(MAKE) uninstall

clean/sox: clean/sox/uninstall
	-rm -rf build/sox-14.4.1
	-rm -rf packages/sox-14.4.1.tar.gz

# LAME MAKE
packages/lame-3.99.5.tar.gz:
	curl $(SRC_URL)/lame-3.99.5.tar.gz -o packages/lame-3.99.5.tar.gz

build/lame-3.99.5/configure: packages/lame-3.99.5.tar.gz
	cd build && tar -xzf ../packages/lame-3.99.5.tar.gz

build/lame-3.99.5/Makefile: build/lame-3.99.5/configure
	cd build/lame-3.99.5 && ./configure --prefix="`pwd`/../../local"

build/lame-3.99.5/src/lame: build/lame-3.99.5/Makefile
	cd build/lame-3.99.5 && $(MAKE)

local/bin/lame: build/lame-3.99.5/src/lame
	cd build/lame-3.99.5 && $(MAKE) install

clean/lame/uninstall:
	-cd build/lame-3.99.5 && $(MAKE) uninstall

clean/lame: clean/lame/uninstall
	-rm -rf build/lame-3.99.5
	-rm -rf packages/lame-3.99.5.tar.gz


