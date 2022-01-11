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

int main(int argc, char **argv){
    // Check if server is running
    if (open("np_balcao", O_RDWR) == -1 ){
        printf("Servidor não encontrado.\n");
        return 0;
    }
    // Check argv for user name
    if(argc < 2) {
        printf("Nome de utilizador não especificado.\n");
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
    printf("Introduza os sintomas do seu problema separados por espaços: ");
    char sintomas[1000];
    scanf("%s", sintomas);

    // Enviar sintomas para o servidor
    U_B msg;
    msg.pid = pid;
    msg.tipo = 1;
    strcpy(msg.nome, argv[1]);
    strcpy(msg.msg, sintomas);
    int fifo_balcao = open(server_fifo, O_WRONLY);
    write(fifo_balcao, &msg, sizeof(U_B));

    // Receber resposta do servidor
    int fifo_cliente = open(fifo, O_RDONLY);
    B_U resposta;
    read(fifo_cliente, &resposta, sizeof(B_U));
    
    // check if resposta is valid
    if(resposta.tipo == -1) {
        printf("Não existem médicos disponíveis.\n");
        return 0;
    }
    if(resposta.tipo != 1) {
        printf("Resposta inválida\n");
        return 0;
    }

    // Parse resposta
    int prioridade = resposta.prioridade;
    char *especialidade = resposta.especialidade;
    printf("Prioridade: %d para a especialidade: %s\n", prioridade, especialidade);



    return 0;
}