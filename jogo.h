#include <stdbool.h>

#ifndef JOGO_H
#define JOGO_H

typedef struct StateNode_t{
    int linhas;
    int colunas;
    char **tabuleiro;
    char **mask;
    int debuggable_id;
    struct StateNode_t *prev_node;
} StateNode_t;

typedef struct {
    int linha;
    int coluna;
} Posicao;

char **aplicarMascara(StateNode_t *node);
bool casasBrancasEstaoConectadas(char **tabuleiro, int linhas, int colunas);
char leLetra(const char *mensagem);
int leInteiro(const char *mensagem);
char **criarTabuleiro(int linhas, int colunas);
void preencherTabuleiro(char **tabuleiro, int linhas, int colunas);
void copiarTabuleiro(char **src, char **dest, int linhas, int colunas);
void salvarEstado(StateNode_t **current_node_ptr, int *history_counter);
void desfazerUltimaJogada(StateNode_t **current_node_ptr, int *history_counter);
int percorrerLista(StateNode_t *curr_node);
void destruirSavePoints(StateNode_t *curr_node);
void imprimirTabuleiro(StateNode_t *curr_node);
void liberarTabuleiro(char **tabuleiro, int linhas);
int lerApartirDoSave(StateNode_t *initial_state);
void salvarTabuleiroParaFicheiro(StateNode_t* last_node);
int modificarCasa(StateNode_t *current_node, int linha, int coluna, char acao);
bool verificar_violacoes(StateNode_t *current_node);

bool verificar_casaunica(StateNode_t *current_node, int linha, int coluna);
void resolver(StateNode_t *initial_state);
void analisar_jogadas(StateNode_t *current_node, int* map);

#endif
