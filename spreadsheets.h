#pragma once

#include "interface.h"

// Data types supported by the spreadsheets
// (TODO: Should we have a separate "CHAR" type?)
typedef enum {
  BOOL,
  STRING, // A stored string may only have up to 80 characters
  INT,
  DOUBLE
} Type;

typedef struct row {
  char *entries; // Points to the raw data (bytes) of the entries in this row
  struct row *next; // Points to the next Row in the spreadsheet this row is in.
                    // It might be NULL, in which case this is the last row.
} Row;

typedef struct {
  int columns; // Amount of columns
  int rows;    // Amount of rows
  Type *column_types;  // One type per column
                       // (TODO: be consistent and use camelCase)
  char **column_names; // Holds the name of each column.
                       // Each name is expected to be in an 81 byte long buffer
  Row *firstRow; // The first row. Might be NULL if there are no rows (rows == 0).
} Spreadsheet;

__attribute__ ((noreturn)) void badType(Type t, char* functionName);
int columnIndex(Spreadsheet s, char* name);
int columnOffset(Spreadsheet s, int col);
void freeRow(Row *row);
void deleteRow(Spreadsheet *s, Row **prev, Row *curr);
void deleteRowByIndex(Spreadsheet *s, int pos);
void initializeSpreadsheet(Spreadsheet *s);
void printCellByIndex(Spreadsheet s, int row, int col, FILE *file);
void writeToFile(Spreadsheet s, char *file_name);
bool readFromFile(Spreadsheet *s, char *file_name);
Spreadsheet example();
void exportAsCsv(Spreadsheet s, char *file_name);
void freeSpreadsheet(Spreadsheet s);
void updateCellValue(Spreadsheet s, int row, int col);
void displaySpreadsheet(Spreadsheet s);
void removeColumn(Spreadsheet *s, int col);
void addColumn(Spreadsheet *s, char* colName, Type type);
void addRow(Spreadsheet *s, int row1);
void ascendingSortByValue(Spreadsheet *s, int col);
void descendingSortByValue(Spreadsheet *s, int col);
void sortByAlphabet(Spreadsheet *s, int col);
