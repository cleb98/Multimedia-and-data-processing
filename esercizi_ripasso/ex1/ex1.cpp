/*ex1*/

#include <iostream>
#include <iomanip>

int main() {

	double num;
	std::cout << "Insert number:\n";
	std::cin >> num;
	std::cout << "\nyou have inserted " << num << "\n";
	double sq = num * num;
	std::cout << num << " at power of 2 is: " << std::fixed << std::setprecision(3) << sq << "\n";
	return 0;

}


