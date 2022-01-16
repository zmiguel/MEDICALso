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

void handle_sig(int signo, siginfo_t *info, void *context){
    // SIGUSR1
    /*
    if(signo == 10){
        if(consulta == 0){
            consulta = 1;
            medico_pid = info->si_value.sival_int;
        }
    }
    // SIGINT aka CTRL+C
    if(signo == 2){
        //send info to balcao telling we left
        // send SIGUSR2 with pid to balcao
        if(balcao_pid == 0){
            // we have not talked to balcao yet
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
            exit(1);
        }else if (balcao_pid != 0 && consulta == 0){
            // we have talked to balcao but not medico
            const union sigval val = { .sival_int = getpid() };
            sigqueue(balcao_pid, SIGUSR2, val);
        }else if (balcao_pid != 0 && consulta == 1){
            // we have talked to balcao and medico
            const union sigval val = { .sival_int = getpid() };
            sigqueue(medico_pid, SIGUSR2, val);
        }else{
            // something unexpected happened
            printf("Something unexpected happened\n");
            printf("Exiting...\n");
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
        }
    }
    // SIGUSR2 someone told us they left...
    if(signo == 12){
        // check if balcao
        if(info->si_pid == balcao_pid){
            // balcao told us they are leving... 
            // we must close!
            printf("Balcao terminou...n");
            printf("Exiting...\n");
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
        }
        // check if medico
        if(info->si_pid == medico_pid){
            // medico told us they are leving... 
            // idk what to do here...
            printf("Medico terminou...n");
            printf("Exiting...\n");
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
        }
    }
    */
   if (signo == SIGINT) {
       unlink(SERVER_FIFO_CLIENTES);
   }
}

int max(int a, int b) {
    return (a>b) ? a : b;
}

int contaClientes(int maxClientes, Utente matriz[][maxClientes]) {
    int cont = 0;
    int i,j;

    for (i = 0;i < 5; i++) {
        for (j = 0; j < maxClientes; j++) {
            if(matriz[i][j].especialidade[0] != '\0') {
                cont++;
            }
        }
    }
    printf("Numero de clientes: %d", cont);

    return cont;
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

    // config signal
    struct sigaction action;
    action.sa_sigaction = handle_sig;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1, &action, NULL); // inicial consulta
    sigaction(SIGINT, &action, NULL); // avisar que vamos sair
    // definição de variáveis
    int maxClientes, maxMedicos;
    int numMedicos = 0;
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

    Utente utentes[5][maxClientes];

    // iniciar utentes a zero
    for(i = 0; i < 5; i++) {
        for(j = 0; j < maxClientes; j++) {
            utentes[i][j].especialidade[0] = '\0';
            utentes[i][j].prioridade = 0;
        }
    }

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
    setbuf(stdout, NULL);

    //classificação de especialidade e respetiva prioridade
    while(1) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(& read_fds);
        FD_SET(0, & read_fds);
        FD_SET(s_c_fifo_fd, & read_fds);
        nfd = select(s_c_fifo_fd + 1, & read_fds, NULL, NULL, & tv);
        if (nfd == 0) {
            fflush(stdout);
            continue;
        }
        if (nfd == -1) {
            perror("\nerro no select");
            close(s_c_fifo_fd);
            unlink(SERVER_FIFO_CLIENTES);
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
            C_B buffer;
            int bytes;
            char temp[256];
            int debug_read = 0;

            bytes = read(s_c_fifo_fd, &buffer, sizeof(buffer));
            if (bytes == -1) {
                perror("erro a ler do cliente\n");
                continue;
            }
            if (buffer.tipo == 1) {
                B_U msg_cli;
                char fifo[256];
                sprintf(fifo, "./cliente-%d", buffer.pid);
                int fifo_cliente = open(fifo, O_WRONLY);

                if (contaClientes(maxClientes, utentes) >= maxClientes) {
                    strcpy(msg_cli.msg, "Nao e possivel aceitar mais clientes!");
                    write(fifo_cliente, &msg_cli, sizeof(msg_cli));
                    close(fifo_cliente);
                } else {
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
                    sscanf(temp, "%s %d", msg_cli.especialidade, &msg_cli.prioridade);
                    strcpy(utenteNovo.especialidade, msg_cli.especialidade);
                    utenteNovo.prioridade = msg_cli.prioridade;
                    utenteNovo.pid = buffer.pid;
                    strcpy(utenteNovo.nome, buffer.nome);

                    if(strcmp(msg_cli.especialidade, "geral") == 0 && filas[0] < 5) {
                        utentes[0][filas[0]] = utenteNovo;
                        filas[0]++;
                        msg_cli.num_utentes = filas[0];
                    }
                    else {
                        if(strcmp(msg_cli.especialidade, "ortopedia") == 0 && filas[1] < 5) {
                            utentes[1][filas[1]] = utenteNovo;
                            filas[1]++;
                            msg_cli.num_utentes = filas[1];
                        }
                        else {
                            if(strcmp(msg_cli.especialidade, "estomatologia") == 0 && filas[2] < 5) {
                                utentes[2][filas[2]] = utenteNovo;
                                filas[2]++;
                                msg_cli.num_utentes = filas[2];
                            }
                            else {
                                if(strcmp(msg_cli.especialidade, "neurologia") == 0 && filas[3] < 5) {
                                    utentes[3][filas[3]] = utenteNovo;
                                    filas[3]++;
                                    msg_cli.num_utentes = filas[3];
                                }
                                else {
                                    if(strcmp(msg_cli.especialidade, "oftalmologia") == 0 && filas[4] < 5) {
                                        utentes[4][filas[4]] = utenteNovo;
                                        filas[4]++;
                                        msg_cli.num_utentes = filas[4];
                                    }
                                    else {
                                        printf("Especialidade nao existe ou filas cheias!\n");
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                    msg_cli.tipo = 1;
                    msg_cli.num_especialistas = numMedicos;
                    write(fifo_cliente, &msg_cli, sizeof(msg_cli));
                    close(fifo_cliente);
                }
            }
            if (buffer.tipo == 2) {
                char fifo_med[256];
                sprintf(fifo_med, "./medico-%d", buffer.pid);
                int fifo_medico = open(fifo_med, O_WRONLY);
                B_M msg_med;
                if (numMedicos >= maxMedicos) {
                    strcpy(msg_med.msg, "Nao e possivel receber mais medicos!");
                    msg_med.tipo = -1;
                    write(fifo_medico, &msg_med, sizeof(msg_med));
                    close(fifo_medico);
                } else {
                    strcpy(medicos[numMedicos].nome, buffer.nome);
                    strcpy(medicos[numMedicos].especialidade, buffer.msg);
                    medicos[numMedicos].pid = buffer.pid;
                    numMedicos++;
                    msg_med.pid = getpid();
                    write(fifo_medico, &msg_med, sizeof(msg_med));
                    close(fifo_medico);
                }
            }
        }
    }

    close(s_c_fifo_fd);
    unlink(SERVER_FIFO_CLIENTES);

    return 0;
}