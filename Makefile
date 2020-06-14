clean:
	rm -rf ./bin/* ./build/*

build:
	pushd build
	cmake .. -GNinja # or cmake ..
	make
	popd
