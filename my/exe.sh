name=${1:-MyEngine}
g++ -g -o $name $name.cpp -std=c++11
gdb ./$name