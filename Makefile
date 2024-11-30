all:
	g++ -std=c++17 -I./sciplot -O3 -fdiagnostics-color=always -g ./BloomFilter.cpp -o ./BloomFilter