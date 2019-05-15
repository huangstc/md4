#include "md4.h"
#include "crack.h"

#include <iostream>

int main(int argc, char *argv[]) {
	int rsd = 0;
	std::cout << "Input a seed : ";
	std::cin >> rsd;
	run(rsd, 0x40000000); // 416 428
	return 0;
}
