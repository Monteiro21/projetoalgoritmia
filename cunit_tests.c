#include "cunit/includes/CUnit.h"
#include "cunit/includes/Basic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jogo.h"


// Não consegui testar isto
/* void test_leinteiro(void){
    CU_ASSERT_TRUE(leInteiro("1") == 1);
}
 */
void test_copiarTabuleiro(void) {
    char **src = criarTabuleiro(2, 2);
    char **dest = criarTabuleiro(2, 2);
    
    src[0][0] = 'a';
    src[0][1] = 'b';
    src[1][0] = 'c';
    src[1][1] = 'd';
    
    copiarTabuleiro(src, dest, 2, 2);
    
    CU_ASSERT_EQUAL(dest[0][0], 'a');
    CU_ASSERT_EQUAL(dest[0][1], 'b');
    CU_ASSERT_EQUAL(dest[1][0], 'c');
    CU_ASSERT_EQUAL(dest[1][1], 'd');
    
    liberarTabuleiro(src, 2);
    liberarTabuleiro(dest, 2);
}
void test_casasBrancasEstaoConectadas_connected(void) {
    char **tab = criarTabuleiro(3, 3);
    tab[0][0] = 'A';
    tab[0][1] = 'B';
    tab[0][2] = 'a';
    tab[1][0] = 'C';
    tab[1][1] = 'a';
    tab[1][2] = 'a';
    tab[2][0] = 'a';
    tab[2][1] = 'a';
    tab[2][2] = 'a';
    
    // Testar todas casas conectadas
    CU_ASSERT_TRUE(casasBrancasEstaoConectadas(tab, 3, 3));
    liberarTabuleiro(tab, 3);
}
void test_casasBrancasEstaoConectadas_disconnected(void) {
    char **tab = criarTabuleiro(3, 3);
    tab[0][0] = 'A';
    tab[0][1] = 'a';
    tab[0][2] = 'a';
    tab[1][0] = 'a';
    tab[1][1] = 'a';
    tab[1][2] = '#';
    tab[2][0] = 'a';
    tab[2][1] = '#';
    tab[2][2] = 'B'; // B isolado
    
    // Uma casa não está conectada
    CU_ASSERT_FALSE(casasBrancasEstaoConectadas(tab, 3, 3));
    liberarTabuleiro(tab, 3);
}
void test_casasBrancasEstaoConectadas_no_white(void) {
    char **tab = criarTabuleiro(3, 3);
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            tab[i][j] = 'a';
        }
    }
    
    // Sem casas brancas retorna true
    CU_ASSERT_TRUE(casasBrancasEstaoConectadas(tab, 3, 3));
    liberarTabuleiro(tab, 3);
}

void test_salvarEstado(void) {
    StateNode_t *current_node = (StateNode_t*)malloc(sizeof(StateNode_t));
    current_node->linhas = 2;
    current_node->colunas = 2;
    current_node->debuggable_id = 0;
    current_node->prev_node = NULL;
    current_node->tabuleiro = criarTabuleiro(2, 2);
    
    current_node->tabuleiro[0][0] = 'a';
    current_node->tabuleiro[0][1] = 'b';
    current_node->tabuleiro[1][0] = 'c';
    current_node->tabuleiro[1][1] = 'd';
    
    int history_counter = 0;
    salvarEstado(&current_node, &history_counter);
    
    CU_ASSERT_EQUAL(history_counter, 1);
    CU_ASSERT_PTR_NOT_NULL(current_node->prev_node);
    CU_ASSERT_EQUAL(current_node->debuggable_id, 1);
    
    CU_ASSERT_EQUAL(current_node->tabuleiro[0][0], 'a');
    CU_ASSERT_EQUAL(current_node->tabuleiro[0][1], 'b');
    CU_ASSERT_EQUAL(current_node->tabuleiro[1][0], 'c');
    CU_ASSERT_EQUAL(current_node->tabuleiro[1][1], 'd');
    
    liberarTabuleiro(current_node->prev_node->tabuleiro, 2);
    free(current_node->prev_node);
    liberarTabuleiro(current_node->tabuleiro, 2);
    free(current_node);
}

void test_salvarTabuleiroParaFicheiro(void) {
    // Create a directory for testing (if it doesn't exist)
    system("mkdir -p saves");
    
    StateNode_t *node = (StateNode_t*)malloc(sizeof(StateNode_t));
    node->linhas = 2;
    node->colunas = 2;
    node->tabuleiro = criarTabuleiro(2, 2);
    
    node->tabuleiro[0][0] = 'a';
    node->tabuleiro[0][1] = 'b';
    node->tabuleiro[1][0] = 'c';
    node->tabuleiro[1][1] = 'd';
    
    // Redirect stdout to prevent console output
    FILE *original_stdout = stdout;
    stdout = tmpfile();
    
    salvarTabuleiroParaFicheiro(node);
    
    // Restore stdout
    stdout = original_stdout;
    
    // Verify the file was created
    FILE* test_file = fopen("saves/tabuleiro_salvo.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(test_file);
    
    if (test_file != NULL) {
        int linhas, colunas;
        fscanf(test_file, "%d %d\n", &linhas, &colunas);
        
        CU_ASSERT_EQUAL(linhas, 2);
        CU_ASSERT_EQUAL(colunas, 2);
        
        fclose(test_file);
    }
    
    // Clean up
    liberarTabuleiro(node->tabuleiro, 2);
    free(node);
}


 void test_desfazerUltimaJogada(void) {
    StateNode_t *node1 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node1->linhas = 2;
    node1->colunas = 2;
    node1->debuggable_id = 0;
    node1->prev_node = NULL;
    node1->tabuleiro = criarTabuleiro(2, 2);
    
    node1->tabuleiro[0][0] = 'a';
    node1->tabuleiro[0][1] = 'b';
    node1->tabuleiro[1][0] = 'c';
    node1->tabuleiro[1][1] = 'd';
    
    StateNode_t *current_node = node1;
    int history_counter = 0;
    
    // Save state
    salvarEstado(&current_node, &history_counter);
    current_node->tabuleiro[0][0] = 'A'; // Modify the board
    
    // Undo the move
    desfazerUltimaJogada(&current_node, &history_counter);
    
    CU_ASSERT_EQUAL(history_counter, 0);
    CU_ASSERT_EQUAL(current_node->debuggable_id, 0);
    CU_ASSERT_PTR_NULL(current_node->prev_node);
    CU_ASSERT_EQUAL(current_node->tabuleiro[0][0], 'a'); // Should have reverted
    
    // Clean up
    liberarTabuleiro(current_node->tabuleiro, 2);
    free(current_node);
}

void test_desfazerUltimaJogada_modriscar(void){

    StateNode_t *node1 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node1->linhas = 2;
    node1->colunas = 2;
    node1->debuggable_id = 0;
    node1->prev_node = NULL;
    node1->tabuleiro = criarTabuleiro(2, 2);
    
    node1->tabuleiro[0][0] = 'a';
    node1->tabuleiro[0][1] = 'b';
    node1->tabuleiro[1][0] = 'c';
    node1->tabuleiro[1][1] = 'd';
    
    StateNode_t *current_node = node1;
    int history_counter = 0;
    
    // Save state
    salvarEstado(&current_node, &history_counter);
    modificarCasa(current_node -> tabuleiro, 0, 0, 'r');
    CU_ASSERT_EQUAL(current_node -> tabuleiro[0][0], '#');
    // Undo the move
    desfazerUltimaJogada(&current_node, &history_counter);
    
    CU_ASSERT_EQUAL(history_counter, 0);
    CU_ASSERT_EQUAL(current_node->debuggable_id, 0);
    CU_ASSERT_PTR_NULL(current_node->prev_node);
    CU_ASSERT_EQUAL(current_node->tabuleiro[0][0], 'a'); // Should have reverted
    
    // Clean up
    liberarTabuleiro(current_node->tabuleiro, 2);
    free(current_node);

}



 void test_percorrerLista(void) {
    // Create a list with 3 nodes
    StateNode_t *node1 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node1->debuggable_id = 0;
    node1->prev_node = NULL;
    
    StateNode_t *node2 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node2->debuggable_id = 1;
    node2->prev_node = node1;
    
    StateNode_t *node3 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node3->debuggable_id = 2;
    node3->prev_node = node2;
    
    // Test that we count 2 previous nodes from node3
    CU_ASSERT_EQUAL(percorrerLista(node3), 2);
    
    // Clean up
    free(node1);
    free(node2);
    free(node3);
}

void test_percorrerLista_unlink(void) {
    // Create a list with 3 nodes
    StateNode_t *node1 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node1->debuggable_id = 0;
    node1->prev_node = NULL;
    
    StateNode_t *node2 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node2->debuggable_id = 1;
    node2->prev_node = node1;
    
    StateNode_t *node3 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node3->debuggable_id = 2;
    node3->prev_node = NULL;
    
    StateNode_t *node4 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node4->debuggable_id = 3;
    node4->prev_node = node3;
    
    // Test that we count 2 previous nodes from node3
    CU_ASSERT_EQUAL(percorrerLista(node4), 1);
    
    // Clean up
    free(node1);
    free(node2);
    free(node3);
    free(node4);
}


 void test_destruirSavePoints(void) {
    // Create a chain of nodes
    StateNode_t *node1 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node1->linhas = 1;
    node1->tabuleiro = criarTabuleiro(1, 1);
    node1->prev_node = NULL;
    
    StateNode_t *node2 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node2->linhas = 1;
    node2->tabuleiro = criarTabuleiro(1, 1);
    node2->prev_node = node1;
    
    StateNode_t *node3 = (StateNode_t*)malloc(sizeof(StateNode_t));
    node3->linhas = 1;
    node3->tabuleiro = criarTabuleiro(1, 1);
    node3->prev_node = node2;
    
    // Destroy all previous save points
    destruirSavePoints(node3);
    
    // Verify that prev_node is now NULL
    CU_ASSERT_PTR_NULL(node3->prev_node);

    // Clean up
    liberarTabuleiro(node3->tabuleiro, 1);
    free(node3);
}

void test_verificar_violacoes_no_violations(void) {
    StateNode_t *node = (StateNode_t*)malloc(sizeof(StateNode_t));
    node->linhas = 3;
    node->colunas = 3;
    node->tabuleiro = criarTabuleiro(3, 3);
    
    // No repeated elements in rows or columns
    node->tabuleiro[0][0] = 'a';
    node->tabuleiro[0][1] = 'b';
    node->tabuleiro[0][2] = 'c';
    node->tabuleiro[1][0] = 'b';
    node->tabuleiro[1][1] = 'c';
    node->tabuleiro[1][2] = 'a';
    node->tabuleiro[2][0] = 'c';
    node->tabuleiro[2][1] = 'a';
    node->tabuleiro[2][2] = 'b';
    
    // Redirect stdout
    FILE *original_stdout = stdout;
    stdout = tmpfile();
    
    bool result = verificar_violacoes(node);
    
    // Restore stdout
    stdout = original_stdout;
    
    CU_ASSERT_FALSE(result); // Should find no violations
    
    // Clean up
    liberarTabuleiro(node->tabuleiro, 3);
    free(node);
}

void test_verificar_violacoes_row_violation(void) {
    StateNode_t *node = (StateNode_t*)malloc(sizeof(StateNode_t));
    node->linhas = 3;
    node->colunas = 3;
    node->tabuleiro = criarTabuleiro(3, 3);
    
    // Repeat 'a' in first row
    node->tabuleiro[0][0] = 'a';
    node->tabuleiro[0][1] = 'a';
    node->tabuleiro[0][2] = 'c';
    node->tabuleiro[1][0] = 'b';
    node->tabuleiro[1][1] = 'c';
    node->tabuleiro[1][2] = 'a';
    node->tabuleiro[2][0] = 'c';
    node->tabuleiro[2][1] = 'a';
    node->tabuleiro[2][2] = 'b';
    
    // Redirect stdout
    FILE *original_stdout = stdout;
    stdout = tmpfile();
    
    bool result = verificar_violacoes(node);
    
    // Restore stdout
    stdout = original_stdout;
    
    CU_ASSERT_TRUE(result); // Should find a violation
    
    // Clean up
    liberarTabuleiro(node->tabuleiro, 3);
    free(node);
}

void test_verificar_violacoes_column_violation(void) {
    StateNode_t *node = (StateNode_t*)malloc(sizeof(StateNode_t));
    node->linhas = 3;
    node->colunas = 3;
    node->tabuleiro = criarTabuleiro(3, 3);
    
    // Repeat 'a' in first column
    node->tabuleiro[0][0] = 'a';
    node->tabuleiro[0][1] = 'b';
    node->tabuleiro[0][2] = 'c';
    node->tabuleiro[1][0] = 'a';
    node->tabuleiro[1][1] = 'c';
    node->tabuleiro[1][2] = 'd';
    node->tabuleiro[2][0] = 'c';
    node->tabuleiro[2][1] = 'd';
    node->tabuleiro[2][2] = 'b';
    
    // Redirect stdout
    FILE *original_stdout = stdout;
    stdout = tmpfile();
    
    bool result = verificar_violacoes(node);
    
    // Restore stdout
    stdout = original_stdout;
    
    CU_ASSERT_TRUE(result); // Should find a violation
    
    // Clean up
    liberarTabuleiro(node->tabuleiro, 3);
    free(node);
}

void test_verificar_violacoes_isolated_piece(void) {
    StateNode_t *node = (StateNode_t*)malloc(sizeof(StateNode_t));
    node->linhas = 3;
    node->colunas = 3;
    node->tabuleiro = criarTabuleiro(3, 3);
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            node->tabuleiro[i][j] = '#';
        }
    }
    node->tabuleiro[1][1] = 'a';
    
    // Redirect stdout
    FILE *original_stdout = stdout;
    stdout = tmpfile();
    
    bool result = verificar_violacoes(node);
    
    // Restore stdout
    stdout = original_stdout;
    
    CU_ASSERT_TRUE(result); // Should find an isolated piece violation
    
    // Clean up
    liberarTabuleiro(node->tabuleiro, 3);
    free(node);
}

void test_verificar_violacoes_general(void) {
    StateNode_t *node = (StateNode_t*)malloc(sizeof(StateNode_t));
    node->linhas = 4;
    node->colunas = 4;
    node->tabuleiro = criarTabuleiro(4, 4);

    
    // Violação de linha
    node->tabuleiro[0][0] = 'a';
    node->tabuleiro[0][1] = 'a'; 
    node->tabuleiro[0][2] = 'c';
    node->tabuleiro[0][3] = 'd';
    
    node->tabuleiro[1][0] = 'a'; // Violação de coluna com 0 0
    node->tabuleiro[1][1] = 'A';
    node->tabuleiro[1][2] = '#';
    node->tabuleiro[1][3] = '#';
    
    node->tabuleiro[2][0] = 'b';
    node->tabuleiro[2][1] = '#';
    node->tabuleiro[2][2] = '#';
    node->tabuleiro[2][3] = '#';
    
    node->tabuleiro[3][0] = 'B'; // Disconnected from A
    node->tabuleiro[3][1] = '#';
    node->tabuleiro[3][2] = '#';
    node->tabuleiro[3][3] = 'e'; // Peça isolada
    
    FILE *original_stdout = stdout;
    stdout = tmpfile();
    
    bool result = verificar_violacoes(node);
    
    stdout = original_stdout;
    
    CU_ASSERT_TRUE(result);

    liberarTabuleiro(node->tabuleiro, 4);
    free(node);
}

 void test_leinteiro(void){
    FILE* temp = tmpfile();
    fputs("1\n", temp);
    rewind(temp);

    FILE* original_stdin = stdin;
    stdin = temp;
    CU_ASSERT(leInteiro("") == 1);
    stdin = original_stdin;
    fclose(temp);
 }

void test_criar_e_liberar_tabuleiro(void) {
    char **tab = criarTabuleiro(3, 4);
    CU_ASSERT_PTR_NOT_NULL(tab);
    for (int i = 0; i < 3; i++)
        CU_ASSERT_PTR_NOT_NULL(tab[i]);
    liberarTabuleiro(tab, 3);
}

void test_preencher_tabuleiro(void) {
    char **tab = criarTabuleiro(2, 2);
    preencherTabuleiro(tab, 2, 2);
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            CU_ASSERT_TRUE(tab[i][j] >= 'a' && tab[i][j] <= 'e');
    liberarTabuleiro(tab, 2);
}

void test_modificarCasa_maiuscula(void) {
    StateNode_t state;
    state.tabuleiro = criarTabuleiro(1, 1);
    char **tab = state.tabuleiro;
    tab[0][0] = 'a';
    modificarCasa(&state, 0, 0, 'b');
    CU_ASSERT_EQUAL(tab[0][0], 'A');
    liberarTabuleiro(tab, 1);
}
void test_modificarCasa_riscado_maiscula(void) {
    StateNode_t state;
    state.tabuleiro = criarTabuleiro(1, 1);
    char **tab = state.tabuleiro;
    tab[0][0] = '#';
    modificarCasa(&state, 0, 0, 'r');
    CU_ASSERT_EQUAL(tab[0][0], '#');
    liberarTabuleiro(tab, 1);

}

void test_modificarCasa_riscar(void) {
    StateNode_t state;
    state.tabuleiro = criarTabuleiro(1, 1);
    char **tab = state.tabuleiro;
    tab[0][0] = 'b';
    modificarCasa(&state, 0, 0, 'r');
    CU_ASSERT_EQUAL(tab[0][0], '#');
    liberarTabuleiro(tab, 1);
}

void test_modificarCasa_invalid_action(void) {
    StateNode_t state;
    state.tabuleiro = criarTabuleiro(1, 1);
    char **tab = state.tabuleiro;
    tab[0][0] = 'a';
    
    // Redirect stdout to capture output
    FILE* original_stdout = stdout;
    stdout = tmpfile();
    
    modificarCasa(tab, 0, 0, 'z');  // Invalid action
    
    // Restore stdout
    stdout = original_stdout;
    
    // Should remain unchanged
    CU_ASSERT_EQUAL(tab[0][0], 'a');
    liberarTabuleiro(tab, 1);
}

int call_tests(void){

    CU_pSuite pSuite = NULL;

    if(CU_initialize_registry() != CUE_SUCCESS){
        return CU_get_error();
    }

    pSuite = CU_add_suite("Testes Jogo", 0, 0);

    if(CU_add_test(pSuite, "Teste ler inteiro", test_leinteiro) == NULL){
        return CU_get_error();
    }
    /*
    if(CU_add_test(pSuite, "Teste libertar tabuleiro", test_liberar_tabuleiro) == NULL){
        return CU_get_error();
    } */
    if(CU_add_test(pSuite, "Teste criação tabuleiro", test_criar_e_liberar_tabuleiro) == NULL){
        return CU_get_error();
    } 
    if(CU_add_test(pSuite, "Teste preencher tabuleiro", test_preencher_tabuleiro) == NULL){
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste modificar - Maiuscula", test_modificarCasa_maiuscula) == NULL){
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste modificar - Riscar", test_modificarCasa_riscar) == NULL){
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste modificar - Maiscula apos riscar", test_modificarCasa_riscado_maiscula) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste modificar - Invalido", test_modificarCasa_invalid_action) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste destruirSavePoints", test_destruirSavePoints) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste percorrerLista", test_percorrerLista) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste percorrerLista - Unlink", test_percorrerLista_unlink) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste desfazerUltimaJogada", test_desfazerUltimaJogada) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste desfazerUltimaJogada - Modificao Riscar", test_desfazerUltimaJogada_modriscar) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste casasBrancasEstaoConectadas - conectadas", test_casasBrancasEstaoConectadas_connected) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste casasBrancasEstaoConectadas - desconectadas", test_casasBrancasEstaoConectadas_disconnected) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste casasBrancasEstaoConectadas - sem células brancas", test_casasBrancasEstaoConectadas_no_white) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste salvarEstado", test_salvarEstado) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste copiarTabuleiro", test_copiarTabuleiro) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste verificar_violacoes - sem violações", test_verificar_violacoes_no_violations) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste verificar_violacoes - linha", test_verificar_violacoes_row_violation) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste verificar_violacoes - coluna", test_verificar_violacoes_column_violation) == NULL) {
        return CU_get_error();
    }
    if(CU_add_test(pSuite, "Teste verificar_violacoes - peça isolada", test_verificar_violacoes_isolated_piece) == NULL) {
        return CU_get_error();
    }  
    if(CU_add_test(pSuite, "Teste verificar_violacoes - peça isolada", test_verificar_violacoes_general == NULL)) {
        return CU_get_error();
    }  
    if(CU_add_test(pSuite, "Teste salvarTabuleiroParaFicheiro", test_salvarTabuleiroParaFicheiro) == NULL) {
        return CU_get_error();
    }
    
    
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CUE_SUCCESS;
}
