#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

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

    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 1;
    }

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

    return (count);
}

int num = 0;
int count = 0;
sem_t mutex;
void *addFun(void *arg, char *dirname, int loopAmount)
{

    printf("Thread %d created\n", num);
    long tid = (long)arg;
    char *filename;

    int primeCount = 0;
    sem_wait(&mutex); // lock critical region

    DIR *dirp2;
    struct dirent *entry2;
    dirp2 = opendir(dirname);
    while ((entry2 = readdir(dirp2)) != NULL)
    {
        count++;
        if (entry2->d_type == DT_REG)
        {

            filename = entry2->d_name;
            char path[100];
            strcpy(path, dirname);
            strcat(path, "/");
            strcat(path, filename);
            int amount = findAmountOfInt(path);
            FILE *file;
            file = fopen(path, "r");
            int number;
            while (fscanf(file, "%d", &number) != EOF)
            {
                if (isPrime(number))
                {
                    primeCount++;
                }
            }
            fclose(file);
            printf("%d",count);
            printf("Thread %lx has found %d prime numbers in %s\n", tid, primeCount, filename);
            primeCount = 0;
        }
        
    }

        printf("Thread %d created\n", num);
    closedir(dirp2);
    sem_post(&mutex); // unlock critical region
    num++;
    return NULL;
}

int main(int argc, char *argv[])
{
    char *dirname = argv[1];
    int numThreads = atoi(argv[2]);
    printf("Number of threads: %d\n", numThreads);
    pthread_t threads[numThreads];
    int file_count = 0;
    
    DIR *dirp;
    struct dirent *entry;

    dirp = opendir(dirname);
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            file_count++;
        }
    }
    closedir(dirp);

    int loopAmount = file_count / numThreads;
    printf("Number of files: %d\n", file_count);

    sem_init(&mutex, 0, numThreads);

    for (int i = 0; i < numThreads; i++)
    {
        pthread_create(&threads[i], NULL, addFun((void *)i, dirname, loopAmount), NULL);
    }
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&mutex);
    return 0;
}