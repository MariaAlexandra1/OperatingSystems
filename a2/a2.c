#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NR_THREADS2 5
#define NR_THREADS7 45
#define NR_THREADS8 6
#define MAX_THREADS7 4
#define NR_BAR (NR_THREADS7/MAX_THREADS7)

pthread_barrier_t bar;

sem_t sem1, sem2;
sem_t* sem_5_4;
sem_t* sem_4_2;

void* thread_fn_2(void* args)
{
    int tid = *(int *) args;

    if(tid == 2)
    {
        info(BEGIN,2,2);
        sem_post(&sem1);
        sem_wait(&sem2);
        info(END,2,2);
    }else if(tid == 5)
    {
        sem_wait(&sem1);
        info(BEGIN, 2, 5);
        info(END, 2, 5);
        sem_post(&sem2);
    }else if(tid == 4)
    {
    	sem_wait(sem_5_4);
    	info(BEGIN, 2, tid);
    	info(END, 2, tid);
    	sem_post(sem_4_2);
    }
    else{
        info(BEGIN, 2, tid);
        info(END, 2, tid);
    }
    
    return NULL;
}

void process2()
{
    pthread_t ths_2[NR_THREADS2 + 1];
    int tids_2[NR_THREADS2 + 1];

    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);

    for(int i = 1; i <= NR_THREADS2; i++)
    {
        tids_2[i] = i;
        pthread_create(&ths_2[i], NULL, thread_fn_2, &tids_2[i]);
    }

    for(int i = 1; i <= NR_THREADS2; i++)
    {
        pthread_join(ths_2[i], NULL);
    }

    sem_destroy(&sem1);
    sem_destroy(&sem2);
}


pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int ths_count = 0;

void* thread_fn_7(void* args) {
    int tid = *(int *) args;

    pthread_mutex_lock(&mutex);
    while (ths_count >= 4) {
        pthread_cond_wait(&cond_var, &mutex);
    }
    ths_count++;
    pthread_mutex_unlock(&mutex);

    info(BEGIN, 7, tid);

    info(END, 7, tid);

    pthread_mutex_lock(&mutex);
    ths_count--;
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&mutex);

    return NULL;
}


void process7()
{
    pthread_t ths_7[NR_THREADS7 + 1];
    int tids_7[NR_THREADS7 + 1];

    for(int i = 1; i <= 45 ; i++)
    {
        tids_7[i] = i;
        pthread_create(&ths_7[i], NULL, thread_fn_7, &tids_7[i]);
        
    }

    for(int i = 1; i <= NR_THREADS7; i++)
    {
        pthread_join(ths_7[i], NULL);
    }
    
}

void* thread_fn_8(void* args)
{
    int tid = *(int *) args;
    
    if(tid == 5)
    {
    info(BEGIN, 8, tid);
    info(END, 8, tid);
    sem_post(sem_5_4);
    }
    else if(tid == 2)
    {
    	sem_wait(sem_4_2);
    	info(BEGIN, 8, tid);
    	info(END, 8, tid);
    }
    else{
    	info(BEGIN, 8, tid);
    	info(END, 8, tid);
    }

    return NULL;
    
}
    

void process8()
{
    pthread_t ths_8[NR_THREADS8 + 1];
    int tids_8[NR_THREADS8 + 1];

    for(int i = 1; i <= NR_THREADS8; i++)
    {
        tids_8[i] = i;
        pthread_create(&ths_8[i], NULL, thread_fn_8, &tids_8[i]);
    }

    for(int i = 1; i <= NR_THREADS8; i++)
    {
        pthread_join(ths_8[i], NULL);
    }
}


int main(){

    init();

    pid_t P2, P3, P4, P5, P6, P7, P8;
    
    sem_5_4 = sem_open("/sem_5_4", O_CREAT, 0644, 0);
    sem_4_2 = sem_open("/sem_4_2", O_CREAT, 0644, 0);
    sem_unlink("/sem_5_4");
    sem_unlink("/sem_4_2");

    info(BEGIN, 1, 0);
    P2 = fork();
    if(P2 == -1)
    {
        perror("Error P2");
        return -1;
    }
    else if(P2 == 0){
        //P2 start
        info(BEGIN, 2, 0);

        P6 = fork();
        if(P6 == -1)
        {
            perror("Error P6");
            return -1;
        }
        else if(P6 == 0)
        {
            //P6 start
            info(BEGIN, 6, 0);

            P8 = fork();
            if(P8 == -1)
            {
                perror("Error P6");
                return -1;
            }
            else if(P8 == 0)
            {
                //P8 start
                info(BEGIN, 8, 0);

                process8();
                
                //P8 finish
                info(END, 8, 0);
            }
            else{
                //P6 wait and finish
                waitpid(P8, NULL, 0);
                info(END, 6, 0);
            }
        }
        else{
            //P2 wait and finish
	    process2();
            waitpid(P6, NULL, 0);
            info(END, 2, 0);
        }
    }else{
        //P1
        P3 = fork();
        if(P3 == -1)
        {
            perror("Error P3");
            return -1;
        }
        else if(P3 == 0){
        //P3 start
            info(BEGIN, 3, 0);

            P4 = fork();
            if(P4 == -1)
            {
                perror("Error P4");
                return -1;
            }
            else if(P4 == 0)
            {
                //P4 start
                info(BEGIN, 4, 0);

                P5 = fork();
                if(P5 == -1)
                {
                    perror("Error P5");
                    return -1;
                }
                else if(P5 == 0)
                {
                    //P5 start
                    info(BEGIN, 5, 0);

                    //P5 finish
                    info(END, 5, 0);
                }else{
                //P4
                    P7 = fork();
                    if(P7 == -1)
                    {
                        perror("Error P7");
                        return -1;
                    }
                    else if(P7 == 0)
                    {
                        //P7 start
                        info(BEGIN, 7, 0);

                        process7();
                        //P7 finish
                        info(END, 7, 0);
                    }
                    else{
                    //P4 wait and finish
                        waitpid(P5, NULL, 0);
                        waitpid(P7, NULL, 0);
                        info(END, 4, 0);
                    }
                }
            }else{
            //P3 wait and finish
                waitpid(P4, NULL, 0);
                info(END, 3, 0);
            }
        }else
        {
       		waitpid(P2, NULL, 0);
    		waitpid(P3, NULL, 0);
    		info(END, 1, 0);
        }
    }


    return 0;
}

