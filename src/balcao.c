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
        close(0);
        dup(fd_in[0]);
        close(fd_in[0]);
        close(fd_in[1]);

        close(1);
        dup(fd_out[1]);
        close(fd_out[0]);
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
    char sintomas[1000];
    int i, j;

    // iniciar utentes a zero
    for(i = 0; i < 5; i++) {
        for(j = 0; j < 5; j++) {
            utentes[i][j].especialidade[0] = '\0';
            utentes[i][j].prioridade = 0;
        }
    }

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
    int sair_while=0;
    while(sair_while < 3) {
        printf("Indique os sintomas: ");
        fgets(sintomas, sizeof(sintomas)-1, stdin);
        char especialidade[256];
        int prioridade=0;
        char temp[256];
        int debug_read = 0;

        if(strcmp(sintomas, "sair\n") == 0) {
            return 0;
        }
        
        // enviar sintomas ao classificador
        write(to_class, sintomas, strlen(sintomas));
        // receber resposta do classificador
        debug_read = read(from_class, temp, sizeof(temp)-1);
        if(debug_read == -1) {
            printf("erro ao ler do classificador\n");
            return 0;
        }
        temp[debug_read] = '\0';
        // separar resposta
        sscanf(temp, "%s %d", especialidade, &prioridade);
        printf("(%s || %s || %d)\n", temp, especialidade, prioridade);
        
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
        sair_while++;
    }

    return 0;
}