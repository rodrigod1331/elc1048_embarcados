/**
 * \file
 *
 * \brief Exemplos diversos de tarefas e funcionalidades de um sistema operacional multitarefas.
 *
 */

/**
 * \mainpage Sistema operacional multitarefas
 *
 * \par Exemplso de tarefas
 *
 * Este arquivo contem exemplos diversos de tarefas e
 * funcionalidades de um sistema operacional multitarefas.
 *
 *
 * \par Conteudo
 *
 * -# Inclui funcoes do sistema multitarefas (atraves de multitarefas.h)
 * -# Inicialização do processador e do sistema multitarefas
 * -# Criação de tarefas de demonstração
 *
 */

/*
 * Inclusao de arquivos de cabecalhos
 */
#include <asf.h>
#include "stdint.h"
#include "multitarefas.h"


/*
 * Prototipos das tarefas
 */
void tarefa_1(void);
void tarefa_2(void);
void tarefa_3(void);
void tarefa_4(void);
void tarefa_5(void);
void tarefa_imprimir(void);

/*
 * Configuracao dos tamanhos das pilhas
 */
#define TAM_PILHA_1			(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_2			(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_3			(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_4			(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_5			(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_6			(TAM_MINIMO_PILHA + 24)
#define TAM_PILHA_OCIOSA	(TAM_MINIMO_PILHA + 24)

/*
 * Declaracao das pilhas das tarefas
 */
uint32_t PILHA_TAREFA_1[TAM_PILHA_1];
uint32_t PILHA_TAREFA_2[TAM_PILHA_2];
uint32_t PILHA_TAREFA_3[TAM_PILHA_3];
uint32_t PILHA_TAREFA_4[TAM_PILHA_4];
uint32_t PILHA_TAREFA_5[TAM_PILHA_5];
uint32_t PILHA_TAREFA_IMPRIMIR[TAM_PILHA_6];
uint32_t PILHA_TAREFA_OCIOSA[TAM_PILHA_OCIOSA];

uint32_t cont1 = 0, cont2 = 0, cont3 = 0, cont4 = 0, cont5 = 0, soma = 0;

/*
 * Funcao principal de entrada do sistema
 */
int main(void)
{
	system_init();

	/* Criacao das tarefas */
	/* Parametros: ponteiro, nome, ponteiro da pilha, tamanho da pilha, prioridade da tarefa */

	CriaTarefa(tarefa_1, "Tarefa 1", PILHA_TAREFA_1, TAM_PILHA_1, 5); // Maior prioridade

    CriaTarefa(tarefa_2, "Tarefa 2", PILHA_TAREFA_2, TAM_PILHA_2, 4);

    CriaTarefa(tarefa_3, "Tarefa 3", PILHA_TAREFA_3, TAM_PILHA_3, 3);

    CriaTarefa(tarefa_4, "Tarefa 4", PILHA_TAREFA_4, TAM_PILHA_4, 2);

    CriaTarefa(tarefa_5, "Tarefa 5", PILHA_TAREFA_5, TAM_PILHA_5, 1); // Menor Prioridade

    CriaTarefa(tarefa_imprimir, "Tarefa Imprimir", PILHA_TAREFA_IMPRIMIR, TAM_PILHA_6, 6);

    /* Suspende todas as Threads exceto a de maior prioridade (tarefa_1) */

    TCB[1].estado = ESPERA;
    TCB[2].estado = ESPERA;
    TCB[3].estado = ESPERA;
    TCB[4].estado = ESPERA;

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

void tarefa_1()
{
    for (;;)
    {
        cont1++;
        TarefaSuspende(1); // A própria tarefa se suspende
    }
}

void tarefa_2()
{
    for (;;)
    {
        cont2++;
        TarefaContinua(1); // Chama a tarefa de maior prioridade (tarefa_1)
        TarefaSuspende(2); // A própria tarefa se suspende
    }
}

void tarefa_3()
{
    for (;;)
    {
        cont3++;
        TarefaContinua(2); // Chama a tarefa de maior prioridade (tarefa_2)
        TarefaSuspende(3); // A própria tarefa se suspende
    }
}

void tarefa_4()
{
    for (;;)
    {
        cont4++;
        TarefaContinua(3); // Chama a tarefa de maior prioridade (tarefa_3)
        TarefaSuspende(4); // A própria tarefa se suspende
    }
}

void tarefa_5()
{
    for (;;)
    {
        cont5++;
        TarefaContinua(4); // Chama a tarefa de maior prioridade (tarefa_4)
    }
}

void tarefa_imprimir()
{
    for (;;)
    {
        soma = cont1 + cont2 + cont3 + cont4 + cont5;
        TarefaEspera(30);
    }
}
