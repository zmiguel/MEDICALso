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
int medico_pid = 0;

void handle_sig(int signo, siginfo_t *info, void *context){
    // SIGUSR1
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
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
            exit(1);
        }else if (balcao_pid != 0 && consulta == 1){
            // we have talked to balcao and medico
            const union sigval val = { .sival_int = getpid() };
            sigqueue(medico_pid, SIGUSR2, val);
            sigqueue(balcao_pid, SIGUSR2, val);
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
            exit(1);
        }else{
            // something unexpected happened
            printf("Something unexpected happened\n");
            printf("Exiting...\n");
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
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
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
            exit(0);
        }
        // check if medico
        if(info->si_pid == medico_pid){
            // medico told us they are leving... 
            // idk what to do here...
            printf("Medico terminou...\n");
            printf("Exiting...\n");
            char *fifo = malloc(sizeof(char)*20);
            sprintf(fifo, "./cliente-%d", getpid());
            unlink(fifo);
            exit(0);
        }
    }
}

int main(int argc, char **argv){
    // Check if server is running
    if (open("np_balcao", O_RDWR | O_NONBLOCK) == -1 ){
        printf("Servidor n??o encontrado.\n");
        return 0;
    }
    // Check argv for user name
    if(argc < 2) {
        printf("Nome de utilizador n??o especificado.\n");
        return 0;
    }
    // Config pipes
    int pid = getpid();
    char *fifo = malloc(sizeof(char)*20);
    sprintf(fifo, "./cliente-%d", pid);
    printf("%s\n", fifo);
    char *server_fifo = "./np_balcao";
    printf("PID: %d\n", pid);
    mkfifo(fifo, 0666);
    // Perguntar sintomas
    printf("Introduza os sintomas do seu problema separados por espa??os: ");
    char sintomas[1000];
    fgets(sintomas, sizeof(sintomas)-1, stdin);

    // config signal
    struct sigaction action;
    action.sa_sigaction = handle_sig;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1, &action, NULL); // inicial consulta
    sigaction(SIGUSR2, &action, NULL); // inicial consulta
    sigaction(SIGINT, &action, NULL); // avisar que vamos sair

    // Enviar sintomas para o servidor
    C_B msg;
    msg.pid = pid;
    msg.tipo = 1;
    strcpy(msg.nome, argv[1]);
    strcpy(msg.msg, sintomas);
    int fifo_balcao = open(server_fifo, O_WRONLY | O_NONBLOCK);
    write(fifo_balcao, &msg, sizeof(C_B));
    close(fifo_balcao);

    // Receber resposta do servidor
    int fifo_cliente = open(fifo, O_RDONLY);
    B_U resposta;
    read(fifo_cliente, &resposta, sizeof(B_U));
    close(fifo_cliente);

    // set balcao pid
    balcao_pid = resposta.pid;
    
    // check if resposta is valid
    if(resposta.tipo == -1) {
        printf("N??o existem m??dicos dispon??veis.\n");
        return 0;
    }
    if(resposta.tipo != 1) {
        printf("Resposta inv??lida\n");
        return 0;
    }

    // Parse resposta
    int prioridade = resposta.prioridade;
    char *especialidade = resposta.especialidade;
    int num_medicos = resposta.num_especialistas;
    int fila_de_espera = resposta.num_utentes;
    printf("Prioridade: %d para a especialidade: %s\n", prioridade, especialidade);
    printf("N??mero de m??dicos dispon??veis: %d\n", num_medicos);
    printf("N??mero de utentes na fila de espera: %d\n", fila_de_espera);
    printf("A aguardar ser chamado para consulta...\n");

    while(consulta == 0) {
        // Wait for signal
        pause();
    }
    setbuf(stdout, NULL);

    // consulta pronta, ler pipe para obter medico
    // abrir pipe do medico e come??ar consulta
    char *fifo_medico = malloc(sizeof(char)*20);
    sprintf(fifo_medico, "./medico-%d", medico_pid);

    // make select system for chat with medico

    int nfd;
    fd_set read_fds;
    struct timeval tv;

    int cliente = open(fifo, O_RDONLY | O_NONBLOCK);
    int sair = 0;

    printf("> ");

    do{
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds); // stdin
        FD_SET(cliente, &read_fds);

        nfd = select(cliente+1, &read_fds, NULL, NULL, &tv);
        if(FD_ISSET(0, &read_fds)){
            // stdin
            char msg[1000];
            fflush(stdin);
            scanf(" %999[^\n]", msg);
            if(strcmp(msg, "adeus") == 0){
                sair = 1;
            }
            else{
                Consulta consulta_msg;
                consulta_msg.pid = pid;
                strcpy(consulta_msg.msg, msg);
                int medico = open(fifo_medico, O_WRONLY | O_NONBLOCK);
                write(medico, &consulta_msg, sizeof(Consulta));
                close(medico);
                printf("> ");
                setbuf(stdout, NULL);
            }
            fflush(stdin);
        }
        if(FD_ISSET(cliente, &read_fds)){
            // medico
            Consulta resposta_consulta;
            read(cliente, &resposta_consulta, sizeof(Consulta));
            printf("%s\n> ", resposta_consulta.msg);
            setbuf(stdout, NULL);
        }
        
    }while(sair!=1);

    // time to leave
    close(cliente);
    unlink(fifo);

    return 0;
}