#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"

#define N_CHAIRS 5 /* número de cadeiras para os clientes aguardarem */
#define N_CUSTOMERS 10

typedef int semaphore; /* iniciando o semáforo */
semaphore customers; /* número inicial de clientes à espera de atendimento */
semaphore barbers; /* número inicial de barbeiros à espera de clientes */
semaphore mutex; /* acesso para a exclusão mútua */

int waiting = 0; /* número inicial de clientes que estão esperando */

/* Protótipos das funções */
void *barber(void *arg);
void *customer(void *arg);
void cut_hair();
void customer_arrived();
void get_haircut();
void giveup_haircut();

int main(){
    sem_init((sem_t*)&customers, 1, N_CHAIRS);
    sem_init((sem_t*)&barbers, 1, 1);
    sem_init((sem_t*)&mutex, 1, 0);

    pthread_t b, c;
    
    pthread_create(&b, NULL, (void *) barber, NULL); /* Criando um único barbeiro */

    while(1) {
        pthread_create(&c, NULL, (void *) customer, NULL); /* Criação indefinida de cliente */
        sleep(1);
    }

    return 0;
}

void *barber(void *arg) {
    while(1) {
       sem_wait(&customers); /* o barbeiro dorme se nº de clientes for 0 */
       sem_wait(&mutex); /* obtém acesso a 'waiting' */
       waiting = waiting - 1; /* descreve o contador de clientes aguardando */
       sem_post(&barbers); /* o barbeiro está pronto para cortar o cabelo do cliente */
       sem_post(&mutex); /* libera 'waiting' */
       cut_hair(); /* corta o cabelo (fora da região crítica) */
       giveup_haircut();
    }
    pthread_exit(NULL);
}
void *customer(void *arg) { 
    sem_wait(&mutex); /* entra na região crítica */
    int id = *((int *)arg); 
    sleep(id); /* Espaço entre chegada de clientes */
    if(waiting < N_CHAIRS) { /* se houver cadeiras livres, entra no laço */
       customer_arrived(); /* cliente chegou!*/
       waiting = waiting + 1; /* incrementa o contador de clientes aguardando */
       sem_post(&customers); /* acorda o barbeiro */
       sem_post(&mutex); /* libera o acesso a 'waiting' */
       sem_wait(&barbers); /* vai dormir se o número de barbeiros livres for 0 */
       get_haircut(); /* sentado e sendo servido */
    } else {
        sem_post(&mutex); /* a barbeira está cheia, não espere*/
    }
    pthread_exit(NULL);
}
void cut_hair() {
    printf("O Barbeiro está cortando o cabelo!\n");
    sleep(3);
}
void customer_arrived() {
    printf("O cliente chegou!\n");
}
void get_haircut() {
    printf("O cliente está sendo servido!\n");
}
void giveup_haircut() {
    printf("O cliente desistiu! O salão está muito cheio!\n");
}