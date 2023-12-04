#pragma once

void clearTerminal();
void fill(char c, int n);
void title(const char title[], const wchar_t wtitle[]);
int menu(const char options[][81], const wchar_t woptions[][81], int quant);
int titleMenu(const char msg[], const wchar_t wmsg[], const char options[][81], const wchar_t woptions[][81], int quant);