#include<stdio.h>
#include<stdlib.h>
#include<string.h>


typedef struct {
	char nome[20];
	float trabalho;
	float prova;
	float classificacao_final;
	char situacao[10];
} registo;

void regista_aluno_1(registo *aluno){

	char nome[20];

	// Regista o nome do aluno
	printf("Digite o nome do aluno 1.\n");
	scanf("%s",nome);
	strcpy(aluno->nome,nome);

	// Regista as notas do aluno
	aluno->trabalho = 10.5;
	aluno->prova = 9.5;
}


void regista_aluno_2(registo *aluno){

	char nome[20];

	// Regista o nome do aluno
	printf("Digite o nome do aluno 2.\n");
	scanf("%s",nome);
	strcpy(aluno->nome,nome);

	// Regista as notas do aluno
	srand(2); // Fixa a semente. Neste caso, poderia ser omitido.
	aluno->trabalho = 0.5 * (rand() % 41);
	aluno->prova = 0.5 * (rand() % 41);
}

void classifica_aluno(registo *aluno){

	// Calcula a classificacao final do aluno
	int prova = aluno->prova;
	int trabalho = aluno->trabalho;
	aluno->classificacao_final = (prova + trabalho)/2;

	// Define a situacao do aluno
	if (aluno->classificacao_final <10)
		strcpy(aluno->situacao,"Reprovado");
	else
		strcpy(aluno->situacao,"Aprovado");	


}

int main(){

	registo *aluno_1;
	aluno_1 = (registo *)  malloc(sizeof(registo));

	registo *aluno_2;
	aluno_2 = (registo *) malloc(sizeof(registo));

	regista_aluno_1(aluno_1);
	regista_aluno_2(aluno_2);

	classifica_aluno(aluno_1);
	classifica_aluno(aluno_2);


	printf("A classificacao do aluno %s e: %f.\n", aluno_1->nome, aluno_1->classificacao_final);
	printf("A situacao do aluno %s e: %s.\n\n", aluno_1->nome, aluno_1->situacao);

	printf("A classificacao do aluno %s e: %f.\n", aluno_2->nome, aluno_2->classificacao_final);
	printf("A situacao do aluno %s e: %s.\n\n", aluno_2->nome, aluno_2->situacao);

	free(aluno_1);
	free(aluno_2);

	return 0;
}
