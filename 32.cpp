#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <omp.h>

const long long N = 100000000;
const long long STUDENT_ID = 431512;
const long long BLOCK_SIZE = 10 * STUDENT_ID;

double computePiOpenMP(int numThreads, long long& elapsedMs) {
    double pi = 0.0;
    const double h = 1.0 / N; 
    
    omp_set_num_threads(numThreads);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel for reduction(+:pi) schedule(dynamic, BLOCK_SIZE)
    for (long long i = 0; i < N; i++) {
        double x = (i + 0.5) * h;             
        pi += (4.0 / (1.0 + x * x)) * h;       
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    return pi;  
}

int main() {
    printf("=============================================\n");
    printf("Task 3.2. Rectangle Method + OpenMP\n");
    printf("Dynamic scheduling with block size = 10 * student ID\n");
    printf("=============================================\n");
    printf("Student ID: %lld\n", STUDENT_ID);
    printf("N (iterations): %lld\n", N);
    printf("Block size: %lld\n", BLOCK_SIZE);
    printf("=============================================\n\n");
    
    int threadsList[] = { 1, 2, 4, 8, 12, 16 };
    const int numTests = 6;
    
    double piValues[numTests];
    long long timesMs[numTests];
    double speedups[numTests];
    
    printf("PERFORMANCE MEASUREMENT IN PROGRESS...\n");
    printf("=============================================\n\n");
    
    for (int idx = 0; idx < numTests; idx++) {
        int threads = threadsList[idx];
        printf("Testing with %2d threads ... ", threads);
        fflush(stdout);
        
        long long timeMs;
        double pi = computePiOpenMP(threads, timeMs);
        
        piValues[idx] = pi;
        timesMs[idx] = timeMs;
        
        printf("Time = %lld ms, Pi = %.15f\n", timeMs, pi);
    }
    
    long long baseTime = timesMs[0];
    for (int i = 0; i < numTests; i++) {
        speedups[i] = (double)baseTime / timesMs[i];
    }
    
    printf("\n=============================================\n");
    printf("RESULTS TABLE (OpenMP - Rectangle Method)\n");
    printf("=============================================\n");
    printf("Threads\t\tTime (ms)\tSpeedup\t\tPi Value\n");
    printf("-------\t\t---------\t-------\t\t--------\n");
    
    for (int i = 0; i < numTests; i++) {
        printf("%d\t\t%lld\t\t%.2fx\t\t%.15f\n",
               threadsList[i], timesMs[i], speedups[i], piValues[i]);
    }
    
    int bestThreads = threadsList[0];
    long long bestTime = timesMs[0];
    for (int i = 1; i < numTests; i++) {
        if (timesMs[i] < bestTime) {
            bestTime = timesMs[i];
            bestThreads = threadsList[i];
        }
    }
    
    const double exact_pi = 3.14159265358979323846;

    return 0;
}
