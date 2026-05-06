#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <iostream>

const long long N = 100000000;
const long long STUDENT_ID = 431512;
const long long BLOCK_SIZE = 10 * STUDENT_ID;
const long long BLOCK_COUNT = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;

volatile long long currentBlock = 0;
double globalSum = 0.0;
long long completedBlocks = 0;
bool allTasksCompleted = false;
CRITICAL_SECTION cs;

struct ThreadData {
    int threadId;
    double localSum;
};

DWORD WINAPI PiThreadFunction(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    const double h = 1.0 / N;
    
    while (true) {
        EnterCriticalSection(&cs);
        long long myBlock = currentBlock;
        if (myBlock < BLOCK_COUNT) {
            currentBlock++;
        }
        LeaveCriticalSection(&cs);
        
        if (myBlock >= BLOCK_COUNT) {
            break;
        }
        
        long long start = myBlock * BLOCK_SIZE;
        long long end = start + BLOCK_SIZE;
        if (end > N) end = N;
        
        double localSum = 0.0;
        for (long long i = start; i < end; i++) {
            double x = (i + 0.5) * h;
            localSum += (4.0 / (1.0 + x * x)) * h;
        }
        
        EnterCriticalSection(&cs);
        data->localSum += localSum;
        globalSum += localSum;
        completedBlocks++;
        LeaveCriticalSection(&cs);
        
        SuspendThread(GetCurrentThread());
    }
    
    return 0;
}

double runWithThreads(int threadCount, long long& elapsedMs) {
    currentBlock = 0;
    globalSum = 0.0;
    completedBlocks = 0;
    allTasksCompleted = false;
    
    InitializeCriticalSection(&cs);
    
    HANDLE* threads = new HANDLE[threadCount];
    ThreadData* threadData = new ThreadData[threadCount];
    
    for (int i = 0; i < threadCount; i++) {
        threadData[i].threadId = i;
        threadData[i].localSum = 0.0;
        
        threads[i] = CreateThread(
            NULL,
            0,
            PiThreadFunction,
            &threadData[i],
            CREATE_SUSPENDED,
            NULL
        );
        
        if (threads[i] == NULL) {
            printf("Error creating thread %d\n", i);
            return 0.0;
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int blocksGiven = 0;
    for (int i = 0; i < threadCount && blocksGiven < BLOCK_COUNT; i++) {
        ResumeThread(threads[i]);
        blocksGiven++;
    }
    
    while (completedBlocks < BLOCK_COUNT) {
        for (int i = 0; i < threadCount; i++) {
            if (completedBlocks >= BLOCK_COUNT) break;
            
            EnterCriticalSection(&cs);
            bool hasTasks = (currentBlock < BLOCK_COUNT);
            LeaveCriticalSection(&cs);
            
            if (hasTasks) {
                ResumeThread(threads[i]);
                Sleep(1);
            }
        }
        Sleep(1);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    double totalSum = 0.0;
    for (int i = 0; i < threadCount; i++) {
        totalSum += threadData[i].localSum;
    }
    
    allTasksCompleted = true;
    for (int i = 0; i < threadCount; i++) {
        ResumeThread(threads[i]);
    }
    
    WaitForMultipleObjects(threadCount, threads, TRUE, INFINITE);
    
    for (int i = 0; i < threadCount; i++) {
        CloseHandle(threads[i]);
    }
    delete[] threads;
    delete[] threadData;
    DeleteCriticalSection(&cs);
    
    return totalSum;
}

int main() {
    printf("=============================================\n");
    printf("Task 3.1. Rectangle Method + Win32 API\n");
    printf("USING SuspendThread / ResumeThread\n");
    printf("=============================================\n");
    printf("Student ID: %lld\n", STUDENT_ID);
    printf("N (iterations): %lld\n", N);
    printf("Block size: %lld\n", BLOCK_SIZE);
    printf("Number of blocks: %lld\n", BLOCK_COUNT);
    printf("=============================================\n\n");
    
    int threadsList[] = { 1, 2, 4, 8, 12, 16 };
    const int numTests = 6;
    
    double piValues[numTests];
    long long timesMs[numTests];
    
    for (int idx = 0; idx < numTests; idx++) {
        int threads = threadsList[idx];
        printf("Testing with %2d threads ...\n", threads);
        
        long long timeMs;
        double pi = runWithThreads(threads, timeMs);
        
        piValues[idx] = pi;
        timesMs[idx] = timeMs;
        
        printf("   pi = %.15f\n", pi);
        printf("   Time = %lld ms\n\n", timeMs);
    }
    
    printf("\n========== RESULTS ==========\n");
    printf("Threads\tTime (ms)\tpi (approximate)\n");
    printf("-------\t---------\t----------------\n");
    for (int i = 0; i < numTests; i++) {
        printf("%d\t%lld\t\t%.15f\n",
               threadsList[i], timesMs[i], piValues[i]);
    }
    
    const double exact_pi = 3.14159265358979323846;
    printf("\nExact pi: %.15f\n", exact_pi);
    printf("Error for last test: %.15f\n", fabs(piValues[numTests-1] - exact_pi));
    
    return 0;
}
