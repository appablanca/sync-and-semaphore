#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>

#define MAX_PATH_LENGTH 256
#define MAX_FILES 1000

int isPrime(int number) // Function to check if a number is prime
{
    if (number <= 1)
    {
        return 0;
    }

    for (int i = 2; i * i <= number; i++)
    {
        if (number % i == 0)
        {
            return 0;
        }
    }

    return 1;
}

int findAmountOfInt(char *filename) // Function to find the amount of integers in a file
{
    FILE *file;
    char ch;
    int count = 0;

    file = fopen(filename, "r");

    while ((ch = fgetc(file)) != EOF)
    {
        if (isdigit(ch))
        {
            count++;
            while (isdigit(ch = fgetc(file)))
            {
            }
        }
    }

    fclose(file);

    return count;
}

typedef struct // Struct to hold data for each thread
{
    char *dirname;
    char *filename;
    int threadNum;
    int fileCount;
} ThreadData;

sem_t mutex;

void *addFun(void *arg) // Function to be executed by each thread
{
    sem_wait(&mutex); // semaphore wait to ensure only numThreads threads are running at the same time
    ThreadData *data = (ThreadData *)arg; // Cast the argument to ThreadData
    // Extract the data from the struct
    int threadNum = data->threadNum; 
    char *filename = data->filename;
    char *dirname = data->dirname;

    sleep(1);
    char *path = malloc(MAX_PATH_LENGTH);
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, filename);
    FILE *file = fopen(path, "r");
    free(path);
    int primeCount = 0;
    int number;

    while (fscanf(file, "%d", &number) != EOF) // Read each number from the file and check if it is prime
    {
        if (isPrime(number))
        {
            primeCount++;
        }
    }

    printf("Thread %d found %d prime numbers in %s\n", threadNum, primeCount, filename);
    sem_post(&mutex); // semaphore post to signal that the thread has finished

    fclose(file);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    char *dirname = argv[1];  
    int numThreads = atoi(argv[2]);

    ThreadData data;
    char *filenamea[MAX_FILES];
    data.dirname = dirname;

    DIR *dirp;
    struct dirent *entry;
    int a = 0;
    dirp = opendir(dirname);

    int file_count = 0; 
    while ((entry = readdir(dirp)) != NULL && file_count < MAX_FILES) // Read all files in the directory and assign each file to a thread with an array
    {
        if (entry->d_type == DT_REG)
        {
            filenamea[a] = strdup(entry->d_name);
            file_count++;
            a++;
        }
    }
    closedir(dirp);

    pthread_t threads[file_count];
    ThreadData thread_data[file_count];

    sem_init(&mutex, 0, numThreads); // Initialize semaphore

    for (int i = 0; i < file_count; i++) // Create threads and assign each file to a thread
    {
        thread_data[i] = data;
        thread_data[i].threadNum = i;
        thread_data[i].fileCount = file_count;
        thread_data[i].filename = filenamea[i];
        pthread_create(&threads[i], NULL, addFun, &thread_data[i]);
    }

    for (int i = 0; i < file_count; i++) // Wait for all threads to finish
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&mutex); // Destroy semaphore

    for (int i = 0; i < file_count; i++) // Free the memory allocated for the filenames
    {
        free(filenamea[i]);
    }

    return 0;
}