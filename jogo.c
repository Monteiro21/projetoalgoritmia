//Standard includes
#include <stdio.h>
#include <stdlib.h>
//String functionalities library
#include <string.h>
//OS Time include
#include <time.h>
//POSIX dir includes
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
// Errno thread-safe symbol, set by the thread-local command *__errno() where __errno() returns address of the thread-local symbol;
#include <errno.h>
// Character formatting
#include <ctype.h>
// Bool
#include <stdbool.h>

#include "jogo.h"

#define ANSI_RESET_SCHEME "\x1b[0m"
#define ANSI_ERROR "\x1b[31m"
#define ANSI_SUCCESS "\x1b[32m"
#define ANSI_TABLE_COLOR "\x1b[36m"

#define ERROR_MSG(x) ANSI_ERROR x ANSI_RESET_SCHEME
#define SUCCESS_MSG(x) ANSI_SUCCESS x ANSI_RESET_SCHEME

#ifdef RUN_TESTS
extern int call_tests(void);
#endif

bool casasBrancasEstaoConectadas(char **tabuleiro, int linhas, int colunas) {
    if (linhas == 0 || colunas == 0) return true;
    
    Posicao inicio = {-1, -1};
    int contadorBrancas = 0;
    
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            if (isupper(tabuleiro[i][j])) {
                contadorBrancas++;
                if (inicio.linha == -1) {
                    inicio.linha = i;
                    inicio.coluna = j;
                }
            }
        }
    }
    
    if (contadorBrancas == 0) return true;
    
    bool **visitado = malloc(linhas * sizeof(bool *));
    for (int i = 0; i < linhas; i++) {
        visitado[i] = calloc(colunas, sizeof(bool));
    }
    
    Posicao *fila = malloc(linhas * colunas * sizeof(Posicao));
    int frente = 0;
    int tras = 0;
    
    fila[tras++] = inicio;
    visitado[inicio.linha][inicio.coluna] = true;
    int contadorConectadas = 1;
    
    int direcoes[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    while (frente < tras) {
        Posicao atual = fila[frente++];
        
        for (int i = 0; i < 4; i++) {
            int novaLinha = atual.linha + direcoes[i][0];
            int novaColuna = atual.coluna + direcoes[i][1];
            
            if (novaLinha >= 0 && novaLinha < linhas && novaColuna >= 0 && novaColuna < colunas) {
                if (!visitado[novaLinha][novaColuna] && isupper(tabuleiro[novaLinha][novaColuna])) {
                    visitado[novaLinha][novaColuna] = true;
                    fila[tras++] = (Posicao){novaLinha, novaColuna};
                    contadorConectadas++;
                }
            }
        }
    }
    
    for (int i = 0; i < linhas; i++) {
        free(visitado[i]);
    }
    free(visitado);
    free(fila);
    
    return contadorConectadas == contadorBrancas;
}

char leLetra(const char *mensagem) {
    char letra;
    printf("%s", mensagem);
    while (1) {
        letra = getchar();
        if (letra != '\n' && letra != ' ') {
            while (getchar() != '\n'); 
            return letra;
        }
    }
}

int leInteiro(const char *mensagem) {
    int valor;
    char c;
    while (1) {
        printf("%s", mensagem);
        if (scanf("%d", &valor) == 1) {
            while ((c = getchar()) != '\n' && c != EOF); // limpa buffer
            return valor;
        } else {
            printf("Entrada inválida. Tente novamente.\n");
            while ((c = getchar()) != '\n' && c != EOF); // limpa buffer
        }
    }
}

char **criarTabuleiro(int linhas, int colunas) {
    char **tabuleiro = malloc(linhas * sizeof(char *));
    if (!tabuleiro) return NULL;

    for (int i = 0; i < linhas; i++) {
        tabuleiro[i] = malloc(colunas * sizeof(char));
        if (!tabuleiro[i]) {
            for (int j = 0; j < i; j++) free(tabuleiro[j]);
            free(tabuleiro);
            return NULL;
        }
    }
    return tabuleiro;
}

void preencherTabuleiro(char **tabuleiro, int linhas, int colunas) {
    char letras[] = {'a', 'b', 'c', 'd', 'e'};
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            int indice = rand() % 5;
            tabuleiro[i][j] = letras[indice];
        }
    }
}

void copiarTabuleiro(char **src, char **dest, int linhas, int colunas) {
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

void salvarEstado(StateNode_t **current_node_ptr, int *history_counter) {
    StateNode_t *current_node = *current_node_ptr;
    int linhas = current_node->linhas;
    int colunas = current_node->colunas;

    StateNode_t *new_node = (StateNode_t *)malloc(sizeof(StateNode_t));
    new_node->tabuleiro = criarTabuleiro(linhas, colunas);
    new_node->linhas = linhas;
    new_node->colunas = colunas;
    new_node -> debuggable_id = current_node -> debuggable_id + 1;
    new_node->prev_node = current_node;

    // Copy current board into new state
    copiarTabuleiro(current_node->tabuleiro, new_node->tabuleiro, linhas, colunas);

    // Update the caller's pointer to point to the new node
    *current_node_ptr = new_node;

    (*history_counter)++;
}

void desfazerUltimaJogada(StateNode_t **curr_node_ptr, int *history_counter) {

    StateNode_t *current_node = *curr_node_ptr;

    printf("\nPrevious node pointer: %p\n", (void*)current_node -> prev_node);
    
    if(current_node -> prev_node){

        StateNode_t *return_target = current_node -> prev_node;
        
        liberarTabuleiro(current_node -> tabuleiro, current_node -> linhas);
        free(current_node);

        *curr_node_ptr = return_target;
        (*history_counter)--;
    }
}

int percorrerLista(StateNode_t *curr_node) {
    int resultado = 0;
    StateNode_t *temp = curr_node;
    while(temp -> prev_node){
        temp = temp -> prev_node;
        resultado++;  
    } 
    return resultado;
}

void destruirSavePoints(StateNode_t *curr_node){

    StateNode_t *iterator = curr_node -> prev_node;
    StateNode_t *temp = NULL;  

    curr_node -> prev_node = NULL;

    while(iterator){
        printf("Enter\n");
        
        if(iterator -> tabuleiro){
            temp = iterator -> prev_node;
            liberarTabuleiro(iterator -> tabuleiro, iterator -> linhas);
            free(iterator);
            iterator = temp;
        }
    }
}

void imprimirTabuleiro(StateNode_t *curr_node) {

    char **tabuleiro = curr_node -> tabuleiro;
    int linhas = curr_node -> linhas;
    int colunas = curr_node -> colunas;

    printf(ANSI_TABLE_COLOR);

    printf("\nTabuleiro %dx%d _id: %d\n", linhas, colunas, curr_node -> debuggable_id);
    printf("   ");
    for (int j = 0; j < colunas; j++)
        printf("%2d ", j);
    printf("\n");

    for (int i = 0; i < linhas; i++) {
        printf("%2d ", i);
        for (int j = 0; j < colunas; j++) {
            printf(" %c ", tabuleiro[i][j]);
        }
        printf("\n");
    }
    printf("Há %d savepoints guardados\n\n", percorrerLista(curr_node));
    printf(ANSI_RESET_SCHEME);
}

void liberarTabuleiro(char **tabuleiro, int linhas) {
    for (int i = 0; i < linhas; i++) free(tabuleiro[i]);
    free(tabuleiro);
}

int lerApartirDoSave(StateNode_t *initial_state){

    DIR* dir;
    struct dirent *entry;
    char ficheiros[100][100];
    int total = 0;

    dir = opendir("saves");
    if(!dir){
        printf("Algo ocorreu de errado ao abrir a pasta saves, começando um novo jogo.\n");
        return -1;
    }

    printf("\n\n=====================================\n");
    printf("         JOGOS DISPONÍVEIS           \n");
    printf("=====================================\n\n");

    while((entry = readdir(dir)) != NULL){
        if(strstr(entry -> d_name, ".txt")){
            printf("-----> %d - %s\n", total, entry -> d_name);
            strcpy(ficheiros[total], entry->d_name);
            total++;
        }
    }

    closedir(dir);

    if(total == 0){
        printf("Nenhum jogo encontrado, começando um novo jogo.\n");
        return -1;
    }

    int escolha = leInteiro("Escolha o numero do save a ser carregado (-1 para cancelar):");

    if(escolha == -1){
        return -1;
    }

    char caminho[150];
    snprintf(caminho, sizeof(caminho), "saves/%s", ficheiros[escolha]);

    FILE *fp = fopen(caminho, "r");
    if(!fp){
        printf("Erro ao abrir o ficheiro selecionado '%s'. Um novo jogo será iniciado.", caminho);
        return -1;
    }

    fscanf(fp, "%d %d\n", &initial_state -> linhas, &initial_state -> colunas);

    int linhas = initial_state -> linhas;
    int colunas = initial_state -> colunas;

    initial_state -> tabuleiro = criarTabuleiro(linhas, colunas);

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            initial_state -> tabuleiro[i][j] = fgetc(fp);
        }
        fgetc(fp);
    }
    fclose(fp);

    printf("\n" SUCCESS_MSG("***** Jogo lido apartir do save com sucesso *****") "\n");

    return 0;
}

void salvarTabuleiroParaFicheiro(StateNode_t* last_node) {
    FILE *fp = fopen("saves/tabuleiro_salvo.txt", "w");
    if (!fp) {
        printf("Erro ao salvar o tabuleiro.\n");
        return;
    }
    fprintf(fp, "%d %d\n", last_node -> linhas, last_node -> colunas);
    for (int i = 0; i < last_node -> linhas; i++) {
        for (int j = 0; j < last_node -> colunas; j++) {
            fprintf(fp, "%c", last_node -> tabuleiro[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    printf("Jogo salvo com sucesso em 'saves/tabuleiro_salvo.txt'.\n");
}

void modificarCasa(char **tabuleiro, int linha, int coluna, char acao) {
    
    if (acao == 'b' && islower(tabuleiro[linha][coluna])) {
        tabuleiro[linha][coluna] = toupper(tabuleiro[linha][coluna]);
    } else if (acao == 'r') {
        tabuleiro[linha][coluna] = '#';
    }
    else{
        printf("Ação inválida.\n");
    }
}

bool verificar_violacoes(StateNode_t *current_node) {

    char **tabuleiro = current_node -> tabuleiro;
    int m = current_node -> linhas;
    int n = current_node -> colunas;

    if(!casasBrancasEstaoConectadas(current_node -> tabuleiro, current_node -> linhas, current_node -> colunas)){
        return 0;
    }

    // Verificar se há peças repetidas nas linhas
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = j + 1; k < n; k++) {
                if (tabuleiro[i][j] == tabuleiro[i][k] && tabuleiro[i][j] != '#') {
                    printf("Violação: Peça repetida na linha %d.\n", i);
                    return true;
                }
            }
        }
    }

    // Verificar se há peças repetidas nas colunas
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++) {
            for (int k = i + 1; k < m; k++) {
                if (tabuleiro[i][j] == tabuleiro[k][j] && tabuleiro[i][j] != '#') {
                    printf("Violação: Peça repetida na coluna %d.\n", j);
                    return true;
                }
            }
        }
    }
    
    //Verificar se há peças isoladas
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (tabuleiro[i][j] != '#') {
                bool tem_vizinho = false;
                
                if (i > 0 && tabuleiro[i-1][j] != '#') tem_vizinho = true;
                
                if (i < m-1 && tabuleiro[i+1][j] != '#') tem_vizinho = true;
                
                if (j > 0 && tabuleiro[i][j-1] != '#') tem_vizinho = true;
                
                if (j < n-1 && tabuleiro[i][j+1] != '#') tem_vizinho = true;

                if (!tem_vizinho) {
                    printf("Violação: Peça isolada na posição (%d, %d).\n", i, j);
                    return true;
                }
            }
        }
    }

    return false;
}


int main(void){

    #ifdef RUN_TESTS
        if(call_tests() == 0) printf("\nTest was sucessful\n");
    #else

    StateNode_t *current_node = (StateNode_t*)malloc(sizeof(StateNode_t));

    if(!current_node){
        printf("Erro ao alocar node inicial, terminando programa...\n");
        return 0;
    }

    if(mkdir("saves", 0777) == -1){
        int error_code = errno;
        if(error_code == EEXIST){
            printf(SUCCESS_MSG("Diretorio de save encontrado") "\n\n");
        }
        else{
            printf(ERROR_MSG("Algo de eu errado ao criar o diretório 'saves' std_error: %d") "\n", error_code);
        }
    }

    int opcao = -2;

    do{
        printf("=====================================\n");
        printf("|            MENU INICIAL           | \n");
        printf("=====================================\n");
        printf("|      1 - Novo jogo                |\n");
        printf("|      2 - Carregar jogo anterior   |\n");
        printf("|      3 - Configurações            |\n");
        printf("|      0 - Sair                     |\n");
        printf("=====================================\n");
        opcao = leInteiro("-----> Escolha a opção: ");
    
        if(opcao == 3){
            printf("=====================================\n");
            printf("         CONTROLOS (in-game)           \n");
            printf("=====================================\n");
            printf("\n");
            printf("g - gravar jogo\n");
            printf("l - ler o estado do jogo de um ficheiro\n");
            printf("b - substituir pela maiúscula\n");
            printf("r - riscar peça\n");
            printf("v - verificar o estado do jogo e apontar todas as restrições violadas\n");
            printf("a - ajudar mudando o estado de todas as casas que se conseguem inferir através do estado atual do tabuleiro\n");
            printf("A - invocar o comando a enquanto o jogo sofrer alterações\n");
            printf("R - resolver o jogo\n");
            printf("d - desfazer o último comando executado\n");
            printf("s - sair para o menu\n");
            printf("0 - terminar o programa\n");
            leLetra("Pressione qualquer tecla para voltar...");
        }

        if(opcao < 0 || opcao > 3) printf("Opcao invalida.\n");

    } while(opcao < 0 || opcao > 2);


    int nosave = 0;

    if(opcao == 2){
        if(lerApartirDoSave(current_node) != 0){
            nosave = 1;
        }
    }
    
    if(opcao == 0){
        printf("Fechando o programa...\n");
        return 0;
    }
    
   if(opcao == 1 || nosave){
        printf("Inicializando um novo jogo...\n");
        int linhas = leInteiro("Digite o número de linhas: ");
        int colunas = leInteiro("Digite o número de colunas: ");
        current_node -> linhas = linhas;
        current_node -> colunas = colunas;
        current_node -> tabuleiro = criarTabuleiro(linhas, colunas);
        
        preencherTabuleiro(current_node -> tabuleiro, linhas, colunas);
    }

    current_node -> debuggable_id = 0;
    current_node -> prev_node = NULL;

    int linhas = current_node -> linhas;
    int colunas = current_node -> colunas;

    if(!(current_node -> tabuleiro)) {
        printf(ERROR_MSG("Erro ao alocar tabuleiro inicial, terminando programa...\n"));
        return 0;
    }

    int continuar = 1;
    int history_size = 0;

    do {
        imprimirTabuleiro(current_node);
        printf("Informe a condição, linha e a coluna (ou 0 para sair): ");
        char entrada[20];
        fgets(entrada, sizeof(entrada), stdin);

        if (entrada[0] == '0' && entrada[1] == '\n') {
            continuar = 0;
        } else if (entrada[0] == 'd' && entrada[1] == '\n') {
            desfazerUltimaJogada(&current_node, &history_size);
        } else if (entrada[0] == 'v' && entrada[1] == '\n') {
            verificar_violacoes(current_node);
        } else if (entrada[0] == 's' && entrada[1] == '\n') {
            printf("Fechando o programa ...");
            return 0;
        } else if (entrada[0] == 'g' && entrada[1] == '\n') {
            salvarTabuleiroParaFicheiro(current_node);
        } else if(entrada[0] == 'l' && entrada[1] == '\n') {
            printf("Este comando iniciara uma nova sessao de jogo, tem a certeza que deseja proseguir? O jogo atual não sera salvo automaticamente.\n");
            char decisao = leLetra("(S/N...)?");
            if(decisao == 'S'){
                StateNode_t *temp = (StateNode_t*)malloc(sizeof(StateNode_t));
                if(lerApartirDoSave(temp) != 0){
                    printf(ERROR_MSG("Algo de errado aconteceu ao tentar ler o save, o estado atual do jogo sera mantido"));
                    free(temp);
                }
                else{
                    temp -> prev_node = NULL;
                    temp -> debuggable_id = 0;
                    destruirSavePoints(current_node);
                    current_node = temp;
                    printf(SUCCESS_MSG("Save carregado"));
                }
            }
            else if(decisao == 'N') printf("Pedido cancelado. O jogo atual continuara.\n");
            else printf("Input invalido, o pedido foi cancelado e o jogo atual continuara.\n");
        }
        else {

            int lin = 0;
            int col = 0;
            char cond = 0;

            if (sscanf(entrada, "%c %d %d", &cond, &lin, &col) != 3) {
                printf("Entrada inválida!\n");
            }
            if (lin >= 0 && lin < linhas && col >= 0 && col < colunas) {
                salvarEstado(&current_node, &history_size);
                //char **mod_tab = current_node -> tabuleiro;
                modificarCasa(current_node -> tabuleiro, lin, col, cond);
            } else {
                printf("Posição inválida!\n");
            }
        }

    } while(continuar);

    #endif

    return 0;
}
