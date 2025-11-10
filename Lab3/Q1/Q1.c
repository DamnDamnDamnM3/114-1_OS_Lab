#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define SIZE 80

int arr[SIZE][SIZE];
int result[SIZE][SIZE] = {0};
bool isStart = false;
typedef struct my_pid
{
    int pid;
} my_pid;

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
void multipfy()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            for (int k = 0; k < SIZE; k++)
            {
                result[i][j] += arr[i][k] * arr[k][j];
            }
        }
    }
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

int getLastCore()
{
    /* code */
    int coreCount = sysconf(_SC_NPROCESSORS_ONLN);
    return coreCount - 1;
}

void setTaskToCore(my_pid *pid) /*將Thread綁定到指定core*/
{
    int core = getLastCore(); /*獲得最後一個core編號*/
    while (pid->pid == 0)
        ; /*等待Thread取得PID*/
    /* code */
    char cmd[128];
    sprintf(cmd, "taskset -cp %d %d > /dev/null", core, pid->pid);
    system(cmd);
}

void *child(void *arg) /*子執行緒*/
{
    long *ret = calloc(1, sizeof(long));
    my_pid *pid = (my_pid *)arg;

    // pid->pid = // tid
    pid->pid = syscall(SYS_gettid);

    // printf(); // pid
    printf("Child: pid = %d\n", getpid());
    // printf(); // tid
    printf("Child: tid = %d\n", pid->pid);

    /* don't change */
    while (!isStart)
        ;
    multipfy();
    *ret = sum();
    /* ------------ */

    // pthread_exit();
    pthread_exit((void *)ret);
}

int main()
{
    pthread_t thread;
    void *ret;
    my_pid pid = {0};
    struct timespec timeStart, timeEnd;

    // printf(); // pid
    printf("Main: pid = %d\n", getpid());
    // printf(); // tid
    printf("Main: tid = %ld\n", syscall(SYS_gettid));

    readMatrix();

    // pthread_create();
    pthread_create(&thread, NULL, child, &pid);

    setTaskToCore(&pid);

    sleep(1);

    isStart = true;
    clock_gettime(CLOCK_REALTIME, &timeStart);

    // pthread_join();
    pthread_join(thread, &ret);

    clock_gettime(CLOCK_REALTIME, &timeEnd);

    // printf(); // verify the answer
    printf("Result = %ld\n", *(long *)ret);

    diff(timeStart, timeEnd); // time spent

    free(ret);
    return 0;
}
