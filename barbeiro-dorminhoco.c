#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"

#define N_CHAIRS 5 // número de cadeiras para os clientes aguardarem 
#define N_CUSTOMERS 10

sem_t customers; // contador de clientes esperando
sem_t barbers; // contador de barbeiros esperando
sem_t mutex; // mutex para controlar o acesso às variáveis compartilhadas

int waiting = 0; // contador de clientes na sala de espera
/* Protótipos das funções */
void barber();
void *customer(void *arg);
void cut_hair();
void customer_arrived(int client);
void get_haircut();
void giveup_haircut();

int main(){    
    sem_init(&customers, 1, 0);
    sem_init(&barbers, 1, 0);
    sem_init(&mutex, 1, 1);
    
    pthread_t barber_thread;
    pthread_t customer_threads[N_CUSTOMERS];

    pthread_create(&barber_thread, NULL, (void *) barber, NULL); // Criando um único barbeiro 

    int ids[N_CUSTOMERS];
    for(int i=0;i < N_CUSTOMERS; i++){
        ids[i] = i;
        pthread_create(&customer_threads[i], NULL, (void *) customer, &ids[i]); // Criação indefinida de cliente
        sleep(1);
    }
    for (int i = 0; i < N_CUSTOMERS; i++){
       pthread_join(customer_threads[i], NULL);
    }  
    return 0;
}

void barber() {
    while(1) {
       sem_wait(&customers); // o barbeiro dorme se nº de clientes for 0 
       sem_wait(&mutex); // obtém acesso a 'waiting' 
       waiting--; // descreve o contador de clientes aguardando 
       sem_post(&barbers); // o barbeiro está pronto para cortar o cabelo do cliente 
       sem_post(&mutex); // libera 'waiting' 
       
       cut_hair(); // corta o cabelo (fora da região crítica) 
    }
    pthread_exit(NULL);
}

void *customer(void *arg) { 
    sem_wait(&mutex); // entra na região crítica 
    int id = *((int *)arg);
    sleep(id); // Espaço de tempo entre chegada de clientes

    if(waiting < N_CHAIRS) { // se houver cadeiras livres, entra no laço 
        customer_arrived(id); // cliente chegou!
        waiting++; // incrementa o contador de clientes aguardando 
        
        sem_post(&customers); // acorda o barbeiro 
        sem_post(&mutex); // libera o acesso a 'waiting' 
        sem_wait(&barbers); // vai dormir se o número de barbeiros livres for 0 
        get_haircut(); // sentado e sendo servido 
    } else {
        sem_post(&mutex); // a barbearia está cheia, não espere
        giveup_haircut();   
    }
    pthread_exit(NULL);
}

void cut_hair() {
    printf("Barbeiro está cortando o cabelo do cliente!\n");
    sleep(3);
}
void customer_arrived(int client) {
    printf("Cliente %d chegou para cortar o cabelo!\n", client);
}
void get_haircut() {
    printf("Cliente está tendo o cabelo cortado!\n");
}
void giveup_haircut() {
    printf("Cliente desistiu! O salão está muito cheio!\n");
}
