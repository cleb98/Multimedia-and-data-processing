#include <string>
#include <iostream>

int main() {
    const std::string input = "abc def ghi"; // Input string
    const char delimiter = ' ';             // Delimiter to split the string

    // Find the position of the first delimiter
    auto pos = input.find(delimiter);

    if (pos != std::string::npos) {
        // If delimiter is found, print the substring
        std::cout << input.substr(0, pos) << std::endl;
    } else {
        // Handle the case where no delimiter is found
        std::cout << "No delimiter found in the string." << std::endl;
    }

    return 0;
}