#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/*
 * int sem_wait (sem_t *sem); decrementa contador, espera ter espaco para prosseguir
 * int sem_post (sem_t *sem); incrementa contador,
 *
 *
 */

// Similar to sem_wait but wait only until ABSTIME.
// int sem_timedwait (sem_t *restrict sem, const struct timespec *restrict abstime);
// Talvez seja útil para starvation

// Test whether SEM is posted.
// int sem_trywait (sem_t *sem);
// AAAAA

// Get current value of SEM and store it in *SVAL.
// int sem_getvalue (sem_t *restrict sem, int *restrict sval);
// AAAAA2

//! sem_post! atomically increases the count  of  the  semaphore  pointed  to  by
//|sem|.  This function never blocks and can safely be used in asynchronous sig-
// nal handlers.

#define MULHER 0
#define HOMEM 1
#define MAX 128

static int numH = 0;
static int numM = 0;
static int banheiro = 0;

sem_t sema_vagaHomem, sema_vagaMulher, lock_homem, lock_mulher;

void *homem()
{
    int valor = numH;
    numH++;
    int vagas;
    int is_unlocked;
    while (1)
    {
        sem_getvalue(&sema_vagaMulher, &vagas);
        sem_getvalue(&lock_homem, &is_unlocked);
        if ((vagas == 3) && (is_unlocked) && (numM == 0)) 
            break;
        else {
            sleep(5);
            sem_wait(&lock_mulher);
            while (1) {
                sem_getvalue(&sema_vagaMulher, &vagas);
                if ((vagas == 3) && (numM == 0))
                    break;
            }
        }
    }
    sem_wait(&sema_vagaHomem);
    sem_post(&lock_mulher);
        //Entrou no banheiro
        printf("Homem %d começou a usar\n", valor);
        banheiro++;
        printf("Homens no banheiro: %d\n", banheiro);
        sleep(5);
        banheiro--;
        printf("Homem %d terminou de usar\n", valor);
        printf("Homens no banheiro: %d\n", banheiro);
        //Saiu do banheiro
    sem_post(&sema_vagaHomem);
}

void *mulher()
{
    int valor = numM;
    numM++;
    int vagas;
    int is_unlocked;
    // Com o código comentado acho que toda vez q uma mulher chega trava a entrada de outros homens
    while (1)
    {
        sem_getvalue(&sema_vagaHomem, &vagas);
        sem_getvalue(&lock_mulher, &is_unlocked);
        if ((vagas == 3) && (is_unlocked) && (numH == 0))
            break;
        else{
            sleep(5);
            sem_wait(&lock_homem);
            while (1){
                sem_getvalue(&sema_vagaHomem, &vagas);
                if ((vagas == 3) && (numH == 0))
                    break;
            }
        }
    }
    sem_wait(&sema_vagaMulher);
    sem_post(&lock_homem);
        //Entrou no banheiro
        printf("Mulher %d começou a usar\n", valor);
        banheiro++;
        printf("Mulheres no banheiro: %d\n", banheiro);
        sleep(5);
        banheiro--;
        printf("Mulher %d terminou de usar\n", valor);
        printf("Mulheres no banheiro: %d\n", banheiro);
        //Saiu do banheiro
    sem_post(&sema_vagaMulher);
}

int filaPessoas()
{// retorna somente 0(MULHER) ou 1(HOMEM)
    srand(time(NULL));
    return rand() % 2;
}

int main(int argc, char const *argv[])
{
    char err_msg[MAX];
    int pessoa;

    pthread_t th_homens, th_mulheres;

    // char *strerror_r(int errnum, char *buf, size_t buflen);
    // int sem_init (sem_t *sem, int pshared, unsigned int value);

    // inicialializando um semáforo com 3 vagas do banheiro;
    if (sem_init(&sema_vagaMulher, 0, 3) < 0)
    {
        strerror_r(errno, err_msg, MAX);
        printf("Erro em sem_init: %s\n", err_msg);
        exit(1);
    }

    // inicialializando um semáforo com 3 vagas do banheiro;
    if (sem_init(&sema_vagaHomem, 0, 3) < 0)
    {
        strerror_r(errno, err_msg, MAX);
        printf("Erro em sem_init: %s\n", err_msg);
        exit(1);
    }

    // inicializa um semáforo binário para bloquear a entrada de homens;
    if (sem_init(&lock_homem, 0, 1) < 0)
    {
        strerror_r(errno, err_msg, MAX);
        printf("Erro em sem_init: %s\n", err_msg);
        exit(1);
    }

    // inicializa um semáforo binário para bloquear a entrada de mulheres;
    if (sem_init(&lock_mulher, 0, 1) < 0)
    {
        strerror_r(errno, err_msg, MAX);
        printf("Erro em sem_init: %s\n", err_msg);
        exit(1);
    }

    while (1){
        pessoa = filaPessoas();
        printf("%s %d gerado\n", pessoa ? "Homem" : "Mulher", pessoa ? numH : numM);

        if (pessoa == HOMEM){
            if (pthread_create(&th_homens, NULL, homem, NULL) != 0){
                strerror_r(errno, err_msg, MAX);
                printf("Erro em pthread_create: %s\n", err_msg);
                exit(1);
            }
        }
        else{ // vai ser mulher
            if (pthread_create(&th_mulheres, NULL, mulher, NULL) != 0){
                strerror_r(errno, err_msg, MAX);
                printf("Erro em pthread_create: %s\n", err_msg);
                exit(1);
            }
        }
        sleep(1);
    }

    pthread_join(th_homens, NULL);
    pthread_join(th_mulheres, NULL);

    sem_destroy(&sema_vagaHomem);
    sem_destroy(&sema_vagaMulher);
    sem_destroy(&lock_homem);
    sem_destroy(&lock_mulher);

    return 0;
}