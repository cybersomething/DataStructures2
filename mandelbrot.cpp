//Credit to Adam Sampson for mandelbrot algorithm

//Original timing = 81615ms
//Split up time = 20778ms

//Credit to Adam Sampson for mandelbrot algorithm
 
//Import required libraries
 
//Required for time measurement
#include <chrono>
 
//Required for general purpose functions and fixed integer types
#include <cstdint>
#include <cstdlib>
 
//Required for numeric types such as float and double
#include <complex>
 
//Required for input and outputs
#include <fstream>
#include <iostream>
 
//Required for multi-threading mechanisms
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
 
//Required for storage during runtime
#include <vector>
 
//Define the alias "the_clock" for the clock type we're going to use.
typedef std::chrono::steady_clock the_clock;
 
//Declaring the default number of threads to use in the event a user does not specifiy the expected range
const unsigned int DEFAULT_NUM_THREADS = std::thread::hardware_concurrency();
 
//The variable to store the number of threads entered by the user within a given range
unsigned int NUM_THREADS = 0;
 
const int WIDTH = 480;
const int HEIGHT = 320;
 
//The number of times to iterate before we assume that a point isn't in the
//Mandelbrot set.
const int MAX_ITERATIONS = 500;
 
//The image data.
//Each pixel is represented as 0xRRGGBB.
uint32_t image[HEIGHT][WIDTH];
 
int firstColour = 0;
int secondColour = 0;
 
//Create vector to store threads
std::vector<std::thread> thread_pool;
 
//Create conditional variable and mutex for the output thread
std::condition_variable mandel_cv;
std::mutex mandel_mutex;
 
//Create integer variable to handle output branching
std::atomic<int> num_threads = 0;
 
//Write the image to a TGA file with the given name.
//Format specification: http://www.gamers.org/dEngine/quake3/TGA.txt
void fileWrite(const char* filename)
{
    std::ofstream outfile(filename, std::ofstream::binary);
 
    uint8_t header[18] = {
         0, //No image ID
         0, //No colour map
         2, //Uncompressed 24-bit image
         0, 0, 0, 0, 0, //Empty colour map specification
         0, 0, //X Origin
         0, 0, //Y Origin
         WIDTH & 0xFF, (WIDTH >> 8) & 0xFF, //Width
         HEIGHT & 0xFF, (HEIGHT >> 8) & 0xFF, //Height
         24, //Bits per pixel
         0, //Image descriptor
    };
    outfile.write((const char*)header, 18);
 
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            uint8_t pixel[3] = {
                image[y][x] & 0xFF, //Blue channel
                (image[y][x] >> 8) & 0xFF, //Green channel
                (image[y][x] >> 16) & 0xFF, //Red channel
            };
            outfile.write((const char*)pixel, 3);
        }
    }
 
    outfile.close();
    if (!outfile)
    {
        //An error has occurred at some point since we opened the file.
        std::cout << "Error writing to " << filename << std::endl;
        exit(1);
    }
}
 
void mandel_consumer(int num_cpu) {
    
    //Block thread until all other threads have finished computing their Mandelbrot slice
    std::unique_lock<std::mutex> lck(mandel_mutex);
    while (num_threads != num_cpu) {
        mandel_cv.wait(lck);
    }
    fileWrite("output.tga"); //Function call will only occur once the while loop exits (thread is unblocked)
}
 
//Render the Mandelbrot set into the image array.
//The parameters specify the region on the complex plane to plot.
void compute_mandelbrot(double left, double right, double top, double bottom, unsigned y_start, unsigned y_stop)
{
    for (int y = y_start; y < y_stop; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            //Work out the point in the complex plane that corresponds to this pixel in the output image.
            std::complex<double> c(left + (x * (right - left) / WIDTH),
                top + (y * (bottom - top) / HEIGHT));
 
            //Start off z at (0, 0).
            std::complex<double> z(0.0, 0.0);
 
            //Iterate z = z^2 + c until z moves more than 2 units away from (0, 0), or we've iterated too many times.
            int iterations = 0;
            while (abs(z) < 2.0 && iterations < MAX_ITERATIONS)
            {
                z = (z * z) + c;
                ++iterations;
            }
 
            if (iterations == MAX_ITERATIONS)
            {
                //Z didn't escape from the circle. This point is in the Mandelbrot set.
                image[y][x] = firstColour; //Black
            }
            else
            {
                //Z escaped within less than MAX_ITERATIONS iterations. This point isn't in the set.
                image[y][x] = secondColour; //White
            }
        }
    }
    num_threads++;
    mandel_cv.notify_one();
}
 
int main(int argc, char* argv[]) {
 
    //Creating values used to compute the Mandelbrot
    double man_left = -2.0;
    double man_right = 1.0;
    double man_top = 1.125;
    double man_bottom = -1.125;
 
    //Prompt user for first colour selection
    std::cout << "1. Red" << std::endl;
    std::cout << "2. Yellow" << std::endl;
    std::cout << "3. Orange" << std::endl;
    std::cout << "Please enter your first colour choice: ";
 
   //Perform input validation on user input
    std::cin >> firstColour;
    while (firstColour < 1 || firstColour > 3 || std::cin.fail()) {
        std::cin.clear(); //Clean the input stream
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Flush the input stream
        std::system("CLS"); //Clear terminal window
        std::cout << "Incorrect input given, please ensure input is between 1-3 and that it's of a number." << std::endl;
        std::cout << "1. Red" << std::endl;
        std::cout << "2. Yellow" << std::endl;
        std::cout << "3. Orange" << std::endl;
        std::cout << "Please enter your first colour choice: ";
        std::cin >> firstColour;
    }
 
    std::system("CLS"); //Clear terminal window
 
    //Determine first colour based on user input
    switch (firstColour)
    {
    case 1: firstColour = 0xFF0000; //Red
        break;
    case 2: firstColour = 0xFFFF00; //Yellow
        break;
    case 3: firstColour = 0xFFA500; //Orange
        break;
    }
 
    //Prompt user for second colour selection
    std::cout << "1. Blue" << std::endl;
    std::cout << "2. Green" << std::endl;
    std::cout << "3. Purple" << std::endl;
    std::cout << "Please enter your second colour choice: ";
 
    //Perform input validation on user input
    std::cin >> secondColour;
    while (secondColour < 1 || secondColour > 3 || std::cin.fail()) {
        std::cin.clear(); //Clean the input stream
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Flush the input stream
        std::system("CLS"); //Clear terminal window
        std::cout << "Incorrect input given, please ensure input is between 1-3 and that it's of a number." << std::endl;
        std::cout << "1. Blue" << std::endl;
        std::cout << "2. Green" << std::endl;
        std::cout << "3. Purple" << std::endl;
        std::cout << "Please enter your second colour choice: ";
        std::cin >> secondColour;
    }
 
    std::system("CLS"); //Clear terminal window
 
    //Determine second colour based on user input
    switch (secondColour)
    {
    case 1: secondColour = 0x0000FF; //Blue
        break;
    case 2: secondColour = 0x00FF00; //Green
        break;
    case 3: secondColour = 0x6a0dad; //Purple
        break;
    }
 
    //Prompt user for number of threads to use
    std::cout << "Maximum threads available: " << DEFAULT_NUM_THREADS << std::endl << std::endl;
    std::cout << "Please enter the amount of threads to use in the Mandelbrot computation: ";
    
    //Perform input validation on user input
    std::cin >> NUM_THREADS;
    if (NUM_THREADS < 0 || std::cin.fail()) {
        std::cin.clear(); //Clean the input stream
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Flush the input stream
        std::system("CLS"); //Clear the terminal window
        std::cout << "Invalid input detected, please ensure " << std::endl;
        std::cout << "Maximum threads available: " << DEFAULT_NUM_THREADS << std::endl << std::endl;
        "Please enter the amount of threads to use in the Mandelbrot computation: ";
        std::cin >> NUM_THREADS;
    }
 
    std::system("CLS"); //Clear the terminal window
 
    //Use maximum amount of threads available on the system when user input is out of bounds of expected input
    if (NUM_THREADS == 0 || NUM_THREADS > DEFAULT_NUM_THREADS) {
        NUM_THREADS = DEFAULT_NUM_THREADS;
    }
 
    std::cout << "Defaulting to maximum amount of threads: " << DEFAULT_NUM_THREADS;
    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); //Sleep for 3 seconds (allow user to read message above)
 
    std::system("CLS"); //Clear terminal window
 
    std::cout << "Please wait..." << std::endl;
 
    //Start timing
    the_clock::time_point start = the_clock::now();
 
    //Start consumer thread that will be responsible for handling Mandelbrot output
    std::thread mandel_consumer_thread(mandel_consumer, NUM_THREADS);
 
    //Create variables to manage the position of each thread on the Mandelbrot set
    unsigned int mandel_current_place = 0;
    unsigned int mandel_step = HEIGHT / NUM_THREADS;
 
    //Carry out dirty compensation for odd divisions
    for (int i = 0; i < NUM_THREADS; i++) {
        if (i == NUM_THREADS - 1) {
            mandel_step = 0;
            mandel_current_place = HEIGHT - 1;
        }
        //Create producer threads responsible for computing their own slices of the Mandelbrot set
        thread_pool.push_back(std::thread(compute_mandelbrot, man_left, man_right, man_top, man_bottom, mandel_current_place, mandel_current_place + mandel_step));
        mandel_current_place += mandel_step;
    }
 
    //Iterate through the thread pool and synchronize all the threads
    for (std::thread& thread : thread_pool) {
        thread.join();
    }
 
    //Synchronize the consumer thread
    mandel_consumer_thread.join();
 
    //Stop timing
    the_clock::time_point end = the_clock::now();
 
    //Compute the difference between the two times in milliseconds
    auto time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Computing the Mandelbrot set took " << time_taken << " ms." << std::endl;
 
    return 0;
}
