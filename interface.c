#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <time.h>

#include "interface.h"

// Base array of format codes
const char formatCodes[8][81] = {
  "\033[31m", // RED
  "\033[32m", // GREEN
  "\033[33m", // YELLOW
  "\033[34m", // BLUE
  "\033[35m", // PURPLE
  "\033[36m", // CYAN
  "\033[1m",  // BOLD
  "\033[m"    // CLEAR
};

// Array that will determine the collection of colors used in the program
// It starts as 8 null strings, indicating that the program will be at standard theme
// If the user opts for a colored console, every string in this array will receive the format codes above
// Deactivating the colored console option will clear the color scheme and turn this back to null
char palette[8][81] = {};

// Clears the console terminal
void clearTerminal() {
  #ifdef _WIN32
      system("cls");
  #else
      system("clear");
  #endif
}

// Sets the color scheme into the active state
void setColor(){
  for (int i = 0; i < 8; i++){
    strcpy(palette[i], formatCodes[i]);
  }
  clearTerminal();
}

// Clears the color scheme of the program
void clearColor(){
  for (int i = 0; i < 8; i++){
    strcpy(palette[i], "");
  }
  printf("%s", formatCodes[CLEAR]);
  clearTerminal();
}

// Delays the stream, accepting fractions of seconds
void wait(float sec){
  struct timespec delay;
  delay.tv_sec = (time_t)sec;
  delay.tv_nsec = (long int)(sec - delay.tv_sec) * 1000000000;
  nanosleep(&delay, NULL);
}

// Simulates a loading section after a given message
void loading(char msg[], float duration){
  printf("%s", msg);
  for (int i = 0; i < 3; i++){
    fflush(stdout);
    wait(duration/3);
    printf(".");
  }
}

// Fills the terminal with n occurrences of the specified character
void fill(char c, int n){
  for (int i = 0; i < n; i++) printf("%c", c);
}

// Prints a stylized title from a given string (and its wide form)
void title(const char title[], const wchar_t wtitle[]){
  unsigned int length = wcslen(wtitle);

  printf("%s%s+", palette[BOLD], palette[CYAN]);
  fill('-', length+6);
  printf("+\n");

  printf("|   %s   |", title);

  printf("\n+");
  fill('-', length+6);
  printf("+%s%s\n", palette[CLEAR], palette[CYAN]);
}

// Displays a simple menu that offers options to the user to choose from
// OBS.: Introduce the strings relative to the options in both forms: normal and wide
int menu(const char options[][81], const wchar_t woptions[][81], int quant){
  unsigned int maxlen = 0;
  char selection[81];

  for (int i = 0; i < quant; i++){
    if (wcslen(woptions[i]) > maxlen) maxlen = wcslen(woptions[i]);
  }

  printf("\n%s+", palette[CYAN]);
  fill('-', maxlen + 5);
  printf("+");
  for (int i = 0; i < quant; i++){
    printf("\n|%s[%d]%s %s", palette[YELLOW], i+1, palette[CYAN], options[i]);
    fill(' ', maxlen + 1 - wcslen(woptions[i]));
    printf("|");
  }

  printf("\n+");
  fill('-', maxlen + 5);
  printf("+\n");

  // Receives defended input from the user
  while (1){
    printf("\n%s>> %s", palette[CYAN], palette[GREEN]);
    scanf("%81[^\n]", selection);
    getchar();
    
    if ((atoi(selection) > 0 || strcmp(selection, "0") == 0) && atoi(selection) <= quant){
      clearTerminal();
      return atoi(selection);
    }
    printf("\n%sEscolha inválida!\n", palette[RED]);
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

  printf("\n%s+", palette[CYAN]);
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
    printf("\n|%s[%d]%s %s", palette[YELLOW], i+1, palette[CYAN], options[i]);
    fill(' ', maxlen + 1 + aux - wcslen(woptions[i]));
    printf("|");
  }

  printf("\n+");
  fill('-', maxlen + 5 + aux);
  printf("+\n");

  // Receives defended input from the user
  while (1){
    printf("\n%s>> %s", palette[CYAN], palette[GREEN]);
    scanf("%81[^\n]", selection);
    getchar();

    if ((atoi(selection) > 0 || strcmp(selection, "0") == 0) && atoi(selection) <= quant){
      clearTerminal();
      return atoi(selection);
    }
      printf("\n%sEscolha inválida!\n", palette[RED]);
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
  titleMenu("Dynamic Spreadsheets", L"Dynamic Spreadsheets", menu_inicial, menu_inicial_a, 3);
  return 0;
}
*/