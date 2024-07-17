#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <dispatch/dispatch.h>
#include <ctype.h>
#include <semaphore.h>

#define MAX_PATH_LENGTH 256
#define MAX_FILES 1000

int isPrime(int number)
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

int findAmountOfInt(char *filename)
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

typedef struct
{
    char *dirname;
    char *filename;
    int threadNum;
    int fileCount;
} ThreadData;

dispatch_semaphore_t mutex;

void *addFun(void *arg)
{
    dispatch_semaphore_wait(mutex, DISPATCH_TIME_FOREVER);
    ThreadData *data = (ThreadData *)arg;
    int threadNum = data->threadNum;
    char *filename = data->filename;
    char *dirname = data->dirname;

    sleep(1);
    char *path = malloc(MAX_PATH_LENGTH);
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, filename);
    FILE *file = fopen(path, "r");
    int primeCount = 0;
    int number;

    while (fscanf(file, "%d", &number) != EOF)
    {
        if (isPrime(number))
        {
            primeCount++;
        }
    }

    printf("Thread %d found %d prime numbers in %s\n", threadNum, primeCount, filename);
    dispatch_semaphore_signal(mutex);

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
    while ((entry = readdir(dirp)) != NULL && file_count < MAX_FILES)
    {
        if (entry->d_type == DT_REG)
        {
            filenamea[a] = entry->d_name;
            file_count++;
            a++;
        }
    }
    closedir(dirp);

    pthread_t threads[file_count];
    ThreadData thread_data[file_count];
    mutex = dispatch_semaphore_create(numThreads);

    for (int i = 0; i < file_count; i++)
    {
        thread_data[i] = data;
        thread_data[i].threadNum = i;
        thread_data[i].fileCount = file_count;
        thread_data[i].filename = filenamea[i];
        pthread_create(&threads[i], NULL, addFun, &thread_data[i]);
    }

    for (int i = 0; i < file_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
