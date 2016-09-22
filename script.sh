make clean
make

./trace ./data/scene5c.scd out5c.png
./trace ./data/scene3c.scd out3c.png

cd ./data
../trace teapot.scd ../teapot.png
../trace dunkit.scd ../dunkit.png