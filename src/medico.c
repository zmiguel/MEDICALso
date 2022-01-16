#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "util.h"

// Global variables
int balcao_pid = 0;
int consulta = 0;
int utente_pid = 0;

void handle_sig(int signo, siginfo_t *info, void *context){
    // SIGUSR1
    if(signo == 10){
        if(consulta == 0){
            consulta = 1;
            utente_pid = info->si_value.sival_int;
        }
    }
    // SIGINT aka CTRL+C
    if(signo == 2){
        //send info to balcao telling we left
        // send SIGUSR2 with pid to balcao
        if(balcao_pid == 0){
            // we have not talked to balcao yet
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./medico-%d", getpid());
            unlink(fifo);
            exit(1);
        }else if (balcao_pid != 0 && consulta == 0){
            // we have talked to balcao but not medico
            const union sigval val = { .sival_int = getpid() };
            sigqueue(balcao_pid, SIGUSR2, val);
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./medico-%d", getpid());
            unlink(fifo);
            exit(1);
        }else if (balcao_pid != 0 && consulta == 1){
            // we have talked to balcao and medico
            const union sigval val = { .sival_int = getpid() };
            sigqueue(utente_pid, SIGUSR2, val);
            sigqueue(balcao_pid, SIGUSR2, val);
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./medico-%d", getpid());
            unlink(fifo);
            exit(1);
        }else{
            // something unexpected happened
            printf("Something unexpected happened\n");
            printf("Exiting...\n");
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./medico-%d", getpid());
            unlink(fifo);
            exit(0);
        }
    }
    // SIGUSR2 someone told us they left...
    if(signo == 12){
        // check if balcao
        if(info->si_pid == balcao_pid){
            // balcao told us they are leving... 
            // we must close!
            printf("Balcao terminou...\n");
            printf("Exiting...\n");
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./medico-%d", getpid());
            unlink(fifo);
            exit(0);
        }
        // check if medico
        if(info->si_pid == utente_pid){
            // medico told us they are leving... 
            // idk what to do here...
            printf("cliente terminou...\n");
            consulta = 0;
        }
    }
    if(signo == SIGALRM) {
        C_B msg;
        msg.tipo = 4;
        msg.ts = (unsigned int)time(NULL);
        msg.pid = getpid();
        char *server_fifo = "./np_balcao";
        int fifo_balcao = open(server_fifo, O_WRONLY | O_NONBLOCK);
        write(fifo_balcao, &msg, sizeof(C_B));
        close(fifo_balcao);
        alarm(20);
    }
}

int main(int argc, char **argv){
    // Check if server is running
    if (open("np_balcao", O_RDWR | O_NONBLOCK) == -1 ){
        printf("Servidor não encontrado.\n");
        return 0;
    }
    // Check argv for user name
    if(argc < 3) {
        printf("Nome de utilizador e/ou especialidade não especificados.\n");
        return 0;
    }
    // Config pipes
    int pid = getpid();
    char *fifo = malloc(sizeof(char)*20);
    sprintf(fifo, "./medico-%d", pid);
    printf("%s\n", fifo);
    char *server_fifo = "./np_balcao";
    printf("PID: %d\n", pid);
    mkfifo(fifo, 0666);

    // config signal
    struct sigaction action;
    action.sa_sigaction = handle_sig;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1, &action, NULL); // inicial consulta
    sigaction(SIGUSR2, &action, NULL); // inicial consulta
    sigaction(SIGINT, &action, NULL); // avisar que vamos sair
    sigaction(SIGALRM, &action, NULL);
    setbuf(stdout, NULL);

    // Enviar info para o servidor
    C_B msg;
    msg.pid = pid;
    msg.tipo = 2;
    strcpy(msg.nome, argv[1]);
    strcpy(msg.msg, argv[2]);
    msg.ts = (unsigned int)time(NULL);
    int fifo_balcao = open(server_fifo, O_WRONLY | O_NONBLOCK);
    write(fifo_balcao, &msg, sizeof(C_B));
    close(fifo_balcao);

    // Receber resposta do servidor
    int fifo_medico = open(fifo, O_RDONLY);
    B_M resposta;
    read(fifo_medico, &resposta, sizeof(B_M));
    close(fifo_medico);

    // set balcao pid
    balcao_pid = resposta.pid;
    
    // check if resposta is valid
    if(resposta.tipo == -1) {
        printf("Especialidade cheia... temte mais tarde\n");
        return 0;
    }
    if(resposta.tipo != 1) {
        printf("Resposta inválida\n");
        return 0;
    }

    int sair = 0;
    alarm(20);

    while(sair != 1){
        while(consulta == 0) {
            // Wait for signal
            pause();
        }

        // consulta pronta, ler pipe para obter medico
        // abrir pipe do medico e começar consulta
        char *fifo_cliente = malloc(sizeof(char)*20);
        sprintf(fifo_cliente, "./cliente-%d", utente_pid);

        // make select system for chat with medico

        int nfd;
        fd_set read_fds;
        struct timeval tv;

        int medico = open(fifo, O_RDONLY | O_NONBLOCK);
        int adeus = 0;

        printf("> ");

        do{
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            FD_ZERO(&read_fds);
            FD_SET(0, &read_fds); // stdin
            FD_SET(medico, &read_fds);

            nfd = select(medico+1, &read_fds, NULL, NULL, &tv);
            if(FD_ISSET(0, &read_fds)){
                // stdin
                char msg[1000];
                fflush(stdin);
                scanf(" %999[^\n]", msg);
                if(strcmp(msg, "adeus") == 0){
                    // consulta terminada \
                    Avisar balcao que estamos disponiveis
                    C_B msg;
                    msg.pid = pid;
                    msg.tipo = 3;
                    int fifo_balcao = open(server_fifo, O_WRONLY | O_NONBLOCK);
                    write(fifo_balcao, &msg, sizeof(C_B));
                    close(fifo_balcao);

                    consulta = 0;
                    adeus = 1;
                }else if (strcmp(msg, "sair") == 0){
                    adeus = 1;
                    sair = 1;
                } else {
                    Consulta consulta_msg;
                    consulta_msg.pid = pid;
                    strcpy(consulta_msg.msg, msg);
                    int cliente = open(fifo_cliente, O_WRONLY | O_NONBLOCK);
                    write(cliente, &consulta_msg, sizeof(Consulta));
                    close(cliente);
                    printf("> ");
                    setbuf(stdout, NULL);
                }
                fflush(stdin);
            }
            if(FD_ISSET(medico, &read_fds)){
                // medico
                Consulta resposta_consulta;
                read(medico, &resposta_consulta, sizeof(Consulta));
                printf("%s\n> ", resposta_consulta.msg);
                setbuf(stdout, NULL);
            }
            
        }while(adeus!=1);

        close(medico);

    }

    // time to leave
    unlink(fifo);

    return 0;
}