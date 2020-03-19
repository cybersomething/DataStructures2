// Race conditions example
// Adam Sampson <a.sampson@abertay.ac.uk>

#include <iostream>
#include <string>
#include "account.h"
#include <thread>

using std::thread;

// Import things we need from the standard library
using std::cout;
using std::endl;

Account bill;

void myThreadFunc(Account& bill)
{
		bill.add(17, 29);
		bill.total();
}

int main(int argc, char *argv[])
{
	cout << "Initial: " << bill.total() << "\n";

	thread myThread(myThreadFunc, bill);
	myThread.join();

	cout << "Total: " << bill.total() << "\n";

	return 0;
}
