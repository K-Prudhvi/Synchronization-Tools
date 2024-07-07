#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <atomic>

using namespace std;

// Global variables
int count = 1;              // Counter for row allocation
atomic_flag lock = ATOMIC_FLAG_INIT;  // Atomic flag for thread synchronization

// Function to perform matrix multiplication for rows
void tas(vector<vector<int>> v, int n, vector<vector<int>> &vec, int rowinc)
{
    while (true)
    {
        // Acquire lock using atomic flag
        while (lock.test_and_set())
            ;

        // Check if all rows are processed
        if (count > n)
        {
            // Release lock and exit the loop
            lock.clear();
            break;
        }

        // Allocate rows to the current thread
        int start = count;
        count = count + rowinc;
        lock.clear();
        int end = start + rowinc;

        // Adjust end to prevent out-of-bounds access
        if (end > n)
            end = n + 1;

        // Iterate over the assigned rows in the chunk
        for (int i = start; i < end; i++)
        {
            int m = i - 1;

            // Matrix multiplication for the assigned row
            for (int l = 0; l < n; l++)
            {
                int h = 0;
                for (int j = 0; j < n; j++)
                {
                    h = h + (v[m][j] * v[j][l]);
                }
                vec[m][l] = h;
            }
        }
    }
}

int main()
{
    int n, k, rowinc;

    // Read matrix size, thread count, and row increment from input file
    ifstream input("inp.txt");
    input >> n >> k >> rowinc;

    // Initialize matrices for input and output
    vector<vector<int>> v;
    vector<vector<int>> vec(n, vector<int>(n));

    int in;

    // Read input matrix from file
    for (int i = 0; i < n; i++)
    {
        vector<int> temp;
        for (int j = 0; j < n; j++)
        {
            input >> in;
            temp.push_back(in);
        }
        v.push_back(temp);
    }
    input.close();

    // Calculate the number of rows each thread will handle
    int p = rowinc;
    
    // Measure the start time
    struct timeval s, e;
    gettimeofday(&s, NULL);

    // Create and run threads for matrix multiplication using vector
    vector<thread> threads;
    for (int i = 0; i < k; i++)
    {
        threads.push_back(thread(tas, v, n, ref(vec), rowinc));
    }

    // Wait for all threads to finish
    for (auto &t : threads)
    {
        t.join();
    }

    // Measure the end time
    gettimeofday(&e, NULL);

    // Calculate and print the elapsed time
    double t;
    t = (e.tv_sec - s.tv_sec) + ((e.tv_usec - s.tv_usec) / 1000000.0);

    // Write the result matrix and time to output file
    ofstream output("out1.txt");
    for (auto it : vec)
    {
        for (auto d : it)
        {
            output << d << " ";
        }
        output << endl;
    }
    output << "time taken: " << t << " seconds" << endl;
    output.close();
}
