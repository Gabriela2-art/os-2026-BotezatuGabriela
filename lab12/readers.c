#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define READERS 3
#define WRITERS 2

int value = 0;
int readers = 0;
sem_t rlock;
sem_t room;

void *reader(void *arg) {
    long id = (long)arg;
    sem_wait(&rlock);
    if (++readers == 1)
        sem_wait(&room);
    sem_post(&rlock);

    printf("reader %ld reads %d     (readers inside: %d)\n",
           id, value, readers);
    usleep(200000);

    sem_wait(&rlock);
    if (--readers == 0)
        sem_post(&room);
    sem_post(&rlock);
    return NULL;
}

void *writer(void *arg) {
    long id = (long)arg;
    sem_wait(&room);
    value += 10;
    printf("writer %ld writes %d (alone)\n", id, value);
    usleep(200000);
    sem_post(&room);
    return NULL;
}

int main(void) {
    sem_init(&rlock, 0, 1);
    sem_init(&room, 0, 1);

    pthread_t r[READERS], w[WRITERS];

    pthread_create(&w[0], NULL, writer, (void *)1L);

    for (long i = 0; i < READERS; i++)
        pthread_create(&r[i], NULL, reader, (void *)(i + 1));

    pthread_create(&w[1], NULL, writer, (void *)2L);

    for (int i = 0; i < READERS; i++)
        pthread_join(r[i], NULL);

    for (int i = 0; i < WRITERS; i++)
        pthread_join(w[i], NULL);

    return 0;
}
