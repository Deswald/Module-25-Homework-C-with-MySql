#include <iostream>
#include "chat.hpp"


int __cdecl main(int argc, char** argv)
{

	std::cout << "CLIENT IS ESTABLISHING A CONNECTION WITH SERVER THROUGH PORT: " << PORT << std::endl;
	chat(argc, argv);

	return 0;

}