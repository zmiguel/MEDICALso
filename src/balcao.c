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
#include <time.h>

#include "util.h"

#define SERVER_FIFO_CLIENTES "np_balcao"

int maxClientes=0, maxMedicos=0;
Utente *utentes;
Especialista *medicos;

void handle_sig(int signo, siginfo_t *info, void *context){
    // SIGINT aka CTRL+C
    int i = 0;
    if(signo == 2){
        // let everyone know we left
        for(i=0; i<maxClientes; i++){
            if(utentes[i].pid != 0){
                const union sigval val = { .sival_int = getpid() };
                sigqueue(utentes[i].pid, SIGUSR2, val);
            }
        }
        for(i=0; i<maxMedicos; i++){
            if(medicos[i].pid != 0){
                const union sigval val = { .sival_int = getpid() };
                sigqueue(medicos[i].pid, SIGUSR2, val);
            }
        }
        // close our pipe
        unlink(SERVER_FIFO_CLIENTES);
        exit(1);
    }
    // SIGUSR2 someone told us they left...
    if(signo == 12){
        // check if cliente
        for(i=0; i<maxClientes; i++){
            if(utentes[i].pid != 0 && utentes[i].pid == info->si_pid){
                // cliente told us they are leving...
                utentes[i].pid = 0;
                utentes[i].especialidade[0] = '\0';
            }
        }
        // check if medico
        for(i=0; i<maxMedicos; i++){
            if(medicos[i].pid != 0 && medicos[i].pid == info->si_pid){
                // medico told us they are leving...
                medicos[i].pid = 0;
                medicos[i].especialidade[0] = '\0';
            }
        }
    }
    // sigalarm for timeout
    if(signo == 14){
        for(i=0; i<maxMedicos; i++){
            if(medicos[i].pid != 0 && (unsigned int)time(NULL) - medicos[i].ts > 20){
                // medico timed out
                medicos[i].pid = 0;
                medicos[i].especialidade[0] = '\0';
            }
        }
        // look for a cliente to consult by priority
        int p;
        int got_medico = 0;
        for(p=3; p>0; p--){
            for(i=0; i<maxClientes; i++){
                if(utentes[i].pid != 0 && utentes[i].especialidade[0] != '\0' && utentes[i].prioridade == p){
                    // cliente found
                    // look for a free medico
                    int j;
                    for(j=0; j<maxMedicos; j++){
                        if(medicos[j].pid != 0 && strcmp(medicos[j].especialidade, utentes[i].especialidade) == 0){
                            // medico found
                            // send a signal to medico
                            const union sigval val = { .sival_int = utentes[i].pid };
                            sigqueue(medicos[j].pid, SIGUSR1, val);
                            medicos[j].em_consulta = 1;
                            // send signal to cliente
                            const union sigval val2 = { .sival_int = medicos[j].pid };
                            sigqueue(utentes[i].pid, SIGUSR1, val2);
                            utentes[i].em_consulta = 1;
                            // set the medico timestamp
                            medicos[j].ts = (unsigned int)time(NULL);
                            got_medico = 1;
                            break;
                        }
                    }
                }
            }
        }
        alarm(10);
    }
}

int max(int a, int b) {
    return (a>b) ? a : b;
}

int contaClientes(int maxClientes, Utente *utentes) {
    int cont = 0;
    int i;

    for (i = 0; i < maxClientes; i++) {
        if(utentes[i].especialidade[0] != '\0') {
            cont++;
        }
    }

    return cont;
}

int somaFilas(int *array) {
    int cont = 0;
    int i;

    for(i = 0; i< 5; i++) {
        cont = array[i];
    }

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
    sigaction(SIGINT, &action, NULL); // avisar que vamos sair
    sigaction(SIGUSR2, &action, NULL); // Avisaram que sairam
    sigaction(SIGALRM, &action, NULL); // Avisaram que sairam
    alarm(10);
    // definição de variáveis
    int numMedicos = 0;
    Utente utenteNovo;
    int filas[5] = {0, 0, 0, 0, 0};
    char sintomas[1000];
    int i, j, res;
    int s_c_fifo_fd, s_m_fifo_fd;
    int freq = 30;
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

    // alocar space for utentes & medicos
    utentes = malloc(sizeof(Utente)*maxClientes);
    medicos = malloc(sizeof(Especialista)*maxMedicos);

    // iniciar utentes a zero
    for(i = 0; i < maxClientes; i++) {
        utentes[i].pid = 0;
        utentes[i].especialidade[0] = '\0';
        utentes[i].em_consulta = 0;
        utentes[i].prioridade = 0;
    }

    // iniciar medicos a zero
    for(i = 0; i < maxMedicos; i++) {
        medicos[i].pid = 0;
        medicos[i].em_consulta = 0;
        medicos[i].ts = 0;
        medicos[i].especialidade[0] = '\0';
    }

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

        if (FD_ISSET(0, & read_fds)) {
            char comando[256];
            scanf(" %255[^\n]", comando);
            if (strcmp(comando, "utentes") == 0) {
                int i, j;
                printf("Utentes\n\n");
                for (i=0;i<maxClientes;i++) {
                    if (utentes[i].especialidade[0] != '\0') {
                        printf("Nome: %s\tEspecialidade: %s\tPrioridade: %d\n", utentes[i].nome, utentes[i].especialidade, utentes[i].prioridade);
                    }
                }
            } else if (strcmp(comando, "especialistas") == 0) {
                int i;
                printf("Especialistas\n\n");
                for (i=0;i<maxMedicos;i++){
                    if(medicos[i].especialidade[0] != '\0'){
                        printf("Nome: %s\tEspecialidade: %s\n", medicos[i].nome, medicos[i].especialidade);
                    }
                }
            } else if (strcmp(comando, "encerra") == 0) {
                printf("A terminar o sistema...\n");
                write(to_class, "sair", strlen("sair"));
                close(s_c_fifo_fd);
                unlink(SERVER_FIFO_CLIENTES);
                for (i=0;i<maxClientes;i++){
                    if (utentes[i].especialidade[0] != '\0') {
                        kill(utentes[i].pid, SIGUSR2);
                    }
                }
                for (i=0; i<maxMedicos; i++) {
                    kill(medicos[i].pid, SIGUSR2);
                }

                return 0;
            } else {
                char *com, *resto, *contexto;
                com = strtok_r(comando, " ", &contexto);
                resto = contexto;
                printf("Comando: %s\tResto: %s\n", com, resto);
                if (strcmp(com, "delut") == 0) {
                    for (i = 0;i< maxClientes; i++) {
                        if(strcmp(resto, utentes[i].nome) == 0) {
                            kill(utentes[i].pid, SIGUSR2);
                            utentes[i].especialidade[0] = '\0';
                            printf("Cliente eliminado!\n");
                        }
                    }
                } else if (strcmp(com, "delesp") == 0) {
                    for (i = 0;i< maxMedicos; i++) {
                        if(strcmp(resto, medicos[i].nome) == 0) {
                            kill(medicos[i].pid, SIGUSR2);
                            medicos[i].especialidade[0] = '\0';
                            printf("Especialista eliminado!\n");
                        }
                    }
                    numMedicos--;
                }else if (strcmp(com, "freq") == 0) { 
                    int nova = atoi(resto);
                    freq = nova;
                } else {
                    printf("Comando invalido!\n");
                }
            }
        }
        if (FD_ISSET(s_c_fifo_fd, & read_fds)) {
            C_B buffer;
            int bytes;
            char temp[256];
            int debug_read = 0;

            bytes = read(s_c_fifo_fd, &buffer, sizeof(C_B));
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
                    write(fifo_cliente, &msg_cli, sizeof(B_U));
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

                    // find empty slot in utentes
                    int utente_slot;
                    for (i = 0; i < maxClientes; i++) {
                        if (utentes[i].pid == 0 && utentes[i].especialidade[0] == '\0') {
                            utente_slot = i;
                            break;
                        }
                    }

                    // atribuir especialidade
                    if(strcmp(msg_cli.especialidade, "geral") == 0 && filas[0] < 5) {
                        utentes[utente_slot]= utenteNovo;
                        filas[0]++;
                        msg_cli.num_utentes = filas[0];
                    }
                    else {
                        if(strcmp(msg_cli.especialidade, "ortopedia") == 0 && filas[1] < 5) {
                            utentes[utente_slot] = utenteNovo;
                            filas[1]++;
                            msg_cli.num_utentes = filas[1];
                        }
                        else {
                            if(strcmp(msg_cli.especialidade, "estomatologia") == 0 && filas[2] < 5) {
                                utentes[utente_slot] = utenteNovo;
                                filas[2]++;
                                msg_cli.num_utentes = filas[2];
                            }
                            else {
                                if(strcmp(msg_cli.especialidade, "neurologia") == 0 && filas[3] < 5) {
                                    utentes[utente_slot] = utenteNovo;
                                    filas[3]++;
                                    msg_cli.num_utentes = filas[3];
                                }
                                else {
                                    if(strcmp(msg_cli.especialidade, "oftalmologia") == 0 && filas[4] < 5) {
                                        utentes[utente_slot] = utenteNovo;
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
                    write(fifo_cliente, &msg_cli, sizeof(B_U));
                    close(fifo_cliente);
                }
            }
            // medico
            if (buffer.tipo == 2) {
                char fifo_med[256];
                sprintf(fifo_med, "./medico-%d", buffer.pid);
                int fifo_medico = open(fifo_med, O_WRONLY);
                B_M msg_med;
                if (numMedicos >= maxMedicos) {
                    strcpy(msg_med.msg, "Nao e possivel receber mais medicos!");
                    msg_med.tipo = -1;
                    write(fifo_medico, &msg_med, sizeof(B_M));
                    close(fifo_medico);
                } else {
                    strcpy(medicos[numMedicos].nome, buffer.nome);
                    strcpy(medicos[numMedicos].especialidade, buffer.msg);
                    medicos[numMedicos].pid = buffer.pid;
                    numMedicos++;
                    msg_med.pid = getpid();
                    msg_med.tipo = 1;
                    write(fifo_medico, &msg_med, sizeof(B_M));
                    close(fifo_medico);
                }
            }
            // medico terminou consulta
            if(buffer.tipo == 3) {
                int i;
                for (i = 0; i < maxMedicos; i++) {
                    if(medicos[i].pid == buffer.pid) {
                        medicos[i].em_consulta = 0;
                        printf("Especialista Terminou consulta!\n");
                    }
                }
                numMedicos--;
            }
            // get time
            if(buffer.tipo == 4) {
                // get timestamp from medico
                for(i=0; i<maxMedicos; i++) {
                    if(medicos[i].pid == buffer.pid) {
                        medicos[i].ts = buffer.ts;
                        //printf("Timestamp do medico %d: %d\n", buffer.pid, buffer.ts);
                    }
                }
            }
        }
    }

    close(s_c_fifo_fd);
    unlink(SERVER_FIFO_CLIENTES);

    return 0;
}