#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <atomic>
using namespace std;

// Global variables for thread synchronization
int count = 1;
int lock = 0;

// Function to perform matrix multiplication for a chunk of rows
void cas(vector<vector<int>> v, int n, vector<vector<int>> &vec, int rowinc)
{
    while (true)
    {
        // Spinlock - Wait until the lock is acquired
        while (__sync_val_compare_and_swap(&lock, 0, 1))
            ;

        if (count > n)
        {
            // Release the lock and exit the loop if processing is complete
            lock = 0;
            break;
        }

        // Update start and end indices for the assigned chunk of rows
        int start = count;
        count = count + rowinc;
        lock = 0;
        int end = start + rowinc;
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
    // Input variables
    int n, k, rowinc;

    // Read matrix size and thread count from input file
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

    // Timing variables
    struct timeval s, e;
    gettimeofday(&s, NULL);

    // Create and run threads for matrix multiplication using vector
    vector<thread> threads;
    for (int i = 0; i < k; i++)
    {
        threads.push_back(thread(cas, v, n, ref(vec), rowinc));
    }

    // Wait for all threads to finish
    for (auto &t : threads)
    {
        t.join();
    }

    gettimeofday(&e, NULL);

    // Calculate and print the elapsed time
    double t;
    t = (e.tv_sec - s.tv_sec) + ((e.tv_usec - s.tv_usec) / 1000000.0);

    // Write the result matrix and time to output file
    ofstream output("out2.txt");
    for (auto it : vec)
    {
        for (auto d : it)
        {
            output << d << " ";
        }
        output << endl;
    }
    output << "time taken:" << t << " seconds" << endl;
    output.close();
}
