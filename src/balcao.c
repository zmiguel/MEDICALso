#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <signal.h>

#include "util.h"

#define SERVER_FIFO_CLIENTES "np_balcao"
#define CLIENTE_FIFO "cliente-%d"
#define SERVER_FIFO_MEDICOS "np_balcao_medicos"
#define MEDICO_FIFO "np_medico_%d"

int max(int a, int b) {
    return (a>b) ? a : b;
}

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
    Utente utenteNovo;
    int filas[5] = {0, 0, 0, 0, 0};
    char sintomas[1000];
    int i, j, res;
    int s_c_fifo_fd, s_m_fifo_fd;
    fd_set read_fds;
    int nfd;
    struct timeval tv;

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

    Utente utentes[5][5];

    // iniciar utentes a zero
    for(i = 0; i < 5; i++) {
        for(j = 0; j < 5; j++) {
            utentes[i][j].especialidade[0] = '\0';
            utentes[i][j].prioridade = 0;
        }
    }

    Utente clientes[maxClientes];
    Especialista medicos[maxMedicos];

    res = mkfifo(SERVER_FIFO_CLIENTES, 0777);
    if (res == -1) {
        perror("\nmkfifo do FIFO do servidor deu erro");
        exit(EXIT_FAILURE);
    }
    s_c_fifo_fd = open(SERVER_FIFO_CLIENTES, O_RDWR | O_NONBLOCK);
    if (s_c_fifo_fd == -1) {
        perror("\nErro ao abrir o FIFO do servidor (RDWR/non blocking)");
        exit(EXIT_FAILURE);
    }
    res = mkfifo(SERVER_FIFO_MEDICOS, 0777);
    if (res == -1) {
        perror("\nmkfifo do FIFO do servidor deu erro");
        exit(EXIT_FAILURE);
    }
    s_m_fifo_fd = open(SERVER_FIFO_MEDICOS, O_RDWR | O_NONBLOCK);
    if (s_m_fifo_fd == -1) {
        perror("\nErro ao abrir o FIFO do servidor (RDWR/non blocking)");
        exit(EXIT_FAILURE);
    }
    setbuf(stdout, NULL);

    //classificação de especialidade e respetiva prioridade
    while(1) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(& read_fds);
        FD_SET(0, & read_fds);
        FD_SET(s_c_fifo_fd, & read_fds);
        FD_SET(s_m_fifo_fd, & read_fds);
        nfd = select(max(s_c_fifo_fd, s_m_fifo_fd) + 1, & read_fds, NULL, NULL, & tv);
        if (nfd == 0) {
            fflush(stdout);
            continue;
        }
        if (nfd == -1) {
            perror("\nerro no select");
            close(s_c_fifo_fd);
            unlink(SERVER_FIFO_CLIENTES);
            close(s_m_fifo_fd);
            unlink(SERVER_FIFO_MEDICOS);
            return EXIT_FAILURE;
        }

        if (FD_ISSET(0, & read_fds)) {
            /* printf("Indique os sintomas: ");
            fgets(sintomas, sizeof(sintomas)-1, stdin);
            char especialidade[256];
            int prioridade=0;
            char temp[256];
            int debug_read = 0;

            if(strcmp(sintomas, "sair\n") == 0) {
                char sair[] = "#fim\n";
                write(to_class, sair, strlen(sair));
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
            printf("----------\nTodos os utentes:\n");
            for(i=0;i<5;i++) {
                for(j=0;j<5;j++) {
                    if(utentes[i][j].especialidade[0] != '\0') {
                        printf("Especialidade: %s\tPrioridade: %d\n", utentes[i][j].especialidade, utentes[i][j].prioridade);
                    }
                }
            }
            printf("----------\n"); */
        }
        if (FD_ISSET(s_c_fifo_fd, & read_fds)) {
            U_B buffer;
            int bytes;
            B_U msg;
            char temp[256];
            int debug_read = 0;

            bytes = read(s_c_fifo_fd, &buffer, sizeof(buffer));
            if (bytes == -1) {
                perror("erro a ler do cliente\n");
                continue;
            }
            printf("Utente: %s\tMensagem: %s\n", buffer.nome, buffer.msg);
            // enviar sintomas ao classificador
            write(to_class, buffer.msg, strlen(buffer.msg));
            // receber resposta do classificador
            debug_read = read(from_class, temp, sizeof(temp)-1);
            if(debug_read == -1) {
                printf("erro ao ler do classificador\n");
                return 0;
            }
            temp[debug_read] = '\0';
            // separar resposta
            sscanf(temp, "%s %d", msg.especialidade, &msg.prioridade);

            if(strcmp(msg.especialidade, "geral") == 0 && filas[0] < 5) {
                strcpy(utenteNovo.especialidade, msg.especialidade);
                utenteNovo.prioridade = msg.prioridade;
                strcpy(utenteNovo.nome, buffer.nome);
                utentes[0][filas[0]] = utenteNovo;
                filas[0]++;
            }
            else {
                if(strcmp(msg.especialidade, "ortopedia") == 0 && filas[1] < 5) {
                    strcpy(utenteNovo.especialidade, msg.especialidade);
                    utenteNovo.prioridade = msg.prioridade;
                    strcpy(utenteNovo.nome, buffer.nome);
                    utentes[1][filas[1]] = utenteNovo;
                    filas[1]++;
                }
                else {
                    if(strcmp(msg.especialidade, "estomatologia") == 0 && filas[2] < 5) {
                        strcpy(utenteNovo.especialidade, msg.especialidade);
                        utenteNovo.prioridade = msg.prioridade;
                        strcpy(utenteNovo.nome, buffer.nome);
                        utentes[2][filas[2]] = utenteNovo;
                        filas[2]++;
                    }
                    else {
                        if(strcmp(msg.especialidade, "neurologia") == 0 && filas[3] < 5) {
                            strcpy(utenteNovo.especialidade, msg.especialidade);
                            utenteNovo.prioridade = msg.prioridade;
                            strcpy(utenteNovo.nome, buffer.nome);
                            utentes[3][filas[3]] = utenteNovo;
                            filas[3]++;
                        }
                        else {
                            if(strcmp(msg.especialidade, "oftalmologia") == 0 && filas[4] < 5) {
                                strcpy(utenteNovo.especialidade, msg.especialidade);
                                utenteNovo.prioridade = msg.prioridade;
                                strcpy(utenteNovo.nome, buffer.nome);
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

            char fifo[256];
            sprintf(fifo, "./cliente-%d", buffer.pid);
            int fifo_cliente = open(fifo, O_WRONLY);
            msg.tipo = 1;

            write(fifo_cliente, &msg, sizeof(msg));
        }
        if (FD_ISSET(s_m_fifo_fd, & read_fds)) {

        }
    }

    close(s_c_fifo_fd);
    unlink(SERVER_FIFO_CLIENTES);
    close(s_m_fifo_fd);
    unlink(SERVER_FIFO_MEDICOS);

    return 0;
}