#include "app.h"
#include <stdexcept>
#include <iostream>


int main()
{

	try 
	{
		App* app = new App();
		app->run();
		delete app;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}