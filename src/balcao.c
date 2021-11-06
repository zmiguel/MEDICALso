#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int main(int argc, char **argv){
    int maxClientes, maxMedicos;
    Utente utentes[5][5];
    Utente utenteNovo;
    int filas[5] = {0, 0, 0, 0, 0};
    char especialidade[256];
    int prioridade;
    int i, j;

    //parte das variaveis de ambiente
    if(getenv("MAXCLIENTES") == NULL) {
        printf("Variavel de ambiente MAXCLIENTES nao existe!\n");
        return 0;
    }
    if(getenv("MAXMEDICOS") == NULL) {
        printf("Variavel de ambiente MAXMEDICOS nao existe!\n");
        return 0;
    }
    maxClientes = atoi(getenv("MAXCLIENTES"));
    maxMedicos = atoi(getenv("MAXMEDICOS"));

    //classificacao de especialidade e respetiva prioridade
    while(1) {
        printf("Indique a especialidade: ");
        scanf(" %[^\n]", especialidade);
        printf("Indique a prioridade: ");
        scanf(" %d", &prioridade);
        if(prioridade < 1 || prioridade > 3) {
            printf("Prioridade tem que estar entre 1 e 3!\n");
            continue;
        }
        if(strcmp(especialidade, "geral") == 0 && filas[0] < 5) {
            strcpy(utenteNovo.especialidade, especialidade);
            utenteNovo.prioridade = prioridade;
            strcpy(utenteNovo.nome, "John Doe");
            utentes[0][filas[0]] = utenteNovo;
            filas[0]++;
        }
        else {
            if(strcmp(especialidade, "ortopedia") == 0 && filas[1] < 5) {
                strcpy(utenteNovo.especialidade, especialidade);
                utenteNovo.prioridade = prioridade;
                strcpy(utenteNovo.nome, "John Doe");
                utentes[1][filas[1]] = utenteNovo;
                filas[1]++;
            }
            else {
                if(strcmp(especialidade, "estomatologia") == 0 && filas[2] < 5) {
                    strcpy(utenteNovo.especialidade, especialidade);
                    utenteNovo.prioridade = prioridade;
                    strcpy(utenteNovo.nome, "John Doe");
                    utentes[2][filas[2]] = utenteNovo;
                    filas[2]++;
                }
                else {
                    if(strcmp(especialidade, "neurologia") == 0 && filas[3] < 5) {
                        strcpy(utenteNovo.especialidade, especialidade);
                        utenteNovo.prioridade = prioridade;
                        strcpy(utenteNovo.nome, "John Doe");
                        utentes[3][filas[3]] = utenteNovo;
                        filas[3]++;
                    }
                    else {
                        if(strcmp(especialidade, "oftalmologia") == 0 && filas[4] < 5) {
                            strcpy(utenteNovo.especialidade, especialidade);
                            utenteNovo.prioridade = prioridade;
                            strcpy(utenteNovo.nome, "John Doe");
                            utentes[4][filas[4]] = utenteNovo;
                            filas[4]++;
                        }
                        else {
                            printf("Especialidade nao existe ou filas cheias!\n");
                            continue;
                        }
                    }
                }
            }
        }
        printf("Todos os utentes:\n");
        for(i=0;i<5;i++) {
            for(j=0;j<5;j++) {
                printf("Especialidade: %s\tPrioridade: %d\n", utentes[i][j].especialidade, utentes[i][j].prioridade);
            }
        }
    }

    return 0;
}