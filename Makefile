clean:
	rm -rf ./bin/* ./build/*

build:
	pushd build
	cmake ..
	make
	popd
