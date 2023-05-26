rmdir /s /q build
mkdir build
pushd build
cmake ..
cmake --build .
start segment-tree.sln
popd