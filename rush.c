/* trabalho pratico sistemas operacionais 
 * kaua teixeira nascimento e sofia maria de jesus leal
 */

// comandos pra testar:
// make
// ./rush

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

// limitadores do projeto
#define MAX_LINHA 1024
#define MAX_ARGS 100
#define MAX_CMDS 20

/*
funçao que recebe a linha original de comandos e realiza a quebra em varios pedacinhos de acordo
com o divisor 
*/
int cortar_linha(char *linha, char *partes[], char *divisores) {
    int qtd = 0;
    char *pedacinho = strtok(linha, divisores);

    while (pedacinho != NULL) {
        partes[qtd] = pedacinho;
        qtd++;
        pedacinho = strtok(NULL, divisores);
    }
    partes[qtd] = NULL;
    return qtd;
}

/*
funçao que vai interpretar a string que o usuario digitou,,
separando o nome do programa, seus argumentos e 
se tem arquivos de entrada ou saida.
*/
void arruma_argumentos(char *comando_puro, char *argumentos[], char **arq_entrada, char **arq_saida) {
    *arq_entrada = NULL;
    *arq_saida = NULL;

    char *comandos[MAX_ARGS];
    int total_comandos = cortar_linha(comando_puro, comandos, " \t\n");

    int pos = 0;
    for (int i = 0; i < total_comandos; i++) {
        // acha arquivo de entrada
        if (strcmp(comandos[i], "<") == 0) {
            i++; 
            *arq_entrada = comandos[i];
        }
        // acha arquivo de saida
        else if (strcmp(comandos[i], ">") == 0) {
            i++;
            *arq_saida = comandos[i];
        }
        // se nao for nenhum, salva como argumento pro execvp
        else {
            argumentos[pos] = comandos[i];
            pos++;
        }
    }
    argumentos[pos] = NULL; // ultimo argumento tem q ser nulo
}


// funçao principal que cria os filhos e interliga eles
void principal(char *linha_digitada) {
    char *lista_argumentos[MAX_CMDS];

    // primeiro a gente divide tudo que tem pipe
    int total_argumentos = cortar_linha(linha_digitada, lista_argumentos, "|");

    int pipe_anterior = -1; // guarda a ponta de leitura do pipe anterior

    for (int i = 0; i < total_argumentos; i++) {
        int pipe_novo[2];
        // pipe_novo[0] ler, pipe_novo[1] escrever

        // so cria pipe novo se nao for o ultimo comando da fila
        if (i < total_argumentos - 1) {
            pipe(pipe_novo);
        }

        pid_t pid_filho = fork();

        if (pid_filho == 0) {
            // == area do processo filho ==
            // printf("teste: processo filho criado\n");

            char *args_comando[MAX_ARGS];
            char *entrada = NULL;
            char *saida = NULL;

            arruma_argumentos(lista_argumentos[i], args_comando, &entrada, &saida);

            // puxa os dados do processo anterior
            if (pipe_anterior != -1) {
                dup2(pipe_anterior, 0); // 0 eh a entrada padrao (stdin)
                close(pipe_anterior);
            }

            // joga os dados pro proximo processo
            if (i < total_argumentos - 1) {
                dup2(pipe_novo[1], 1); // 1 eh a saida padrao (stdout)
                close(pipe_novo[0]); 
                close(pipe_novo[1]);
            }

            // ler arquivo do disco (<)
            if (entrada != NULL) {
                int fd_in = open(entrada, O_RDONLY);
                if (fd_in < 0) {
                    perror("erro ao abrir arquivo pra ler");
                    exit(1);
                }
                dup2(fd_in, 0);
                close(fd_in);
            }

            // gravar no disco (>)
            if (saida != NULL) {
                // abre arquivo, cria se n existir, limpa se existir
                int fd_out = open(saida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("erro ao tentar criar arquivo de saida");
                    exit(1);
                }
                dup2(fd_out, 1);
                close(fd_out);
            }

            // faz a substituicao e vira o programa q o usuario digitou
            execvp(args_comando[0], args_comando);
            perror("deu erro no comando ou ele nao existe");
            exit(1);
        }

        // == area do processo pai (shell) ==
        
        if (pipe_anterior != -1) {
            close(pipe_anterior); // fecha o pipe antigo q o filho ja usou pra nao travar
        }

        if (i < total_argumentos - 1) {
            close(pipe_novo[1]); // pai nao vai escrever nada
            pipe_anterior = pipe_novo[0]; // salva o bocal de leitura pro proximo filho usar
        }
    }

    // espera todos os filhos acabarem antes de imprimir o prompt dnv
    for (int i = 0; i < total_argumentos; i++) {
        wait(NULL);
    }
}

int main() {
    char texto_usuario[MAX_LINHA];

    // loop infinito do shell
    while (1) {
        printf("Digite comando: ");
        fflush(stdout); // limpa o buffer pra imprimir na hora

        // le do teclado, se o cara apertar ctrl+d sai
        if (fgets(texto_usuario, MAX_LINHA, stdin) == NULL) {
            break;
        }

        // tira o enter q fica no final da string
        texto_usuario[strcspn(texto_usuario, "\n")] = 0;

        // condicao de parada do shell
        if (strncmp(texto_usuario, "exit", 4) == 0) {
            break;
        }

        // se a pessoa der enter vazio nao faz nada e repete o loop
        if (strlen(texto_usuario) == 0) {
            continue;
        }

        principal(texto_usuario);
    }

    return 0;
}