#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include "interface.h"
#include "spreadsheets.h"
#include "filter.h"

// ASSIGNMENTS

void newSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName, bool *createdNow);
void openExistentSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName, bool *backToMenu);
void addField(Spreadsheet *mainSpreadsheet, char *myColumnName, Type *myColumnType, char *name);
void removeField(Spreadsheet *mainSpreadsheet, char *myColumnName, int *columnPosition);
void addLine(Spreadsheet *mainSpreadsheet, char *buffer, int *rowPosition);
void deleteLine(Spreadsheet *mainSpreadsheet, char *buffer, int *rowPosition);
void editCell(Spreadsheet *mainSpreadsheet, char *buffer, int *columnPosition, int *rowPosition);
void sortSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *myColumnName, int *columnPosition);
void completeDisplay(Spreadsheet *mainSpreadsheet, char *name);
void filteredDisplay(Spreadsheet *mainSpreadsheet, char *name, char *fileName);
void showSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName);
void exportSpreadsheet(Spreadsheet *mainSpreadsheet, char *name);
void saveSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName);
void information(Spreadsheet *mainSpreadsheet);
void configurations(bool *colored);

// GLOBAL MENUS

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

int main(){
  const char mainMenu[][81] = {
    "Nova Planilha",
    "Carregar Planilha    ",
    "Informações",
    "Configurações",
    "Sair"
  };
  const wchar_t wmainMenu[][81] = {
    L"Nova Planilha",
    L"Carregar Planilha    ",
    L"Informações",
    L"Configurações",
    L"Sair"
  };

  bool colored = false;

  while (true){
    clearTerminal();

    title("DYNAMIC SPREADSHEETS", L"DYNAMIC SPREADSHEETS");

    Spreadsheet mainSpreadsheet;
    char name[76], fileName[81];
    bool createdNow = false, backToMenu = false;

    int mainSelection = menu(mainMenu, wmainMenu, 5);

    switch (mainSelection){
      case 1: // "Nova Planilha"
        newSpreadsheet(&mainSpreadsheet, name, fileName, &createdNow);

      case 2: // "Carregar Planilha"
        if (!createdNow){ // If the user did not create a spreadsheet now
          openExistentSpreadsheet(&mainSpreadsheet, name, fileName, &backToMenu);
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
          printf("\n%sGerencie %s'%s'%s à sua maneira!\n", palette[CYAN], palette[YELLOW], name, palette[CYAN]);

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

                char myColumnName[81], buffer[81];
                int columnPosition, rowPosition;
                Type myColumnType;

                printf("\n%sAdicione, edite e remova elementos de sua planilha!\n", palette[CYAN]);

                int editionSelection = menu(editionMenu, weditionMenu, 7);

                switch (editionSelection){
                  case 1: // "Adicionar Campo"
                    addField(&mainSpreadsheet, myColumnName, &myColumnType, name);
                    break;
                  case 2: // "Remover Campo"
                    removeField(&mainSpreadsheet, myColumnName, &columnPosition);
                    break;
                  case 3: // "Adicionar Linha"
                    addLine(&mainSpreadsheet, buffer, &rowPosition);
                    break;
                  case 4: // "Remover Linha"
                    deleteLine(&mainSpreadsheet, buffer, &rowPosition);
                    break;
                  case 5: // "Editar Célula"
                    editCell(&mainSpreadsheet, buffer, &columnPosition, &rowPosition);
                    break;
                  case 6: // "Ordenar Planilha"
                    sortSpreadsheet(&mainSpreadsheet, name, myColumnName, &columnPosition);
                    break;
                  case 7: // "Voltar"
                    break;
                }

                // Save the spreadsheet automatically after every modification
                writeToFile(mainSpreadsheet, fileName);

                if (editionSelection == 7) break;
              }

              break;
            case 2: // "Exibir Planilha"
              showSpreadsheet(&mainSpreadsheet, name, fileName);
              break;
            case 3: // "Exportar Planilha como CSV"
              exportSpreadsheet(&mainSpreadsheet, name);
              break;
            case 4: // "Salvar Planilha"
              saveSpreadsheet(&mainSpreadsheet, name, fileName);
              break;
            case 5: // "Voltar"
              break;
          }

          if (loadSelection == 5) break;
        }
        break;
      case 3: // "Informações"
        information(&mainSpreadsheet);
        break;
      case 4: // "Configurações"
        configurations(&colored);
        break;
      case 5: // "Sair"
        // Release the memory alocated
        freeSpreadsheet(mainSpreadsheet);
        break;
    }

    if (mainSelection == 5) break;
  }

  clearColor();

  return 0;
}

// SCREENS

// "Nova Planilha"
void newSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName, bool *createdNow){
  clearTerminal();

  printf("\n%sCrie uma nova planilha aqui, dando suas informações iniciais.\n", palette[CYAN]);
  while (true){
    printf("\n%sQual será o nome da nova planilha <sem acentos> ?\n\n>> %s", palette[CYAN], palette[GREEN]);
    strcpy(name, "");
    scanf("%81[^\n]", name);
    getchar();
    if (strcmp(name, "") != 0) break;
    printf("\n%sInsira um nome válido!\n", palette[RED]);
  }
  snprintf(fileName, 81, "%s.data", name);

  // Initializing the spreadsheet
  initializeSpreadsheet(mainSpreadsheet);

  writeToFile(*mainSpreadsheet, fileName);

  //loading("\nCriando Planilha", 2000);

  // Send the user directly to the editing menu...
  *createdNow = true; // and uses a flag to identify a newly created spreadsheet
}

// "Carregar Planilha"
void openExistentSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName, bool *backToMenu){
  int tries = 1;

  clearTerminal();
  printf("\n%sCarregue sua planilha para acessar seus dados.\n", palette[CYAN]);

  while (true){ // Defending the file name
    printf("\n%sQual o nome da planilha que deseja acessar?\n\n>> %s", palette[CYAN], palette[GREEN]);
    strcpy(name, "");
    scanf("%81[^\n]", name);
    getchar();

    snprintf(fileName, 81, "%s.data", name);
    printf("\n");
    if (readFromFile(mainSpreadsheet, fileName)) break;
    printf("\n%sDigite o nome de uma planilha existente.\n", palette[RED]);

    if (tries > 2){
      if (titleMenu("Voltar ao início?", L"Voltar ao início?", confirmation, wconfirmation, 2) == 1){
        *backToMenu = true;
        break;
      }
    }

    if (*backToMenu) break;
    tries++;
  }
}

// "Adicionar Campo"
void addField(Spreadsheet *mainSpreadsheet, char *myColumnName, Type *myColumnType, char *name){
  printf("\n%sAdicione um novo campo, correspondente a uma coluna, em sua planilha.\n", palette[CYAN]);
                    
  // Receiving the column name
  while (true){
    bool alreadyExists = false;
    printf("\n%sQual será o nome do novo campo <sem acentos> ?\n\n>> %s", palette[CYAN], palette[GREEN]);
    strcpy(myColumnName, "");
    scanf("%81[^\n]", myColumnName);
    getchar();
    for (int i = 0; i < mainSpreadsheet->columns; i++){
      if (strcmp(myColumnName, mainSpreadsheet->column_names[i]) == 0){
        alreadyExists = true;
        break;
      }
    }
    if (alreadyExists){
      printf("\n%sJá existe campo com esse nome.\n", palette[RED]);
      continue;
    }
    if (strcmp(myColumnName, "") != 0) break;
    printf("\n%sInsira um nome válido.\n", palette[RED]);
  }

  const char typeMenu[][81] = {
    "Inteiro",
    "Racional",
    "Booleano",
    "String"
  };
  const wchar_t wtypeMenu[][81] = {
    L"Inteiro",
    L"Racional",
    L"Booleano",
    L"String"
  };

  int typeSelection = titleMenu("Qual será o tipo do campo?", L"Qual será o tipo do campo?", typeMenu, wtypeMenu, 4);

  // Selecting its type
  switch (typeSelection){
    case 1: // "Inteiro"
      *myColumnType = INT;
      printf("\n%sO campo %s'%s'%s, de tipo %s'inteiro'%s, foi anexado à planilha %s'%s'%s com sucesso!\n", palette[GREEN], palette[YELLOW], myColumnName, 
            palette[GREEN], palette[YELLOW], palette[GREEN], palette[YELLOW], name, palette[GREEN]);
      break;
    case 2: // "Racional"
      *myColumnType = DOUBLE;
      printf("\n%sO campo %s'%s'%s, de tipo %s'racional'%s, foi anexado à planilha %s'%s'%s com sucesso!\n", palette[GREEN], palette[YELLOW], myColumnName, 
            palette[GREEN], palette[YELLOW], palette[GREEN], palette[YELLOW], name, palette[GREEN]);
      break;
    case 3: // "Booleano"
      *myColumnType = BOOL;
      printf("\n%sO campo %s'%s'%s, de tipo %s'booleano'%s, foi anexado à planilha %s'%s'%s com sucesso!\n", palette[GREEN], palette[YELLOW], myColumnName, 
            palette[GREEN], palette[YELLOW], palette[GREEN], palette[YELLOW], name, palette[GREEN]);
      break;
    case 4: // "String"
      *myColumnType = STRING;
      printf("\n%sO campo %s'%s'%s, de tipo %s'string'%s, foi anexado à planilha %s'%s'%s com sucesso!\n", palette[GREEN], palette[YELLOW], myColumnName, 
            palette[GREEN], palette[YELLOW], palette[GREEN], palette[YELLOW], name, palette[GREEN]);
      break;
  }

  // If it is not empty in rows, instruct the user about the insertion of data in the existing rows on the new column
  if (mainSpreadsheet->rows > 0){
    printf("\n%sInsira os valores a serem adicionados à nova coluna.\n", palette[CYAN]);
    printf("\nIdentifique booleanos por 'verdadeiro' ou 'falso', e use . para delimitar a parte fracionária de racionais.\n");
  }

  addColumn(mainSpreadsheet, myColumnName, *myColumnType);

  menu(backMenu, wbackMenu, 1);
}

// "Remover Campo"
void removeField(Spreadsheet *mainSpreadsheet, char *myColumnName, int *columnPosition){
  printf("\n%sRetire um campo da planilha.\n", palette[CYAN]);

  // Treats the case where the spreadsheet does not have any columns to remove
  if (mainSpreadsheet->columns == 0){
    printf("\n%sA planilha atual não possui campos para remoção.\n", palette[RED]);
    menu(backMenu, wbackMenu, 1);
    return;
  }

  while (true){
    bool exists = false;

    printf("\n%sQual o nome do campo a ser excluído?\n\n>> %s", palette[CYAN], palette[GREEN]);
    strcpy(myColumnName, "");
    scanf("%81[^\n]", myColumnName);
    getchar();

    for (int i = 0; i < mainSpreadsheet->columns; i++){
      if (strcmp(myColumnName, mainSpreadsheet->column_names[i]) == 0){
        exists = true;
        *columnPosition = i;
        break;
      }
    }

    if (!exists){
      printf("\n%sNão há campo com esse nome na planilha.\n", palette[RED]);
      continue;
    }

    break;
  }

  removeColumn(mainSpreadsheet, *columnPosition);

  printf("\n%sO campo %s'%s'%s foi removido com sucesso!\n", palette[GREEN], palette[YELLOW], myColumnName, palette[GREEN]);

  menu(backMenu, wbackMenu, 1);
}

// "Adicionar Linha"
void addLine(Spreadsheet *mainSpreadsheet, char *buffer, int *rowPosition){
  printf("\n%sAdicione uma nova linha em qualquer posição disponível na planilha.\n", palette[CYAN]);

  if (mainSpreadsheet->rows > 0){
    while (true){
      printf("\n%sQual o índice da linha que você quer incluir %s<de 0 a %d>%s ?\n\n>> %s", palette[CYAN], palette[BLUE], mainSpreadsheet->rows, palette[CYAN], palette[GREEN]);
      strcpy(buffer, "");
      scanf("%81[^\n]", buffer);
      getchar();
      
      if (atoi(buffer) <= 0 && strcmp(buffer, "0") != 0){
        printf("\n%sDigite um número válido para a posição.\n", palette[RED]);
        continue;
      }
      
      *rowPosition = atoi(buffer);

      if (*rowPosition >= 0 && *rowPosition <= mainSpreadsheet->rows) break;

      printf("\n%sInsira uma posição válida.\n", palette[RED]);
    }
  }
  else *rowPosition = 0;

  printf("\n%sA linha foi criada com sucesso!\n", palette[GREEN]);
  
  // If it is not empty in columns, instruct the user about the insertion of data in the existing columns on the new row
  if (mainSpreadsheet->columns > 0){
    printf("\n%sInsira os valores a serem adicionados à nova linha.\n", palette[CYAN]);
    printf("\nIdentifique booleanos por 'verdadeiro' ou 'falso', e use . para delimitar a parte fracionária de racionais.\n");
  }

  addRow(mainSpreadsheet, *rowPosition);

  printf("\n%sLinha adicionada com sucesso na posição %s%d%s.\n", palette[CYAN], palette[YELLOW], *rowPosition, palette[CYAN]);

  menu(backMenu, wbackMenu, 1);
}

// "Remover Linha"
void deleteLine(Spreadsheet *mainSpreadsheet, char *buffer, int *rowPosition){

  printf("\n%sExclua uma linha com praticidade!\n", palette[CYAN]);

  // Treats the case where the spreadsheet does not have any rows to remove
  if (mainSpreadsheet->rows == 0){
    printf("\n%sA planilha atual não possui linhas para remoção.\n", palette[RED]);
    menu(backMenu, wbackMenu, 1);
    return;
  }

  if (mainSpreadsheet->rows > 0){
    while (true){
      printf("\n%sQual o índice da linha a ser eliminada %s<de 0 a %d>%s ?\n\n>> %s", palette[CYAN], palette[BLUE], mainSpreadsheet->rows-1, palette[CYAN], palette[GREEN]);
      strcpy(buffer, "");
      scanf("%81[^\n]", buffer);
      getchar();
      
      if (atoi(buffer) <= 0 && strcmp(buffer, "0") != 0){
        printf("\n%sDigite um número válido para a posição.\n", palette[RED]);
        continue;
      }
      
      *rowPosition = atoi(buffer);

      if (*rowPosition >= 0 && *rowPosition <= mainSpreadsheet->rows-1) break;

      printf("\n%sInsira uma posição válida.\n", palette[RED]);
    }
  }
  else *rowPosition = 0;

  deleteRowByIndex(mainSpreadsheet, *rowPosition);

  printf("\n%sA linha %s%d%s foi apagada com sucesso!\n", palette[GREEN], palette[YELLOW], *rowPosition, palette[GREEN]);

  menu(backMenu, wbackMenu, 1);
}

// "Editar Célula"
void editCell(Spreadsheet *mainSpreadsheet, char *buffer, int *columnPosition, int *rowPosition){
  printf("\n%sModifique o valor de uma célula em específico.\n", palette[CYAN]);

  // Empty spreadsheets do not have cells to edit
  if (mainSpreadsheet->columns == 0 || mainSpreadsheet->rows == 0){
    printf("\n%sNão existem células na planilha para serem editadas.\n", palette[RED]);
    menu(backMenu, wbackMenu, 1);
    return;
  }

  *rowPosition = *columnPosition = -1;
  while (true){
    printf("\n%sQual a posição da célula %s<linha> <coluna>%s ?\n\n>> %s", palette[CYAN], palette[BLUE], palette[CYAN], palette[GREEN]);
    strcpy(buffer, "");
    scanf("%81[^\n]", buffer);
    getchar();
    
    sscanf(buffer, "%d %d", rowPosition, columnPosition);

    if (*rowPosition >= 0 && *rowPosition <= mainSpreadsheet->rows-1 && *columnPosition >= 0 && *columnPosition <= mainSpreadsheet->columns-1) break;

    printf("\n%sDigite coordenadas existentes na planilha.\n", palette[RED]);
  }

  printf("\n%sA célula foi encontrada!", palette[GREEN]);
  printf("\nSeu valor atual é %s", palette[YELLOW]);
  printCellByIndex(*mainSpreadsheet, *rowPosition, *columnPosition, stdout);
  printf("%s.\n", palette[GREEN]);

  updateCellValue(*mainSpreadsheet, *rowPosition, *columnPosition);

  printf("\n%sA célula na linha %s<%d>%s e campo %s<%s>%s foi atualizada para %s", palette[CYAN], palette[YELLOW], *rowPosition, palette[CYAN],
        palette[YELLOW], mainSpreadsheet->column_names[*columnPosition], palette[CYAN], palette[YELLOW]);
  printCellByIndex(*mainSpreadsheet, *rowPosition, *columnPosition, stdout);
  printf("%s.\n", palette[CYAN]);

  menu(backMenu, wbackMenu, 1);
}

// "Ordenar Planilha"
void sortSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *myColumnName, int *columnPosition){
  printf("\n%sOrdene a planilha de acordo com os dados de uma coluna.\n", palette[CYAN]);

  // Defends empty spreadsheets
  if (mainSpreadsheet->columns == 0){
    printf("\n%sA planilha atual não possui nenhum campo para ser ordenado.\n", palette[RED]);
    menu(backMenu, wbackMenu, 1);
    return;
  }

  while (true){
    bool exists = false, isBool = false;

    printf("\n%sQual o nome do campo a ser considerado?\n\n>> %s", palette[CYAN], palette[GREEN]);
    strcpy(myColumnName, "");
    scanf("%81[^\n]", myColumnName);
    getchar();

    for (int i = 0; i < mainSpreadsheet->columns; i++){
      if (strcmp(myColumnName, mainSpreadsheet->column_names[i]) == 0){
        exists = true;
        if (mainSpreadsheet->column_types[i] == BOOL){
          isBool = true;
          break;
        }
        *columnPosition = i;
        break;
      }
    }

    if (!exists){
      printf("\n%sNão há campo com esse nome na planilha.\n", palette[RED]);
      continue;
    }
    else if (isBool){
      printf("\n%sImpossível ordenar por valor booleano.\n", palette[RED]);
      continue;
    }

    break;
  }

  // The implementation of sorting is a little bit different for strings, so we need to divide the cases
  if (mainSpreadsheet->column_types[*columnPosition] != STRING){
    const char sortMenu[][81] = {
      "Ordem Crescente",
      "Ordem Decrescente"
    };
    const wchar_t wsortMenu[][81] = {
      L"Ordem Crescente",
      L"Ordem Decrescente"
    };

    if (titleMenu("Ordenar de qual forma?", L"Ordenar de qual forma?", sortMenu, wsortMenu, 2) == 1){
      ascendingSortByValue(mainSpreadsheet, *columnPosition);
      printf("\n%sA planilha foi ordenada, a partir do campo %s'%s'%s, em ordem crescente de valor.\n", palette[CYAN], palette[YELLOW], myColumnName, palette[CYAN]);
    }
    else {
      descendingSortByValue(mainSpreadsheet, *columnPosition);
      printf("\n%sA planilha foi ordenada, a partir do campo %s'%s'%s, em ordem decrescente de valor.\n", palette[CYAN], palette[YELLOW], myColumnName, palette[CYAN]);
    }
  }
  else {
    clearTerminal();
    sortByAlphabet(mainSpreadsheet, *columnPosition);
    printf("\n%sA planilha foi ordenada, a partir do campo %s'%s'%s, em ordem alfabética.\n", palette[CYAN], palette[YELLOW], myColumnName, palette[CYAN]);
  }

  printf("\n%sPlanilha: %s%s\n\n", palette[PURPLE], palette[YELLOW], name);

  displaySpreadsheet(*mainSpreadsheet);

  menu(backMenu, wbackMenu, 1);
}

// "Planilha Completa"
void completeDisplay(Spreadsheet *mainSpreadsheet, char *name){
  printf("\n%sPlanilha: %s%s\n\n", palette[CYAN], palette[YELLOW], name);

  displaySpreadsheet(*mainSpreadsheet);

  menu(backMenu, wbackMenu, 1);
}

// "Planilha Filtrada"
void filteredDisplay(Spreadsheet *mainSpreadsheet, char *name, char* fileName){
  printf("\n%sDiga quais filtros deseja aplicar à exibição de sua planilha.", palette[CYAN]);
  printf("\nIndique cada filtro em uma linha, seguindo o formato %s<literal/campo> <operador> <literal/campo>%s.", palette[BLUE], palette[CYAN]);
  printf("\nLembre-se de incluir, pelo menos, o nome de um campo no filtro.\n");
  printf("\nIdentifique strings por aspas duplas (\"\"), booleanos por 'verdadeiro' ou 'falso', e use . para delimitar a parte fracionária de racionais.");
  printf("\nOBS.: Insira racionais na forma mais reduzida possível.\n");
  printf("\nPule a linha duas vezes para parar.\n");

  printf("\nOperadores disponíveis:\n");
  printf("\n> (Maior que);");
  printf("\n>= (Maior ou igual a);");
  printf("\n< (Menor que);");
  printf("\n<= (Menor ou igual a);");
  printf("\n= (Igual a);\n\n");

  char buffer[81];
  Spreadsheet auxSpreadsheet;
  Filter condition;

  initializeSpreadsheet(&auxSpreadsheet);
  writeToFile(*mainSpreadsheet, fileName);
  readFromFile(&auxSpreadsheet, fileName);

  while (true){
    printf("%s>> %s", palette[CYAN], palette[GREEN]);
    strcpy(buffer, "");
    scanf("%81[^\n]", buffer);
    getchar();
    if (strcmp(buffer, "") == 0) break;

    if(!inputToFilter(auxSpreadsheet, buffer, &condition)) continue;

    // Filtering the auxiliar spreadsheet
    filterSpreadsheet(&auxSpreadsheet, condition);
  }

  printf("\n%sPlanilha: %s%s\n\n", palette[CYAN], palette[YELLOW], name);

  displaySpreadsheet(auxSpreadsheet);

  freeSpreadsheet(auxSpreadsheet);

  menu(backMenu, wbackMenu, 1);
}

// "Exibir Planilha"
void showSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName){
  while (true){
    printf("\n%sExiba sua planilha integralmente ou de maneira filtrada!\n", palette[CYAN]);

    // Tells the user to insert data to the spreadsheet before showing it
    if (mainSpreadsheet->columns == 0 || mainSpreadsheet->rows == 0){
      printf("\n%sParece que sua planilha não tem dados o suficiente.", palette[RED]);
      printf("\nVolte novamente depois de alimentá-la.\n");
      menu(backMenu, wbackMenu, 1);
      break;
    }

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
        completeDisplay(mainSpreadsheet, name);
        break;
      case 2: // "Planilha Filtrada"
        filteredDisplay(mainSpreadsheet, name, fileName);
        break;
      case 3: // "Voltar"
        break;
    }

    if (displaySelection == 3) break;
  }
}

// "Exportar Planilha como CSV"
void exportSpreadsheet(Spreadsheet *mainSpreadsheet, char *name){
  printf("\n%sUm arquivo .csv será criado com o nome de sua planilha.\n", palette[CYAN]);

  char csvName[81];
  snprintf(csvName, 81, "%s.csv", name);

  exportAsCsv(*mainSpreadsheet, csvName);

  printf("\n%sO arquivo %s'%s'%s foi criado com sucesso!\n", palette[GREEN], palette[YELLOW], csvName, palette[GREEN]);

  menu(backMenu, wbackMenu, 1);
}

// "Salvar Planilha"
void saveSpreadsheet(Spreadsheet *mainSpreadsheet, char *name, char *fileName){
  printf("\n%sSua planilha será salva em um arquivo binário para uso futuro.\n", palette[CYAN]);

  snprintf(fileName, 81, "%s.data", name);

  writeToFile(*mainSpreadsheet, fileName);

  printf("\n%sO arquivo %s'%s'%s foi salvo com sucesso!\n", palette[GREEN], palette[YELLOW], fileName, palette[GREEN]);

  menu(backMenu, wbackMenu, 1);
}

// "Informações"
void information(Spreadsheet *mainSpreadsheet){
  printf("\n%sEste programa consiste em uma ferramenta de auxílio para o gerenciamento dinâmico de planilhas.", palette[CYAN]);
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
    *mainSpreadsheet = example();
    writeToFile(*mainSpreadsheet, "exemplo.data");
    printf("\n%sUma planilha de exemplo foi gerada com nome %s'exemplo'%s e guardada no arquivo %s'exemplo.data'%s.", palette[CYAN], palette[YELLOW], palette[CYAN],
          palette[YELLOW], palette[CYAN]);
    printf("\n%sVocê pode acessá-la em %s'Carregar Planilha'%s.\n", palette[CYAN], palette[YELLOW], palette[CYAN]);
    menu(backMenu, wbackMenu, 1);
  }
}

// "Configurações"
void configurations(bool *colored){
  printf("\n%sPersonalize sua experiência com o programa aqui!\n", palette[CYAN]);

  int configMenu;

  if (!*colored){
    configMenu = titleMenu("Deseja ativar a interface colorida?", L"Deseja ativar a interface colorida?", confirmation, wconfirmation, 2);
    if (configMenu == 1){ 
      setColor();
      *colored = true;
      printf("\n%sEsquema de cores ativado!\n", palette[GREEN]);
    }
    else return;
  }
  else {
    configMenu = titleMenu("Deseja desativar a interface colorida?", L"Deseja desativar a interface colorida?", confirmation, wconfirmation, 2);
    if (configMenu == 1){ 
      clearColor();
      *colored = false;
      printf("\n%sEsquema de cores desativado!\n", palette[GREEN]);
    }
    else return;
  }

  menu(backMenu, wbackMenu, 1);
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