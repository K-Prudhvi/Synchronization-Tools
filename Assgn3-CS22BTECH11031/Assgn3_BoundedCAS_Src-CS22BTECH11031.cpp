#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <atomic>
using namespace std;

// Global variables for thread synchronizationg++ Assgn3_ATOMIC_Src-CS22BTECH11031.cpp
int count = 1;
int lock = 0;

// Function to perform matrix multiplication for a chunk of rows using the Bounded CAS algorithm
void bcas(vector<vector<int>> v, int n, int k, vector<vector<int>> &vec, vector<bool> &waiting, int rowinc, int i)
{
    while (true)
    {
        // Mark the current thread as waiting
        waiting[i] = true;
        bool key = true;

        // Spinlock: Wait until it's this thread's turn to enter the critical section
        while (waiting[i] && key)
        {
            key = __sync_val_compare_and_swap(&lock, 0, 1);
        }

        // Reset waiting status for the current thread
        waiting[i] = false;

        // Exit the loop if matrix multiplication is complete
        if (count > n)
        {
            lock = 0;
            break;
        }

        // Update start and end indices for the assigned chunk of rows
        int start = count;
        count = count + rowinc;
        lock = 0;

        // Find the next waiting thread in the circular queue
        int j = (i + 1) % k;
        while ((j != i) && !waiting[j])
        {
            j = (j + 1) % k;
        }

        // Release the lock if the next waiting thread is not found
        if (j == i)
        {
            lock = 0;
        }
        else
        {
            // Release the next waiting thread
            waiting[j] = false;
        }

        // Calculate the end index for the assigned rows
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

    // Read matrix size, thread count, and row increment from the input file
    ifstream input("inp.txt");
    input >> n >> k >> rowinc;

    // Initialize matrices for input and output
    vector<vector<int>> v;
    vector<vector<int>> vec(n, vector<int>(n));

    int in;

    // Read the input matrix from the file
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

    // Timing variables
    struct timeval s, e;
    gettimeofday(&s, NULL);

    // Create and run threads for matrix multiplication using the Bounded CAS algorithm
    vector<thread> threads;
    vector<bool> waiting(k, false);  // Initialize the waiting vector with false
    for (int i = 0; i < k; i++)
    {
        threads.push_back(thread(bcas, v, n, k, ref(vec), ref(waiting), rowinc, i));
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

    // Write the result matrix and time to the output file
    ofstream output("out3.txt");
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
