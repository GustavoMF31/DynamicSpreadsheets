#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>

#include "spreadsheets.h"

// Crashes the program due to encontering and unexpected type.
// The "noreturn" attribute informs the compiler that this function never returns
// and helps to get rid of wrong warnings about forgetting a return statement
__attribute__ ((noreturn)) void badType(Type t, char* functionName){
  printf("ERROR: Got unrecognized type %d when executing %s\n", t, functionName);
  exit(1);
}

// Returns the size, in bytes, of the space necessary to store
// one element of a given type
int sizeOfType(Type t){
  switch (t) {
    case BOOL:
      // TODO: Maybe we can be more efficient and use perhaps
      // a single bit to store a boolean value
      return sizeof(bool);
    case STRING:
      // 80 characters plus null terminator
      return 81 * sizeof(char);
    case INT:
      return sizeof(int);
    case DOUBLE:
      return sizeof(double);
    default:
      badType(t, "sizeOfType");
  }
}

// Determines the index of a column given its name
int columnIndex(Spreadsheet s, char* name){
  for (int i = 0; i < s.columns; i++)
    if (strcmp(s.column_names[i], name) == 0) return i;

  return -1;
}

// Returns the index of the data for a given column within a row
// Assumes 0 <= col < s.columns
int columnOffset(Spreadsheet s, int col){
  int offset = 0;
  for (int i = 0; i < col; i++)
    offset += sizeOfType(s.column_types[i]);

  return offset;
}

// Returns a pointer to the start of the n'th row in the spreadsheet.
// Assumes 0 <= rowIndex < s.rows. (returns NULL otherwise)
// Note that this is an O(n) operation, which traverses the list of
// rows up to the specified row.
Row *getRow(Spreadsheet s, int rowIndex){
  Row *row = s.firstRow;
  while (rowIndex-- && row != NULL) row = row->next;
  return row;
}

char* rowEntries(Spreadsheet s, int rowIndex){
  return getRow(s, rowIndex)->entries;
}

// Returns the size (in bytes) occupied by a given row of the spreadsheet
int rowSize(Spreadsheet s){
  return columnOffset(s, s.columns);
}

// Returns the size (in bytes) of the data for all rows of the spreadsheet
// combined
int dataSize(Spreadsheet s){
  return s.rows * rowSize(s);
}

// Given row and column indices, returns a pointer to the cell they specify
char *getCell(Spreadsheet s, int row, int col){
  return rowEntries(s, row) + columnOffset(s, col);
}

// Prints a cell of the spreadsheet to a file according to its type
void printCell(Type t, char *cell, FILE *file){
  switch (t) {
    case BOOL:
      fprintf(file, * (bool*) cell ? "true" : "false");
      break;
    case INT:
      fprintf(file, "%d", * (int*) cell);
      break;
    case STRING:
      fprintf(file, "\"%s\"", cell);
      break;
    case DOUBLE:
      fprintf(file, "%g", * (double*) cell);
      break;
    default:
      printf("Unrecognized type: %d\n", t);
      exit(1);
  }
}

// Prints to a file the cell of a spreadsheet at the chosen row and column
void printCellByIndex(Spreadsheet s, int row, int col, FILE *file){
  printCell(s.column_types[col], getCell(s, row, col), file);
}

// Prints a spreadsheet to the terminal (mostly for debugging purposes)
void displaySpreadsheet(Spreadsheet s){
  for (int col = 0; col < s.columns; col++){
    printf("%s", s.column_names[col]);
    printf(" ");
  }
  printf("\n");

  for (int row = 0; row < s.rows; row++){
    for (int col = 0; col < s.columns; col++){
      printCellByIndex(s, row, col, stdout);
      printf(" ");
    }
    printf("\n");
  }
}

// Ensures a file has been opened successfully (Exits the program otherwise)
void checkOpen(FILE *file, char *file_name){
  if (file != NULL) return;
  printf("Não foi possível abrir arquivo %s", file_name);
  exit(1);
}

// Writes a spreadsheet to a file (in a binary format)
void writeToFile(Spreadsheet s, char *file_name){
  FILE *file = fopen(file_name, "wb");
  checkOpen(file, file_name);

  // Write the header
  fwrite(&s.columns, sizeof(int) , 1, file);
  fwrite(&s.rows, sizeof(int), 1, file);
  fwrite(s.column_types, sizeof(Type), s.columns, file);
  for (int i = 0; i < s.columns; i++)
    fwrite(s.column_names[i], sizeof(char), 81, file);

  // Write the actual data, traversing the list of rows
  Row *row = s.firstRow;
  while (row != NULL){
    fwrite(row->entries, sizeof(char), rowSize(s), file);
    row = row->next;
  }

  fclose(file);
}

// Reads a spreadsheet from a binary file onto the given pointer.
// Memory is allocated to hold buffers
// (column_types, column_names and each row node, as well as its entries)
// which the caller should remember to free with freeSpreadsheet
void readFromFile(Spreadsheet *s, char *file_name){
  FILE *file = fopen(file_name, "rb");
  checkOpen(file, file_name);

  // Read the header
  fread(&s->columns, sizeof(int), 1, file);
  fread(&s->rows, sizeof(int), 1, file);

  Type *types = malloc(s->columns * sizeof(Type));
  fread(types, sizeof(Type), s->columns, file);
  s->column_types = types;

  // Read each column name (which should always occupy the full 81 bytes)
  char **names = malloc(s->columns * sizeof(char*));
  for (int i = 0; i < s->columns; i++){
    char *name = malloc(81 * sizeof(char));
    fread(name, sizeof(char), 81, file);
    names[i] = name;
  }
  s->column_names = names;

  // Precalculate the row size, to avoid recomputing it in a loop
  int size = rowSize(*s);

  // Read the spreadsheet's contents, row by row
  Row **row = &s->firstRow;
  for (int i = 0; i < s->rows; i++){
    *row = malloc(sizeof(**row));
    (*row)->entries = malloc(size * sizeof(char));
    fread((*row)->entries, sizeof(char), size, file);
    row = &(*row)->next;
  }
  *row = NULL; // Write a NULL to signal the end of the list of rows

  fclose(file);
}

void freeRow(Row *row){
  free(row->entries);
  free(row);
}

void freeRows(Row *row){
  // Free each of the rows, traversing the list
  while (row != NULL){
    Row current = *row;
    freeRow(row);
    row = current.next;
  }
}

// Frees the fields of a spreadsheet opened by readFromFile
void freeSpreadsheet(Spreadsheet s){
  for (int i = 0; i < s.columns; i++) free(s.column_names[i]);
  free(s.column_names);
  free(s.column_types);
  freeRows(s.firstRow);
}

// Deletes a row from a spreadsheet
void deleteRow(Spreadsheet *s, Row **prev, Row *curr){
  s->rows--;
  Row *next = curr->next;
  *prev = next;
  freeRow(curr);
}

// Exports a spreadsheet in csv (comma separated values) format to a file
void exportAsCsv(Spreadsheet s, char *file_name){
  FILE *file = fopen(file_name, "w");
  checkOpen(file, file_name);

  // Write the header
  for (int i = 0; i < s.columns; i++){
    if (i != 0) fprintf(file, ", ");
    fprintf(file, "%s", s.column_names[i]);
  }
  fprintf(file, "\n");

  // Write each row
  for (int r = 0; r < s.rows; r++){
    for (int c = 0; c < s.columns; c++){
      if (c != 0) fprintf(file, ", ");
      printCellByIndex(s, r, c, file);
    }
    fprintf(file, "\n");
  }

  fclose(file);
}

// Creates an example spreadsheet, which might be useful for testing
Spreadsheet example(){
  Spreadsheet s;
  s.columns = 4;
  s.rows = 2;

  // Fill in the column names
  char* col_names[] = { "bool", "int", "str", "double" };
  s.column_names = malloc(s.columns * sizeof(char*));
  for (int i = 0; i < s.columns; i++){
    // TODO: Define a macro for the maximum string length, set to 81
    char *name = malloc(81 * sizeof(char));
    s.column_names[i] = name;
    strncpy(name, col_names[i], 81);
  }

  // Fill in the types
  Type types[] = { BOOL, INT, STRING, DOUBLE };
  s.column_types = malloc(s.columns * sizeof(Type));
  for (int i = 0; i < s.columns; i++) s.column_types[i] = types[i];

  // Allocate the rows
  Row **row = &s.firstRow;
  for (int i = 0; i < s.rows; i++){
    *row = malloc(sizeof(**row));
    (*row)->entries = malloc(rowSize(s));
    row = &(*row)->next;
  }
  *row = NULL;

  // Manually fill in the values arbitrarily
  *((bool *) getCell(s, 0, 0)) = true;
  *((bool *) getCell(s, 1, 0)) = false;
  *((int  *) getCell(s, 0, 1)) = 10;
  *((int  *) getCell(s, 1, 1)) = 20;

  strncpy(getCell(s, 0, 2), "hello", 81);
  strncpy(getCell(s, 1, 2), "world", 81);

  *((double *) getCell(s, 0, 3)) = 1.5;
  *((double *) getCell(s, 1, 3)) = 2.9;

  return s;
}
