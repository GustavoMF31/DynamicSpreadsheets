#pragma once

// Enumerating the formatting values available
enum format {
  RED,     // Some colors to stylize the console
  GREEN,
  YELLOW,
  BLUE,
  PURPLE,
  CYAN,
  BOLD,    // Used to highlight some pieces of text
  CLEAR    // Turn back to the normal state
};

// Global Variables
extern const char formatCodes[8][81];
extern char palette[8][81];

void clearTerminal();
void setColor();
void clearColor();
void wait(float sec);
void loading(char msg[], float duration);
void fill(char c, int n);
void title(const char title[], const wchar_t wtitle[]);
int menu(const char options[][81], const wchar_t woptions[][81], int quant);
int titleMenu(const char msg[], const wchar_t wmsg[], const char options[][81], const wchar_t woptions[][81], int quant);