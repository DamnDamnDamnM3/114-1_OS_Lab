#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define SIZE 80
#define THREAD 4

int arr[SIZE][SIZE];
int result[SIZE][SIZE] = {0};
bool isStart = false;

/* don't change */
void readMatrix()
{
    FILE *fp;
    fp = fopen("number.txt", "r");
    if (!fp)
    {
        printf("number.txt not found\n");
        exit(1);
    }
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            fscanf(fp, "%d", &arr[i][j]);
        }
    }
    fclose(fp);
}

/* don't change */
long sum()
{
    long ret = 0;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            ret += result[i][j];
        }
    }
    return ret;
}

/* don't change */
void diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0)
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    printf("Time = %ld seconds and %ld nanoseconds\n", temp.tv_sec, temp.tv_nsec);
}

void multipfy(int startIndex, int endIndex)
{
    for (int i = startIndex; i < endIndex; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; k++)
            {
                result[i][j] += arr[i][k] * arr[k][j];
            }
        }
    }
}

void *child(void *arg)
{
    int *range = (int *)arg;
    int startIndex = range[0];
    int endIndex = range[1];

    printf("Child pid = %d\n", getpid());
    printf("Child tid = %ld, range = [%d, %d)\n", syscall(SYS_gettid), startIndex, endIndex);

    while (!isStart)
        ;

    multipfy(startIndex, endIndex);

    pthread_exit(NULL);
}

int main()
{
    pthread_t *threads;
    struct timespec timeStart, timeEnd;

    printf("Main pid = %d\n", getpid());
    printf("Main tid = %ld\n", syscall(SYS_gettid));

    readMatrix();

    threads = (pthread_t *)malloc(THREAD * sizeof(pthread_t));
    int ranges[THREAD][2];
    int per = SIZE / THREAD;

    for (int i = 0; i < THREAD; i++)
    {
        ranges[i][0] = i * per;
        ranges[i][1] = (i == THREAD - 1) ? SIZE : (i + 1) * per;
        pthread_create(&threads[i], NULL, child, ranges[i]);
    }

    sleep(1);
    isStart = true;
    clock_gettime(CLOCK_REALTIME, &timeStart);

    for (int i = 0; i < THREAD; i++)
    {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_REALTIME, &timeEnd);

    printf("Sum = %ld\n", sum());
    diff(timeStart, timeEnd);

    free(threads);
    return 0;
}
