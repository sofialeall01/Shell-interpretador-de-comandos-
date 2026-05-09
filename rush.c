/* TRABALHO SISTEMAS OPERACIONAIS 
Kaua Teixeira Nascimento
Sofia Maria de Jesus Leal
*/

/* CODIGOS:
    make # executa o Makefile e cria um executavel
    ./rush # roda o executavel no terminal
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_LINHA 1024
#define MAX_ARGS 100
#define MAX_CMDS 20

/*
Funçao que recebe a linha original de comandos e realiza a quebra em varios tokens de acordo
com o divisor 

parametros:
- char *linha: ponteiro para a string original de comandos a ser separa em tokens
- char *partes[]: vetor de ponteiros onde serão armazenados os endereços dos tokens 
- char *divisor: ponteiro para string contendo os caracteres que separam os tokens 
*/
int separa_tokens(char *linha, char *partes[], char *divisor) {
    int count = 0;
    char *token = strtok(linha, divisor);

    while (token != NULL) {
        partes[count++] = token;
        token = strtok(NULL, divisor);
    }
    partes[count] = NULL;
    return count;
}

/*
Funçao que analisa e interpreta m comando digitado pelo usuário,
separando o nome do programa, seus argumentos e os
possíveis redirecionamentos de entrada ('<') e saída ('>').

parametros:
- char *comando: ponteiro para a string contendo um comando individual de pipeline
- char **args[]: vetor de ponteiros preechido pelos tokens da linha de comando
- char **input: ponteiro para ponteiro que guarda o nome do arquivo de entrada
- char **output: ponteiro para ponteiro que guarda o nome do arquivo de saida
*/
void separar_comandos(char *comando, char **args, char **input, char **output) {
    *input = NULL;
    *output = NULL;

    char *tokens[MAX_ARGS];
    int n = separa_tokens(comando, tokens, " \t\n");

    int j = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            *input = tokens[++i];
        }
        else if (strcmp(tokens[i], ">") == 0) {
            *output = tokens[++i];
        }
        else {
            args[j++] = tokens[i];
        }
    }
    args[j] = NULL;
}

/*
Função responsavel por executar uma linha de comandos digitada pelo usuário 
parametros:
- char *linha: ponteiro para a string contendo a linha de comando completa digitada pelo usuário    
*/
void execute_pipeline(char *linha) {
    char *cmds[MAX_CMDS];

    int n_cmds = separa_tokens(linha, cmds, "|");

    int pipe_anterior = -1;

    for (int i = 0; i < n_cmds; i++) {
        int fd[2];
        // o vetor fd[2] armazena os descritores de arquivo de um pipe,
        //fd[0] ponta de leitura, fd[1] ponta de escrita

        if (i < n_cmds - 1) {
            pipe(fd);
        }

        pid_t pid = fork();

        if (pid == 0) {
            // ===== FILHO =====

            char *args[MAX_ARGS];
            char *input = NULL;
            char *output = NULL;

            separar_comandos(cmds[i], args, &input, &output);

            // Entrada do pipe anterior
            if (pipe_anterior != -1) {
                dup2(pipe_anterior, STDIN_FILENO);
                //STDIN_FILENO: descritor de arquivo de entrada padrão
                close(pipe_anterior);
            }

            // Saída para próximo pipe
            if (i < n_cmds - 1) {
                dup2(fd[1], STDOUT_FILENO);
                //STDOUT_FILENO: descritor de saida padrão
                close(fd[0]);
                close(fd[1]);
            }

            // Redirecionamento de entrada <
            if (input != NULL) {
                int in_fd = open(input, O_RDONLY);
                //O_RDONLY: abrir arquivo para leitura
                if (in_fd < 0) {
                    perror("Erro ao abrir arquivo de entrada");
                    exit(1);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // Redirecionamento de saída >
            if (output != NULL) {
                int out_fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                //O_WRONLY: abrir arquivo para escrita, O_CREAT: criar arquivo, O_TRUNC: apagar conteúdo
                if (out_fd < 0) {
                    perror("Erro ao abrir arquivo de saída");
                    exit(1);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            execvp(args[0], args);
            perror("Erro no execvp");
            exit(1);
        }

        // ===== PAI =====
        if (pipe_anterior != -1) {
            close(pipe_anterior);
        }

        if (i < n_cmds - 1) {
            close(fd[1]);
            pipe_anterior = fd[0];
        }
    }

    // Espera todos os filhos
    for (int i = 0; i < n_cmds; i++) {
        wait(NULL);
    }
}

int main() {
    char linha[MAX_LINHA];

    while (1) {
        printf("Digite o comando: ");
        fflush(stdout);

        if (fgets(linha, MAX_LINHA, stdin) == NULL)
            break;

        linha[strcspn(linha, "\n")] = 0;

        if (strncmp(linha, "exit", 4) == 0)
            break;

        if (strlen(linha) == 0)
            continue;

        execute_pipeline(linha);
    }

    return 0;
}
