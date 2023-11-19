#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>

// Data types supported by the spreadsheets
// (TODO: Should we have a separate "CHAR" type?)
typedef enum {
  BOOL,
  STRING, // A stored string may only have up to 80 characters
  INT,
  DOUBLE
} Type;

typedef struct {
  int columns; // Amount of columns
  int rows;    // Amount of rows
  Type *column_types;  // One type per column
  char **column_names; // Holds the name of each column.
                       // Each name is expected to be in an 81 byte long buffer
  char *data; // Points to the raw data (bytes) of the spreadsheet
} Spreadsheet;

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
      printf("Unrecognized type %d\n", t);
      exit(1);
  }
}

// Returns the index of the data for a given column within a row
// Assumes 0 <= col < s.columns
int columnOffset(Spreadsheet s, int col){
  int offset = 0;
  for (int i = 0; i < col; i++)
    offset += sizeOfType(s.column_types[i]);

  return offset;
}

// Returns the size (in bytes) occupied by a given row of the spreadsheet
int rowSize(Spreadsheet s){
  return columnOffset(s, s.columns);
}

// Returns the size (in bytes) of the data buffer of a given spreadsheet
int dataSize(Spreadsheet s){
  return s.rows * rowSize(s);
}

// Given row and column indices, returns a pointer to the cell they specify
char* getCell(Spreadsheet s, int row, int col){
  return s.data + rowSize(s) * row + columnOffset(s, col);
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

  // Write the actual data
  fwrite(s.data, sizeof(char), dataSize(s), file);

  fclose(file);
}

// Reads a spreadsheet from a binary file onto the given pointer.
// Memory is allocated to hold buffers (column_types, column_names and data)
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

  // Read the spreadsheet contents
  s->data = malloc(dataSize(*s));
  fread(s->data, sizeof(char), dataSize(*s), file);

  fclose(file);
}

// Frees the fields of a spreadsheet opened by readFromFile
void freeSpreadsheet(Spreadsheet s){
  for (int i = 0; i < s.columns; i++) free(s.column_names[i]);
  free(s.column_names);
  free(s.column_types);
  free(s.data);
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
}

int main(){
  // Initialize a test spreadsheet, then display it
  Spreadsheet s;
  s.columns = 4;
  s.rows = 2;

  char* col_names[] = { "bool", "int", "str", "double" };
  s.column_names = col_names;
  Type types[] = { BOOL, INT, STRING, DOUBLE };
  s.column_types = types;

  s.data = malloc(dataSize(s));

  // Manually fill in the values arbitrarily
  *((bool *) getCell(s, 0, 0)) = true;
  *((bool *) getCell(s, 1, 0)) = false;
  *((int  *) getCell(s, 0, 1)) = 10;
  *((int  *) getCell(s, 1, 1)) = 20;

  memcpy(getCell(s, 0, 2), "hello", 6);
  memcpy(getCell(s, 1, 2), "world", 6);

  *((double *) getCell(s, 0, 3)) = 1.5;
  *((double *) getCell(s, 1, 3)) = 2.9;

  // Display it to make sure everything is ok
  displaySpreadsheet(s);

  // Let's write it to disk and read it back
  writeToFile(s, "test_spreadsheet.data");
  Spreadsheet s2;
  readFromFile(&s2, "test_spreadsheet.data");

  // Then check the result is the same
  printf("\n");
  displaySpreadsheet(s2);

  // Export it as a csv file
  exportAsCsv(s2, "test.csv");

  freeSpreadsheet(s2);
  free(s.data);
  return 0;
}
