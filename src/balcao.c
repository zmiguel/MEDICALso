#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "util.h"

int main(int argc, char **argv, char **envp) {
    // configurar e lançar programa classificador
    int fd_in[2], fd_out[2];
    pipe(fd_in);
    pipe(fd_out);
    int to_class = fd_in[1];
    int from_class = fd_out[0];
    if(fork() == 0) {
        // in child
        // close input side (0) of out
        // close output side (1) of in
        close(fd_in[1]);
        close(fd_out[0]);
        dup2(fd_in[0], STDIN_FILENO);
        dup2(fd_out[1], STDOUT_FILENO);
        close(fd_in[0]);
        close(fd_out[1]);
        execl("./classificador", "./classificador", NULL);
    }else{
        // in parent
        // close 1 of in
        // close 0 of out
        close(fd_in[0]);
        close(fd_out[1]);
    }
    // definição de variáveis
    int maxClientes, maxMedicos;
    Utente utentes[5][5];
    Utente utenteNovo;
    int filas[5] = {0, 0, 0, 0, 0};
    char sintomas[256];
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

    //classificação de especialidade e respetiva prioridade
    while(1) {
        printf("Indique os sintomas: ");
        scanf(" %[^\n]", sintomas);
        char especialidade[256];
        int prioridade=0;
        char temp[256];

        if(strcmp(sintomas, "sair") == 0) {
            break;
        }

        // enviar sintomas ao classificador
        write(to_class, sintomas, sizeof(sintomas));
        // receber resposta do classificador
        read(from_class, &temp, sizeof(temp));
        printf("%s\n", temp);
        // separar resposta
        sscanf(temp, "%s %d", especialidade, &prioridade);

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