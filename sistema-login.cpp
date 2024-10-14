#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>//usei para o comando strcmp e strlen
#include <ctype.h>// usar to lower <- colocar letras maiusculas em minuscula
#include <mysql.h>//precisa disso pra conectar com o banco de dados;
#include <conio.h>
//função para conectar ao banco de dados
void conectar_ao_banco(MYSQL* conn) {//mysql* conn, o parametro con é usado para conectar com o banco de dados;
    // Conectar ao banco de dados;; local host== rodando na minha maquina;; lucas== meu nome de usario no banco;; password== é a senha para eu logar;; sistec== nome do banco de dados
    if (mysql_real_connect(conn, "localhost", "lucas", "password", "sistec_", 0, NULL, 0) == NULL) {//my sql_real_connect é a função que tenta se conectar ao banco usando os parametros
        fprintf(stderr, "Erro na conexão: %s\n", mysql_error(conn));//mysql error da imprime uma msg de erro pro usuario 
        mysql_close(conn);//e o mysql_close fecha a conexão
        exit(1);
    }
}
//deixar as letras em padrão
void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}
//verificar se a senha e usuario digitados sao os mesmos cadastrados
int verificar_usuario(MYSQL* conn, const char* nome_usuario, const char* senha) {
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2], result[1];
    char query[] = "SELECT role FROM usuarios WHERE nome_usuario = ? AND senha = ?";
    char role[20] = "";  // Buffer para armazenar o valor do papel (role)

    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        fprintf(stderr, "Erro ao inicializar a consulta: %s\n", mysql_error(conn));
        return -1;
    }

    if (mysql_stmt_prepare(stmt, query, (unsigned long)strlen(query)) != 0) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    memset(bind, 0, sizeof(bind));

    // Nome do usuário
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)nome_usuario;
    bind[0].buffer_length = (unsigned long)strlen(nome_usuario);

    // Senha
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)senha;
    bind[1].buffer_length = (unsigned long)strlen(senha);

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "Erro ao associar parametros: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "Erro ao executar a consulta: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    memset(result, 0, sizeof(result));

    // Vincular o resultado da consulta
    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = role;
    result[0].buffer_length = sizeof(role);

    if (mysql_stmt_bind_result(stmt, result) != 0) {
        fprintf(stderr, "Erro ao associar resultados: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_store_result(stmt) != 0) {
        fprintf(stderr, "Erro ao armazenar resultado: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Verificar se há linhas retornadas
    if (mysql_stmt_fetch(stmt) == MYSQL_NO_DATA) {
        printf("Usuario ou senha incorretos.\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // Verificar o papel do usuário
    if (strcmp(role, "admin") == 0) {
        printf("Login bem-sucedido! Bem-vindo, Administrador.\n");
        mysql_stmt_close(stmt);
        return 1; // Administrador
    }
    else if (strcmp(role, "caixa") == 0) {
        printf("Login bem-sucedido! Bem-vindo, Caixa.\n");
        mysql_stmt_close(stmt);
        return 2; // Caixa
    }
    else if (strcmp(role, "estoquista") == 0) {
        printf("Login bem-sucedido! Bem-vindo, Estoquista.\n");
        mysql_stmt_close(stmt);
        return 3; // Estoquista
    }
    else {
        printf("Role desconhecido. Acesso negado.\n");
        mysql_stmt_close(stmt);
        return -1; // Role desconhecido
    }
}
//função pra criar usarios novos
int cadastrar_usuario(MYSQL* conn, const char* nome_usuario, const char* senha, const char* role) {
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[3];
    char query[] = "INSERT INTO usuarios (nome_usuario, senha, role) VALUES (?, ?, ?)";

    stmt = mysql_stmt_init(conn);  // Inicializar o statement
    if (stmt == NULL) {
        fprintf(stderr, "Erro ao inicializar a consulta: %s\n", mysql_error(conn));
        return -1;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Inicializar o array de bind
    memset(bind, 0, sizeof(bind));

    // Nome do usuário
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)nome_usuario;
    bind[0].buffer_length = strlen(nome_usuario);

    // Senha
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)senha;
    bind[1].buffer_length = strlen(senha);

    // Role (admin, caixa, estoquista)
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char*)role;
    bind[2].buffer_length = strlen(role);

    // Associar os parâmetros
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "Erro ao associar parametros: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Executar a consulta
    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "Erro ao executar a consulta: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    printf("Usuário cadastrado com sucesso!\n");
    mysql_stmt_close(stmt);
    return 1;  // Sucesso
}
//função para ver todos os usuarios existentes
int buscar_usuarios(MYSQL* conn) {
    if (mysql_query(conn, "SELECT * FROM usuarios")) {
        fprintf(stderr, "Erro ao buscar o usuario: %s\n", mysql_error(conn));
        return -1;
    }
    MYSQL_RES* resultado = mysql_store_result(conn);
    if (resultado == NULL) {
        fprintf(stderr, "Erro ao armazenar resultado: %s\n", mysql_error(conn));
        return -1;
    }

    MYSQL_ROW linha;
    MYSQL_FIELD* campos = mysql_fetch_fields(resultado);
    unsigned int num_campos = mysql_num_fields(resultado);

    // Imprimir cabeçalho
    for (unsigned int i = 0; i < num_campos; i++) {
        printf("%-20s", campos[i].name);
    }
    printf("\n");

    // Imprimir linhas do resultado
    while ((linha = mysql_fetch_row(resultado))) {
        for (unsigned int i = 0; i < num_campos; i++) {
            printf("%-20s", linha[i] ? linha[i] : "NULL");
        }
        printf("\n");
    }

    mysql_free_result(resultado);
}
//função para excluir usuarios existentes
int excluir_usuario(MYSQL* conn, int usuario_id) {
    char query[256];
    sprintf_s(query, "DELETE FROM usuarios WHERE id = %d", usuario_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao excluir o usuario: %s\n", mysql_error(conn));
        return -1;
    }

    if (mysql_affected_rows(conn) == 0) {
        printf("Nenhum usuario encontrado com o ID %d\n", usuario_id);
        return -1;
    }

    printf("Usuario com ID %d excluido com sucesso.\n", usuario_id);
    return 0;
}
//inserir produto no estoque
void inserir_produto(MYSQL* conn) {
    char nome_produto[100];
    int quantidade;
    char fornecedor[100];

    // Coletar os dados do produto
    printf("Digite o nome do produto: ");
    fgets(nome_produto, sizeof(nome_produto), stdin);
    nome_produto[strcspn(nome_produto, "\n")] = 0;  // Remover o '\n' que o fgets inclui

    printf("Digite a quantidade: ");
    scanf_s("%d", &quantidade);
    getchar(); // Limpar o buffer do '\n'

    printf("Digite o nome do fornecedor: ");
    fgets(fornecedor, sizeof(fornecedor), stdin);
    fornecedor[strcspn(fornecedor, "\n")] = 0;  // Remover o '\n' que o fgets inclui

    // Construir a query para inserir no banco de dados
    char query[256];
    sprintf_s(query, "INSERT INTO estoque (nome_produto, quantidade, fornecedor, data_cadastrada) VALUES ('%s', %d, '%s', NOW())",
        nome_produto, quantidade, fornecedor);

    // Executar a query
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao inserir produto: %s\n", mysql_error(conn));
        return;
    }

    printf("Produto inserido com sucesso.\n");
}
//editar produtos do estoque
void editar_estoque(MYSQL* conn) {
    int id;
    char nome_produto[100];
    int quantidade;
    char fornecedor[100];

    // Solicitar o ID do produto a ser editado
    printf("Digite o ID do produto a ser editado: ");
    scanf_s("%d", &id);
    getchar(); // Limpar o buffer do '\n'

    // Verificar se o ID do produto existe
    char query_check[256];
    sprintf_s(query_check, "SELECT id FROM estoque WHERE id=%d", id);
    if (mysql_query(conn, query_check)) {
        fprintf(stderr, "Erro ao verificar produto: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES* resultado = mysql_store_result(conn);
    if (resultado == NULL || mysql_num_rows(resultado) == 0) {
        printf("Produto com ID %d não encontrado.\n", id);
        mysql_free_result(resultado);
        return;
    }
    mysql_free_result(resultado);

    // Coletar novos dados
    printf("Digite o novo nome do produto: ");
    fgets(nome_produto, sizeof(nome_produto), stdin);
    nome_produto[strcspn(nome_produto, "\n")] = 0;  // Remover o '\n'

    printf("Digite a nova quantidade: ");
    scanf_s("%d", &quantidade);
    getchar(); // Limpar o buffer do '\n'

    printf("Digite o novo nome do fornecedor: ");
    fgets(fornecedor, sizeof(fornecedor), stdin);
    fornecedor[strcspn(fornecedor, "\n")] = 0;  // Remover o '\n'

    // Construir e executar a query de atualização
    char query_update[256];
    sprintf_s(query_update, "UPDATE estoque SET nome_produto='%s', quantidade=%d, fornecedor='%s' WHERE id=%d",
        nome_produto, quantidade, fornecedor, id);

    if (mysql_query(conn, query_update)) {
        fprintf(stderr, "Erro ao editar o produto: %s\n", mysql_error(conn));
        return;
    }

    printf("Produto atualizado com sucesso.\n");
}

//mostrar os produtos do estoque
void mostrar_estoque(MYSQL* conn) {
    if (mysql_query(conn, "SELECT * FROM estoque")) {
        fprintf(stderr, "Erro ao buscar o estoque: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES* resultado = mysql_store_result(conn);
    if (resultado == NULL) {
        fprintf(stderr, "Erro ao armazenar resultado: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_ROW linha;
    MYSQL_FIELD* campos = mysql_fetch_fields(resultado);
    unsigned int num_campos = mysql_num_fields(resultado);

    // Imprimir cabeçalho
    for (unsigned int i = 0; i < num_campos; i++) {
        printf("%-20s", campos[i].name);
    }
    printf("\n");

    // Imprimir linhas do resultado
    while ((linha = mysql_fetch_row(resultado))) {
        for (unsigned int i = 0; i < num_campos; i++) {
            printf("%-20s", linha[i] ? linha[i] : "NULL");
        }
        printf("\n");
    }

    mysql_free_result(resultado);
}
//excluir produto no estoque
void excluir_produto(MYSQL* conn) {
    int id;

    // Solicitar o ID do produto a ser excluído
    printf("Digite o ID do produto que deseja excluir: ");
    scanf_s("%d", &id);

    // Construir e executar a query de exclusão
    char query[256];
    sprintf_s(query, "DELETE FROM estoque WHERE id = %d", id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao excluir o produto: %s\n", mysql_error(conn));
        return;
    }

    if (mysql_affected_rows(conn) == 0) {
        printf("Nenhum produto encontrado com o ID %d\n", id);
    }
    else {
        printf("Produto com ID %d excluído com sucesso.\n", id);
    }
}

//função de mostrar o menu ao usuario
void mostrar_menu(const char* role) {
    int opcao;
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Erro ao inicializar MySQL: %s\n", mysql_error(conn));
        exit(1);
    }
    conectar_ao_banco(conn);

    do {
        system("cls");
        printf("\n--- MENU ---\n");
        printf("1. Acesso exclusivo ao Administrador\n");
        printf("2. Acesso ao Administrador e Estoquista\n");
        printf("3. Acesso ao Administrador e Caixa\n");
        printf("0. Sair\n");
        printf("Escolha uma opcao: ");
        scanf_s("%d", &opcao);

        switch (opcao) {
        case 1:
            if (strcmp(role, "admin") == 0) {
                int tarefa;
                fflush(stdin);
                system("cls");
                printf("Bem-vindo ao Menu Exclusivo de Administradores!\n");
                    printf("1. criar usuário\n");
                    printf("2. Exibir usuários\n");
                    printf("3. Excluir usuário\n");
                    printf("4. editar usuarios\n");
                    printf("5. Sair\n");
                    if (scanf_s("%d", &tarefa) > 5) {
                        printf("Erro ao ler a opcao.\n");
                    }
                    switch (tarefa) {
                    case 1:
                        char nome_usuario[50];
                        char senha[50];
                        char role[20];
                        // Coletar informações do novo usuário
                        printf("Digite o nome do usuário a ser cadastrado: ");
                        scanf_s("%49s", nome_usuario);  // Limitando a entrada a 49 caracteres para evitar overflow

                        printf("Digite a senha para o novo usuário: ");
                        scanf_s("%49s", senha);

                        // Pedir o tipo de usuário
                        printf("Digite o tipo de usuário (admin, caixa, estoquista): ");
                        scanf_s("%19s", role);

                        // Chamar a função para cadastrar o usuário no banco de dados
                        if (cadastrar_usuario(conn, nome_usuario, senha, role) == 1) {
                            printf("Usuário cadastrado com sucesso!\n");
                        }
                        else {
                            printf("Erro ao cadastrar o usuário.\n");
                        }

                        // Fechar a conexão com o banco de dados
                        mysql_close(conn);
                        break;
                    case 2:
                        fflush(stdin);
                        buscar_usuarios(conn);
                        printf("aperte qualquer tecla pra voltar");
                        _getch();
                        break;
                    case 3:
                        fflush(stdin);
                        int usuario_id;
                        printf_s("\nDigite o ID do usuário que deseja excluir: ");
                        scanf_s("%d", &usuario_id);

                        // Chamar a função para excluir o usuário
                        excluir_usuario(conn, usuario_id);
                        printf("aperte qualquer tecla para voltar");
                        _getch();
                        break;
                    default: printf("opção inválida");
                    }
            }
            else {
                printf("Acesso negado. Somente Administradores podem acessar esta opção.\n");
            }
            break;

        case 2:
            if (strcmp(role, "admin") == 0 || strcmp(role, "estoquista") == 0) {
                int tarefa;
                do {
                    printf("\nMenu de Opções:\n");
                    printf("1. Inserir produto\n");
                    printf("2. Editar produto\n");
                    printf("3. Mostrar estoque\n");
                    printf("4. Excluir produto\n");
                    printf("5. Sair\n");
                    printf("Escolha uma opção: ");
                    scanf_s("%d", &tarefa);
                    getchar(); // Limpar o buffer

                    switch (tarefa) {
                    case 1:
                        inserir_produto(conn);
                        break;
                    case 2:
                        editar_estoque(conn);
                        break;
                    case 3:
                        mostrar_estoque(conn);
                        break;
                    case 4:
                        excluir_produto(conn);
                        break;
                    case 5:
                        printf("Saindo...\n");
                        break;
                    default:
                        printf("Opção inválida!\n");
                    }
                } while (opcao != 5);
            }
            else {
                printf("Acesso negado. Somente Administradores ou Estoquistas podem acessar esta opção.\n");
            }
            break;

        case 3:
            if (strcmp(role, "admin") == 0 || strcmp(role, "caixa") == 0) {
                printf("Bem-vindo ao Menu de Caixa!\n");
            }
            else {
                printf("Acesso negado. Somente Administradores ou Caixas podem acessar esta opção.\n");
            }
            break;

        case 0:
            printf("Saindo...\n");
            break;

        default:
            printf("Opcao invalida!\n");
        }
    } while (opcao != 0);
}

int main() {
    setlocale(LC_ALL, "Portuguese");
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Erro ao inicializar MySQL: %s\n", mysql_error(conn));
        exit(1);
    } 
    conectar_ao_banco(conn);

    char nome_usuario[50];
    char senha[50];
    char role[20];
    int login_status;

    // Pedir informações ao usuário para login
    printf("Digite o nome do usuário: ");
    scanf_s("%49s", nome_usuario);  // Limitando a entrada a 49 caracteres para evitar overflow

    printf("Digite a senha: ");
    scanf_s("%49s", senha);

    // Verificar o usuário e obter o role (1 = admin, 2 = usuario comum)
    login_status = verificar_usuario(conn, nome_usuario, senha);

    if (login_status == 1) {
        strcpy_s(role, "admin");
    }
    else if (login_status == 2) {
        printf("Digite o tipo de usuário (caixa, estoquista): ");
        scanf_s("%19s", role);

        // Converter o valor de role para minúsculas
        to_lowercase(role);
    }
    else {
        printf("Nome de usuário ou senha incorretos.\n");
        mysql_close(conn);
        return -1;
    }

    // Mostrar menu
    mostrar_menu(role);

    // Fechar a conexão com o banco de dados
    mysql_close(conn);

    return 0;
}
