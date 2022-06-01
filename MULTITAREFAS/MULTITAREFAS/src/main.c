#include <stdint.h>
#include <stdio.h>

#include "multitarefas.h"

semaforo cheio= 0;
semaforo vazio = N;

/*
 * Prototipos das tarefas
 */
void tarefa_1(void);
void tarefa_2(void);
void tarefa_3(void);
void produtor(void);
void consumidor(void);

/*
 * Configuracao dos tamanhos das pilhas
 */
#define TAM_PILHA_1		(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_2		(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_3		(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_4		(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_5		(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_OCIOSA	(TAM_MINIMO_PILHA + 24)

/*
 * Declaracao das pilhas das tarefas
 */
uint32_t PILHA_TAREFA_1[TAM_PILHA_1];
uint32_t PILHA_TAREFA_2[TAM_PILHA_2];
uint32_t PILHA_TAREFA_3[TAM_PILHA_3];
uint32_t PILHA_TAREFA_4[TAM_PILHA_4];
uint32_t PILHA_TAREFA_5[TAM_PILHA_5];
uint32_t PILHA_TAREFA_OCIOSA[TAM_PILHA_OCIOSA];

/*
 * Funcao principal de entrada do sistema
 */
int main(void)
{
	
	/* Criacao das tarefas */
	/* Parametros: ponteiro, nome, ponteiro da pilha, tamanho da pilha, prioridade da tarefa */
	
	CriaTarefa(tarefa_1, "Tarefa 1", PILHA_TAREFA_1, TAM_PILHA_1, 1);
	
	CriaTarefa(tarefa_2, "Tarefa 2", PILHA_TAREFA_2, TAM_PILHA_2, 2);
        
        CriaTarefa(tarefa_3, "Tarefa 3", PILHA_TAREFA_3, TAM_PILHA_3, 3);
        
        CriaTarefa(produtor, "Produtor", PILHA_TAREFA_4, TAM_PILHA_4, 4);
        
        CriaTarefa(consumidor, "consumidor", PILHA_TAREFA_5, TAM_PILHA_5, 5);
	
	/* Cria tarefa ociosa do sistema */
	CriaTarefa(tarefa_ociosa,"Tarefa ociosa", PILHA_TAREFA_OCIOSA, TAM_PILHA_OCIOSA, 0);
	
	/* Configura marca de tempo */
	ConfiguraMarcaTempo();   
	
	/* Inicia sistema multitarefas */
	IniciaMultitarefas();
	
	/* Nunca chega aqui */
	while (1)
	{
	}
}


/* Tarefas de exemplo que usam funcoes para suspender/continuar as tarefas */
void tarefa_1(void)
{
	volatile uint16_t a = 0;
	for(;;)
	{
		a++;
		TarefaContinua(2);
	
	}
}

void tarefa_2(void)
{
	volatile uint16_t b = 0;
	for(;;)
	{
		b++;
		TarefaSuspende(2);
                TarefaContinua(3);
                
	}
}

void tarefa_3(void)
{
	volatile uint16_t c = 0;
	for(;;)
	{
		c++;
		TarefaSuspende(3);	
	}
}

void produtor(void)
{
	volatile uint16_t d = 0;
	for(;;)
	{
		wait(vazio);
                f = (f+1)%N;
                buffer[f] = produz();
                signal(cheio);
	}
}
void produtor(void)
{
	volatile uint16_t e = 0;
	for(;;)
	{
		wait(vazio);
                f = (i+1)%N;
                consome(buffer[i]);
                signal(cheio);
	}
}