.PHONY: clean

all: lib/libxed.a

lib/libxed.a:
	mkdir -p xed/build
	xed/xed/mfile.py --jobs=4 --build-dir=xed/build/obj --src-dir=xed/xed --static --opt=3 --prefix=. --install-dir=xed/kits/xed-install-date-os-cpu --no-amd --no-via --no-encoder --compress-operands install

clean:
	rm -Rf include lib xed/build xed/kits