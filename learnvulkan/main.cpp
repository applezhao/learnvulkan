

#include "BasicDrawTri.h"
#include <iostream>
using namespace std;
int main()
{
	BasicDrawTri drawApp;
	try
	{
		drawApp.run();
	}
	catch (const runtime_error& e)
	{
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}