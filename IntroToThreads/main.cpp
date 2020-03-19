// Simple threading example
// Adam Sampson <a.sampson@abertay.ac.uk>

#include <chrono>
#include <iostream>
#include <thread>
#include <Windows.h>

// Import things we need from the standard library
using std::chrono::seconds;
using std::cout;
using std::endl;
using std::ofstream;
using std::this_thread::sleep_for;
using std::thread;

struct ThreadArgs
{
	int id = 0;
	int delay = 500;
};


void myThreadFunc(int answer, struct ThreadArgs* args) //We create a pointer which will point towards the memory address that holds 'args'
{
	cout << "The answer is : " << answer << endl;
	int sleep_counter = args->delay;
	for (int i = 0; i < 10; i++)
	{
		args->id = args->id + 1;
		Sleep(args->delay); 
		args->delay = args->delay + 100;
		cout << "The id is: " << args->id << "\n";
	}
	cout << "id is: " << (args->id) << std::endl;
}

//Note: When accessing members of a class/struct, it's important we access them using '->' rather than '.' since we're dealing with pointers.

int main(int argc, char* argv[])
{
	int answer = 40;

	// At the moment our program is only running one thread (the initial one the operating system gave us).
	ThreadArgs args; //Create an instance of ThreadArgs called 'args'
	thread myThread(myThreadFunc, answer, &args); //Pass the memory address of args to the thread that will work on it

	// Now our program is running two threads in parallel (the initial one, and myThread).

	cout << "I am main\n";

	// Wait for myThread to finish.
	myThread.join();

	// Now we just have the initial thread. So it's safe to exit.

	cout << "All done\n";

	return 0;
}
