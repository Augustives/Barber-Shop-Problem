#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>

#define capacidade_maxima 20
#define numero_clientes_dia 20
#define numero_barbeiros 3
#define capacidade_sala_espera 16
#define capacidade_sofa 4

typedef struct{
    sem_t lider;
    sem_t seguidor;
} FIFO;

FIFO *sala_de_espera;
FIFO *sofa;


FIFO* criar_fifo (int num) {
    FIFO* F=(FIFO*)malloc(sizeof(FIFO));
    sem_init(&(F -> lider), 0, 0);
    sem_init(&(F -> seguidor), 0, num);
    return (F);
};


void incrementa_fifo (FIFO* F) {
    sem_wait(&(F -> lider));
    sem_post(&(F -> seguidor));
};


void decrementa_fifo (FIFO* F) {
    sem_wait(&(F -> seguidor));
    sem_post(&(F -> lider));
};


sem_t mutex, cadeira, barbeiro, cliente, pagamento_cliente, pagamento_barbeiro, registradora;

pthread_t thread_clientes[numero_clientes_dia];
pthread_t thread_barbeiros[numero_barbeiros];

int num_clientes = 0;


// Ações clientes
void entra_loja(int num) {
   printf("Cliente %d entrou na loja.\n", num);
};


void senta_sofa(int num) {
    printf("Cliente %d sentou no sofa.\n", num);
};


void senta_corte(int num) {
    printf("Cliente %d sentou para receber corte.\n", num);
};


void receber_corte(int num) {
    printf("Cliente %d está recebendo seu corte.\n", num);
};


void paga_corte(int num) {
    printf("Cliente %d esta pagando seu corte.\n", num);
};


void sai_loja(int num) {
    printf("Cliente %d saiu da loja.\n", num);
};


//Ações dos barbeiros
void corta_cabelo() {
    printf("Barbeiro esta cortando cabelo.\n");
};


void aceita_pagamento() {
    printf("Barbeiro esta aceitando um pagamento.\n");
};


void* thread_atendimento (void* arg) {
    int num = *(int *)arg;

    int i;

    for (i=0; i<numero_clientes_dia; i++) {
        sem_wait(&cliente);
        sem_post(&barbeiro);
        corta_cabelo();

        sem_wait(&pagamento_cliente);
        aceita_pagamento();
        sem_post(&pagamento_barbeiro);
        sem_post(&registradora);
        
        sem_post(&cadeira);
    };
};


void* thread_principal (void *arg) {
    int num = *(int *)arg;

    sem_wait(&mutex);
        
    if (num_clientes==capacidade_maxima) {
        sem_post(&mutex);
        sai_loja(num);
    };

    num_clientes+=1;
    sem_post(&mutex);

    decrementa_fifo(sala_de_espera);
    entra_loja(num);

    decrementa_fifo(sofa);
    senta_sofa(num);
    incrementa_fifo(sala_de_espera);

    sem_wait(&cadeira);
    senta_corte(num);
    incrementa_fifo(sofa);

    sem_post(&cliente);
    sem_wait(&barbeiro);
    receber_corte(num);

    paga_corte(num);
    sem_post(&pagamento_cliente);
    sem_wait(&pagamento_barbeiro);

    sem_wait(&mutex);
    num_clientes-=1;
    sem_post(&mutex);

    sai_loja(num);
};


main() {
    int num_cliente[numero_clientes_dia];

    sem_init(&mutex, 0, 1);
    sem_init(&cliente, 0, 0);
    sem_init(&barbeiro, 0, 0);
    sem_init(&cadeira, 0, 3);
    sem_init(&registradora, 0, 0);
    sem_init(&pagamento_cliente, 0, 0);
    sem_init(&pagamento_barbeiro, 0, 0);

    sala_de_espera = criar_fifo(capacidade_sala_espera);
    sofa = criar_fifo(capacidade_sofa);

    int i; 

    for (i=0; i<numero_clientes_dia; i++) {
        num_cliente[i]=i;
        pthread_create(&thread_clientes[i], 0, thread_principal, &num_cliente[i]);
    };
    
    for (i=0; i<numero_barbeiros; i++) {
        pthread_create(&thread_barbeiros[i], 0, thread_atendimento, &num_cliente[i]);
    };

    for (i=0; i<numero_clientes_dia; i++) {
        pthread_join(thread_clientes[i], NULL);
    };

    for (i=0; i<numero_barbeiros; i++) {
        pthread_join(thread_barbeiros[i], NULL);
    };
};