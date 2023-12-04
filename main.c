#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>

#include "interface.h"
#include "spreadsheets.h"
#include "filter.h"

int main(){
  const char mainMenu[][81] = {
    "Nova Planilha",
    "Carregar Planilha",
    "Informações",
    "Sair"
  };
  const wchar_t wmainMenu[][81] = {
    L"Nova Planilha",
    L"Carregar Planilha",
    L"Informações",
    L"Sair"
  };

  // Appears in multiple scenarios where the user only confirms a choice
  const char confirmation[][81] = {
    "Sim",
    "Não"
  };
  const wchar_t wconfirmation[][81] = {
    L"Sim",
    L"Não"
  };

  // Important in all the cases where the program only waits input from the user
  const char backMenu[][81] = {
    "Voltar"
  };
  const wchar_t wbackMenu[][81] = {
    L"Voltar"
  };

  while (true){
    clearTerminal();
    title("DYNAMIC SPREADSHEETS", L"DYNAMIC SPREADSHEETS");

    int mainSelection = menu(mainMenu, wmainMenu, 4);
    Spreadsheet mainSpreadsheet;
    Filter condition;
    char name[76], fileName[81], csvName[81];
    bool createdNow = false, backToMenu = false;

    switch (mainSelection){
      case 1: // "Nova Planilha"
        clearTerminal();
        printf("\nCrie uma nova planilha aqui, dando suas informações iniciais.\n");
        while (true){
          printf("\nQual será o nome da nova planilha <não inclua acentos> ?\n\n>> ");
          strcpy(name, "");
          scanf("%[^\n]", name);
          getchar();
          if (strcmp(name, "") != 0) break;
          printf("\nInsira um nome válido!\n");
        }
        snprintf(fileName, 81, "%s.data", name);

        // Initializing the spreadsheet
        initializeSpreadsheet(&mainSpreadsheet);

        writeToFile(mainSpreadsheet, fileName);

        // Send the user directly to the editing menu...
        createdNow = true; // and uses a flag to identify a newly created spreadsheet

      case 2: // "Carregar Planilha"
        if (!createdNow){ // If the user did not create a spreadsheet now
          int tries = 1;

          clearTerminal();
          printf("\nCarregue sua planilha para acessar seus dados.\n");

          while (true){ // Defending the file name
            printf("\nQual o nome da planilha que deseja acessar?\n\n>> ");
            strcpy(name, "");
            scanf("%[^\n]", name);
            getchar();

            snprintf(fileName, 81, "%s.data", name);
            printf("\n");
            if (readFromFile(&mainSpreadsheet, fileName)) break;
            printf("\nDigite o nome de uma planilha existente.\n");

            if (tries > 2){
              if (titleMenu("Voltar ao início?", L"Voltar ao início?", confirmation, wconfirmation, 2) == 1){
                backToMenu = true;
                break;
              }
            }

            if (backToMenu) break;
            tries++;
          }
          if (backToMenu) break;
        }

        // If an existent spreadsheet is opened, the values of 'fileName' and 'mainSpreadsheet' are updated.
        // If not, they stay the same as in the previous screen
        
        const char loadMenu[][81] = {
          "Editar Planilha",
          "Exibir Planilha",
          "Exportar Planilha como CSV",
          "Salvar Planilha",
          "Voltar"
        };
        const wchar_t wloadMenu[][81] = {
          L"Editar Planilha",
          L"Exibir Planilha",
          L"Exportar Planilha como CSV",
          L"Salvar Planilha",
          L"Voltar"
        };

        while (true){
          clearTerminal();
          printf("\nGerencie sua planilha de sua maneira!\n");

          int loadSelection = menu(loadMenu, wloadMenu, 5);

          switch (loadSelection){
            case 1: // "Editar Planilha"
              while (true){
                const char editionMenu[][81] = {
                  "Adicionar Campo",
                  "Remover Campo",
                  "Adicionar Linha",
                  "Remover Linha",
                  "Editar Célula",
                  "Ordenar Planilha",
                  "Voltar"
                };
                const wchar_t weditionMenu[][81] = {
                  L"Adicionar Campo",
                  L"Remover Campo",
                  L"Adicionar Linha",
                  L"Remover Linha",
                  L"Editar Célula",
                  L"Ordenar Planilha",
                  L"Voltar"
                };

                printf("\nAdicione, edite e remova elementos de sua planilha!\n");

                int editionSelection = menu(editionMenu, weditionMenu, 7);

                switch (editionSelection){ // TODO
                  case 1: // "Adicionar Campo"
                  case 2: // "Remover Campo"
                  case 3: // "Adicionar Linha"
                  case 4: // "Remover Linha"
                  case 5: // "Editar Célula"
                  case 6: // "Ordenar Planilha"
                  case 7: // "Voltar"
                    break;
                }

                if (editionSelection == 7) break;
              }
              break;
            case 2: // "Exibir Planilha"
              while (true){
                printf("\nExiba sua planilha integralmente ou de maneira personalizada!\n");

                const char displayMenu[][81] = {
                  "Planilha Completa",
                  "Planilha Filtrada",
                  "Voltar"
                };
                const wchar_t wdisplayMenu[][81] = {
                  L"Planilha Completa",
                  L"Planilha Filtrada",
                  L"Voltar"
                };

                int displaySelection = menu(displayMenu, wdisplayMenu, 3);

                switch (displaySelection){
                  case 1: // "Planilha Completa"
                    printf("\nPlanilha: %s\n\n", name);

                    displaySpreadsheet(mainSpreadsheet);

                    menu(backMenu, wbackMenu, 1);
                    break;
                  case 2: // "Planilha Filtrada"
                    printf("\nDiga quais filtros deseja aplicar à exibição de sua planilha.");
                    printf("\nIndique cada filtro em uma linha, seguindo o formato <campo> <operador> <literal/campo>.");
                    printf("\nIdentifique strings por aspas duplas (\"), booleanos por 'verdadeiro' ou 'falso', e use . para delimitar a parte fracionária de racionais.");
                    printf("\nOBS.: Insira racionais na forma mais reduzida possível.");
                    printf("\nPule a linha duas vezes para parar.\n");

                    printf("\nOperadores disponíveis:\n");
                    printf("\n> (Maior que);");
                    printf("\n>= (Maior ou igual a);");
                    printf("\n< (Menor que);");
                    printf("\n<= (Menor ou igual a);");
                    printf("\n= (Igual a);\n\n");

                    char buffer[81];
                    Spreadsheet auxSpreadsheet;

                    initializeSpreadsheet(&auxSpreadsheet);
                    writeToFile(mainSpreadsheet, fileName);
                    readFromFile(&auxSpreadsheet, fileName);

                    while (true){
                      printf(">> ");
                      strcpy(buffer, "");
                      scanf("%[^\n]", buffer);
                      getchar();
                      if (strcmp(buffer, "") == 0) break;

                      if(!inputToFilter(auxSpreadsheet, buffer, &condition)) continue;

                      // Filtering the auxiliar spreadsheet
                      filterSpreadsheet(&auxSpreadsheet, condition);
                    }

                    printf("\nPlanilha: %s\n\n", name);

                    displaySpreadsheet(auxSpreadsheet);

                    freeSpreadsheet(auxSpreadsheet);

                    menu(backMenu, wbackMenu, 1);
                    break;
                  case 3: // "Voltar"
                    break;
                }

                if (displaySelection == 3) break;
              }
              break;
            case 3: // "Exportar Planilha como CSV"
              printf("\nUm arquivo .csv será criado com o nome de sua planilha.\n");
              
              snprintf(csvName, 81, "%s.csv", name);

              exportAsCsv(mainSpreadsheet, csvName);

              printf("\nO arquivo %s foi criado com sucesso!\n", csvName);

              menu(backMenu, wbackMenu, 1);
              break;
            case 4: // "Salvar Planilha"
              printf("\nSua planilha será salva em um arquivo binário para uso futuro.\n");

              snprintf(fileName, 81, "%s.data", name);

              writeToFile(mainSpreadsheet, fileName);

              printf("\nO arquivo %s foi salvo com sucesso!\n", fileName);

              menu(backMenu, wbackMenu, 1);
              break;
            case 5: // "Voltar"
              break;
          }

          if (loadSelection == 5) break;
        }
        break;
      case 3: // "Informações"
        printf("\nEste programa consiste em uma ferramenta de auxílio para o gerenciamento dinâmico de planilhas.");
        printf("\nValendo-se de uma interface amigável e didática, procuramos fornecer uma ótima solução para qualquer");
        printf("\nsituação em que se precise de um grau de organização.\n");

        printf("\nFuncionalidades disponíveis:\n");

        printf("\n1) Criação de planilhas");
        printf("\n2) Carregamento de planilhas através de leitura de arquivos binários");
        printf("\n3) Salvamento de planilhas em arquivos binários");
        printf("\n4) Adição, edição e remoção de linhas, colunas e células");
        printf("\n5) Ordenação de dados em planilhas de acordo com valores de determinada coluna");
        printf("\n6) Exportação de planilhas para o formato CSV");
        printf("\n7) Exibição de planilhas de forma personalizada\n");

        printf("\nA navegação pelo programa é feita por meio de menus, cuja seleção pode ser realizada digitando o número correspondente");
        printf("\nà opção escolhida. Algumas entradas são por escrita, e, por impossibilitações técnicas, não são aceitas palavras com");
        printf("\nacentos. No mais, esperamos que tenha uma excelente experiência com nosso projeto.\n");

        printf("\nDesenvolvedores:\n");

        printf("\n- Gustavo de Mendonça Freire");
        printf("\n- João Victor Lopez Pereira");
        printf("\n- William Victor Quintela Paixão");
        printf("\n- Yuri Rocha de Albuquerque\n");

        int exampleMenu = titleMenu("Gostaria de gerar uma planilha de exemplo?", L"Gostaria de gerar uma planilha de exemplo?", confirmation, wconfirmation, 2);

        if (exampleMenu == 1){
          mainSpreadsheet = example();
          writeToFile(mainSpreadsheet, "exemplo.data");
          printf("\nUma planilha de exemplo foi gerada com nome 'exemplo' e guardada no arquivo 'exemplo.data'.");
          printf("\nVocê pode acessá-la em 'Carregar Planilha'.\n");
          menu(backMenu, wbackMenu, 1);
        }

        break;
      case 4: // "Sair"
        break;
    }

    if (mainSelection == 4) break;
  }

  return 0;
}

/*
  // Initialize a test spreadsheet
  Spreadsheet s = example();

  // Display it to make sure everything is ok
  displaySpreadsheet(s);

  // Let's write it to disk and read it back
  writeToFile(s, "test_spreadsheet.data");
  Spreadsheet s2;
  readFromFile(&s2, "test_spreadsheet.data");

  // Then check the result is the same
  printf("\n");

  // Test to see if it's printing correctly
  displaySpreadsheet(s2);

  getchar();
  clearTerminal();

  // Export it as a csv file
  exportAsCsv(s2, "test.csv");

  // Represent the condition "int > 15"
  Filter condition;
  condition.leftTerm.isLiteral = false;
  strncpy(condition.leftTerm.expr.columnName, "int", 81);
  condition.op = GT;
  condition.rightTerm.isLiteral = true;
  condition.rightTerm.expr.constant.literalType = INT;
  condition.rightTerm.expr.constant.literal.intValue = 15;

  // Then filter with it, and display the result
  filterSpreadsheet(&s2, condition);
  printf("\n");
  displaySpreadsheet(s2);

  freeSpreadsheet(s);
  freeSpreadsheet(s2);
*/