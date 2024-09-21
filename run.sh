clear
echo "run the project"
cd build
cmake .. && make && ./httpserver $1