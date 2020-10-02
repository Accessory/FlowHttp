#include "HPack.h"
#include "HPackTable.h"
#include <iostream>

int main(int argc, char *argv[]) {
    std::string toEncode = ":status: 200\n\r"
                                 "cache-control: private\n\r"
                                 "date: Mon, 21 Oct 2013 20:13:21 GMT\n\r"
                                 "location: https://www.example.com\n\r";
    auto data = HPack::encode(toEncode, true);
    std::cout << std::string(data.begin(), data.end()) << std::endl;
    std::cout << HPack::decode(data);
}