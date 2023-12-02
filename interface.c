#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// Clears the console terminal
void clearTerminal(){
/*
  #ifdef _WIN32 // Includes both 32 bit and 64 bit
    #ifdef _WIN64
      printf("Windows 64 bit\n");
    #else 
      printf("Windows 32 bit\n");
    #endif
  #else
    printf("Not a Windows OS\n");
  #endif


  #ifdef _WIN32
    system("cls");

  #else
    system("clear");

  else
    system("clr"); 
    */
}

// Fills the terminal with n occurrences of the specified caracter
void fill(char c, int n){
  for (int i = 0; i < n; i++) printf("%c", c);
}

// Prints a stylized title from a given string (and its wide form)
void title(const char title[], const wchar_t wtitle[]){
  unsigned int length = wcslen(wtitle);

  printf("\n+");
  fill('-', length+6);
  printf("+\n");

  printf("|   %s   |", title);

  printf("\n+");
  fill('-', length+6);
  printf("+\n");
}

// Displays a simple menu that offers options to the user to choose from
// OBS.: Introduce the strings relative to the options in both forms: normal and wide
int menu(const char options[][81], const wchar_t woptions[][81], int quant){
  unsigned int maxlen = 0;
  char selection[81];

  for (int i = 0; i < quant; i++){
    if (wcslen(woptions[i]) > maxlen) maxlen = wcslen(woptions[i]);
  }

  printf("\n+");
  fill('-', maxlen + 5);
  printf("+");
  for (int i = 0; i < quant; i++){
    printf("\n|[%d] %s", i+1, options[i]);
    fill(' ', maxlen + 1 - wcslen(woptions[i]));
    printf("|");
  }

  printf("\n+");
  fill('-', maxlen + 5);
  printf("+\n");

  // Receives defended input from the user
  while (1){
    printf("\n>> ");
    scanf("%[^\n]", selection);

    getchar();
    if ((atoi(selection) > 0 || strcmp(selection, "0") == 0) && atoi(selection) <= quant){
      //clearTerminal();
      return atoi(selection);
    }
    printf("\nEscolha inválida!\n");
  }
}

// Displays a message and offers options to the user to choose from
// OBS.: Introduce the strings relative to the title and options in both forms: normal and wide
int titleMenu(const char msg[], const wchar_t wmsg[], const char options[][81], const wchar_t woptions[][81], int quant){
  unsigned int maxlen = 0, titlelen = wcslen(wmsg);
  char selection[81];
  int space, aux = titlelen % 2;

  maxlen = titlelen;

  for (int i = 0; i < quant; i++){
    if (wcslen(woptions[i]) > maxlen) maxlen = wcslen(woptions[i]);
  }
  
  if(maxlen > titlelen){
    space = (maxlen+5+aux-titlelen)/2;
  }
  else{
    space = 3;
    aux = 1;
  }

  printf("+");
  fill('-', maxlen + 5 + aux);
  printf("+\n");

  printf("|");
  fill(' ', space);
  printf("%s", msg);
  fill(' ', space);
  printf("|");

  printf("\n+");
  fill('-', maxlen + 5 + aux);
  printf("+");
  for (int i = 0; i < quant; i++){
    printf("\n|[%d] %s", i+1, options[i]);
    fill(' ', maxlen + 1 + aux - wcslen(woptions[i]));
    printf("|");
  }

  printf("\n+");
  fill('-', maxlen + 5 + aux);
  printf("+\n");

  // Receives defended input from the user
  while (1){
    printf("\n>> ");
    scanf("%[^\n]", selection);

    if ((atoi(selection) > 0 || strcmp(selection, "0") == 0) && atoi(selection) <= quant){
      getchar(); 
      //clearTerminal();
      return atoi(selection);
    }
      printf("\nEscolha inválida!\n");
  }
}

/*
int main() {
  char menu_inicial[][81] = {
    "Começar",
    "Configurações",
    "Sair"
  };

  wchar_t menu_inicial_a[][81] = {
    L"Começar",
    L"Configurações",
    L"Sair"
  };
  menu(menu_inicial, menu_inicial_a, 3);
  return 0;
}*/