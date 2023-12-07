#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>

#include "spreadsheets.h"
#include "interface.h"

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
      fprintf(file, * (bool*) cell ? "verdadeiro" : "falso");
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
      badType(t, "printCell");
  }
}

// Prints to a file the cell of a spreadsheet at the chosen row and column
void printCellByIndex(Spreadsheet s, int row, int col, FILE *file){
  printCell(s.column_types[col], getCell(s, row, col), file);
}

// Ensures a file has been opened successfully (Exits the program if wanted)
bool checkOpen(FILE *file, char *file_name, bool leave){
  if (file != NULL) return true;
  printf("Não foi possível abrir arquivo %s", file_name);
  if (leave) exit(1);
  return false;
}

// Proceeds the initialization of a spreadsheet, for the sake of convenience
void initializeSpreadsheet(Spreadsheet *s){
  s->columns = 0;
  s->rows = 0;
  s->firstRow = NULL;
  s->column_names = NULL;
  s->column_types = NULL;
}

// Writes a spreadsheet to a file (in a binary format)
void writeToFile(Spreadsheet s, char *file_name){
  FILE *file = fopen(file_name, "wb");
  checkOpen(file, file_name, true);

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
bool readFromFile(Spreadsheet *s, char *file_name){
  FILE *file = fopen(file_name, "rb");
  if (!checkOpen(file, file_name, false)) return false;

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

  return true;
}

void freeRow(Row *row){
  free(row->entries);
  free(row);
}

// Free each of the rows, traversing the list
void freeRows(Row *row){
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

// Deletes a row from a spreadsheet by its index
void deleteRowByIndex(Spreadsheet *s, int pos){
  Row *prev = s->firstRow;
  Row *clear;

  if (pos == 0){
    s->firstRow = prev->next;
    freeRow(prev);
    s->rows--;
    return;
  }

  for (int i = 0; i < pos-1; i++) prev = prev->next;

  clear = prev->next;
  prev->next = clear->next;
  freeRow(clear);
  s->rows--;
}

// Adds a new row in a specified index
void addRow(Spreadsheet *s, int row1) {
  Row *newRow = malloc(sizeof(*newRow));
  newRow->entries = malloc(rowSize(*s));

  if (s->rows == 0){
    s->firstRow = newRow;
    newRow->next = NULL;
  }
  else if (row1 == 0){
    newRow->next = s->firstRow;
    s->firstRow = newRow;
  }
  else {
    Row **currentRow = &(s->firstRow);
    for (int i = 0; i < row1 - 1; i++) {
      currentRow = &(*currentRow)->next;
    }
    newRow->next = (*currentRow)->next;
    (*currentRow)->next = newRow;
  }

  s->rows++;
  for (int i = 0; i < s->columns; i++) {
    updateCellValue(*s, row1, i);
  }
}

// Updates the value of a cell at a specific position
void updateCellValue(Spreadsheet s, int row, int col) {
  bool bool_entry, error;
  int int_entry;
  char entry[81];
  double double_entry;
  while (true){
    error = false;

    printf("\nDigite o novo valor da célula da linha <%d> e campo <%s> : ", row, s.column_names[col]);
    strcpy(entry, "");
    scanf("%[^\n]", entry);
    getchar();

    switch (s.column_types[col]) {
      case BOOL:
        if (strcmp(entry, "verdadeiro") != 0 && strcmp(entry, "falso") != 0){
          printf("\nInsira um valor booleano válido.\n");
          error = true;
          break;
        }
        else if (strcmp(entry, "verdadeiro") == 0) bool_entry = true;
        else bool_entry = false;
        *((bool *) getCell(s, row, col)) = bool_entry;
        break;
      case INT:
        if (atoi(entry) != 0 || strcmp(entry, "0") == 0) {
          int_entry = atoi(entry);
          *((int *) getCell(s, row, col)) = int_entry;
        }
        else {
          printf("\nInsira um valor inteiro válido.\n");
          error = true;
        }
        break;
      case STRING:
        strncpy(getCell(s, row, col), entry, 81);
        break;
      case DOUBLE:
        if (atof(entry) != 0 || strcmp(entry, "0") == 0) {
          double_entry = atof(entry);
          *((double *) getCell(s, row, col)) = double_entry;
        }
        else {
          printf("\nnsira um valor racional válido.\n");
          error = true;
        }
        break;
      default:
        badType(s.column_types[col], "updateCellValue");
    }

    if (!error) break;
  }
}

// Adds a column after the last column
void addColumn(Spreadsheet *s, char* colName, Type type) {
  s->columns++;
  s->column_names = realloc(s->column_names, (s->columns)*(sizeof(char*)));
  s->column_types = realloc(s->column_types ,(s->columns)*(sizeof(Type)));
  s->column_names[s->columns-1] = malloc(81);
  strncpy(s->column_names[s->columns-1], colName, 81);
  s->column_types[s->columns-1] = type;
  Row *currentRow = s->firstRow;
  while (currentRow != NULL) {
    currentRow->entries = realloc(currentRow->entries, rowSize(*s));
    currentRow = currentRow->next;
  }

  if (s->rows == 0) return;

  for (int row = 0; row < s->rows; row++) {
    updateCellValue(*s, row, s->columns-1);
  }
};

// Removes a column from a spreedsheet
void removeColumn(Spreadsheet *s, int col) {

  // MOVE TYPES AND NAMES TO THE LEFT AFTER COL

  Type *auxTypeOld, *auxTypeNew;
  char **auxNamesOld, **auxNamesNew;

  auxNamesOld = (char**) malloc((s->columns)*sizeof(char*));
  auxNamesNew = (char**) malloc((s->columns-1)*sizeof(char*));
  auxTypeOld = (Type*) malloc((s->columns)*sizeof(Type));
  auxTypeNew = (Type*) malloc((s->columns-1)*sizeof(Type));

  for (int i = 0; i < s->columns-1; i++) {
    *(auxNamesNew+i) = (char *) malloc(81);
  } //alloc memory for pointers

  for (int i = 0; i < s->columns; i++) {
    *(auxNamesOld+i) = (char *) malloc(81);
  } //alloc memory for pointers

  auxNamesOld = s->column_names; //for free the old memory region
  auxTypeOld = s->column_types; //for free the old memory region

  for (int i = 0; i < col; i++) { //copy values between 0 and col to the new memory region
    *(auxTypeNew+i) = s->column_types[i];
    strncpy(*(auxNamesNew+i), s->column_names[i], 81);
  }

  for (int i = col; i < s->columns-1; i++) { //copy values between col+1 and s->columns to the new memory region
    *(auxTypeNew+i) = s->column_types[i+1];
    strncpy(*(auxNamesNew+i), s->column_names[i+1], 81); 
  }

  // MOVE CELLS TO THE LEFT AFTER COL
  int oldOffSet = 0;
  for (int i = 0; i < s->columns; i++) {
    oldOffSet += sizeOfType(auxTypeOld[i]);
  }

  int newOffSet = 0;
  for (int i = 0; i < s->columns-1; i++) {
    newOffSet += sizeOfType(auxTypeNew[i]);
  }

  void **newMemoryRegion = (void**)malloc((s->rows)*sizeof(void*));
  for (int i = 0; i < (s->rows); i++) {
    *(newMemoryRegion+i) = malloc(newOffSet);
  }
  Row *aux = s->firstRow;
  
  int row = 0;
  int moveBytes = 0;

  while (row < s->rows) { //copy data to the new memory region
    char *deleteEntries;
    for (int c = 0; c < col; c++) {
      switch(auxTypeNew[c]) {
        case BOOL:
          *((bool*) *(newMemoryRegion+row)+moveBytes) = *((bool *) getCell(*s, row, c));
          break;
        case STRING:
          strncpy(*(newMemoryRegion+row)+moveBytes, getCell(*s, row, c), 81);
          break;
        case INT:
          // *((int*) *(newMemoryRegion+row)+moveBytes) = *((int *) getCell(*s, row, c));
          memmove(*(newMemoryRegion+row)+moveBytes, (int *)getCell(*s, row, c), 4);
          break;
        case DOUBLE:
          //*((double*) *(newMemoryRegion+row)+moveBytes) = *((double *)getCell(*s, row, c));
          memmove(*(newMemoryRegion+row)+moveBytes, (double *)getCell(*s, row, c), 8);
          break;
      }

      moveBytes += sizeOfType(auxTypeNew[c]);
    }

    for (int c = col; c < s->columns-1; c++) {
      switch(auxTypeNew[c]) {
        case BOOL:
          *((bool*) *(newMemoryRegion+row)+moveBytes) = *((bool *) getCell(*s, row, c+1));
          break;
        case STRING:
          strncpy(*(newMemoryRegion+row)+moveBytes, getCell(*s, row, c+1), 81);
          break;
        case INT:
          //*((int*) *(newMemoryRegion+row)+moveBytes) =  *((int *) getCell(*s, row, c+1));
          memmove(*(newMemoryRegion+row)+moveBytes, (int *)getCell(*s, row, c+1), 4);
          break;
        case DOUBLE:
          //*((double*) *(newMemoryRegion+row)+moveBytes) =  *((double *) getCell(*s, row, c+1));
          memmove(*(newMemoryRegion+row)+moveBytes, (double *)getCell(*s, row, c+1), 8);
          break;
      }

      moveBytes += sizeOfType(auxTypeNew[c]);
    } 
    
    deleteEntries = aux->entries;
    aux->entries = (char*)*(newMemoryRegion + row);
    free(deleteEntries);

    aux = aux->next;
    row++;
    moveBytes = 0;
  }

  s->column_names = realloc(s->column_names, (s->columns-1)*(sizeof(char*)));
  s->column_types = realloc(s->column_types ,(s->columns-1)*(sizeof(Type)));

  free(auxNamesOld);
  free(auxTypeOld);

  s->column_names = auxNamesNew;
  s->column_types = auxTypeNew;

  s->columns--;
}

//Sort the rows of a Spreedsheet by the value of cells of a specific col in ascending order
void ascendingSortByValue(Spreadsheet *s, int col) {
  int qntRows = s->rows;
  bool done = false;
  Row *aux = s->firstRow;

  while(!done) {
    done = true;
    for (int i = 0; i < qntRows-1; i++) {
      if (*getCell(*s, i+1, col) < *getCell(*s, i, col) ) {
        if (i != 0) {
          Row *aux2 = getRow(*s, i+1)->next;
          Row *aux3 = getRow(*s, i);
          Row *aux4 = getRow(*s, i-1);
          aux4->next = getRow(*s, i+1);
          aux3->next = aux2;
          getRow(*s, i)->next = aux3;
          done = false;
        } else {
          Row *aux2 = aux;
          s->firstRow = getRow(*s, i+1);
          aux2->next = s->firstRow->next;
          s->firstRow->next = aux2;
          done = false;
        }
      }
    }
    qntRows--;
  }
}

//Sort the rows of a Spreedsheet by the value of cells of a specific col in descending order
void descendingSortByValue(Spreadsheet *s, int col) {
  int qntRows = s->rows;
  bool done = false;
  Row *aux = s->firstRow;

  while(!done) {
    done = true;
    for (int i = 0; i < qntRows-1; i++) {
      if (*getCell(*s, i+1, col) > *getCell(*s, i, col) ) {
        if (i != 0) {
          Row *aux2 = getRow(*s, i+1)->next;
          Row *aux3 = getRow(*s, i);
          Row *aux4 = getRow(*s, i-1);
          aux4->next = getRow(*s, i+1);
          aux3->next = aux2;
          getRow(*s, i)->next = aux3;
          done = false;
        } else {
          Row *aux2 = aux;
          s->firstRow = getRow(*s, i+1);
          aux2->next = s->firstRow->next;
          s->firstRow->next = aux2;
          done = false;
        }
      }
    }
    qntRows--;
  }
}

//Sort the rows of a Spreedsheet by the name of cells of a specific col
void sortByAlphabet(Spreadsheet *s, int col) {
  int qntRows = s->rows;
  bool done = false;
  Row *aux = s->firstRow;

  while(!done) {
    done = true;
    for (int i = 0; i < qntRows-1; i++) {
      if (strcmp(getCell(*s, i, col), getCell(*s, i+1, col)) > 0) {
        if (i != 0) {
          Row *aux2 = getRow(*s, i+1)->next;
          Row *aux3 = getRow(*s, i);
          Row *aux4 = getRow(*s, i-1);
          aux4->next = getRow(*s, i+1);
          aux3->next = aux2;
          getRow(*s, i)->next = aux3;
          done = false;
        } else {
          Row *aux2 = aux;
          s->firstRow = getRow(*s, i+1);
          aux2->next = s->firstRow->next;
          s->firstRow->next = aux2;
          done = false;
        }
      }
    }
    qntRows--;
  }
}

// Exports a spreadsheet in csv (comma separated values) format to a file
void exportAsCsv(Spreadsheet s, char *file_name){
  FILE *file = fopen(file_name, "w");
  checkOpen(file, file_name, true);

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

// Displays the Spreadsheet in a beautiful way
void displaySpreadsheet(Spreadsheet s){
  int* columnWidths = (int*)malloc(s.columns * sizeof(int));
  char buffer[81];
  int currentSize;
    
  // Calculate the maximum width for each column
  for (int col = 0; col < s.columns; col++) {
    columnWidths[col] = strlen(s.column_names[col]);   
    if (s.column_types[col] == BOOL && columnWidths[col] > 10) continue;

    Row* currentRow = s.firstRow;

    while (currentRow != NULL) {
      char* cell = currentRow->entries + columnOffset(s, col);
      switch (s.column_types[col]){
        case STRING:
          currentSize = strlen(cell)+2;
          if (columnWidths[col] < currentSize){
                columnWidths[col] = currentSize;
          }
          break;
        case BOOL:
          if (*(bool*) cell){
              columnWidths[col] = 10;
          }
          else if (columnWidths[col] < 5){
              columnWidths[col] = 5;
          }
          break;
        case INT:
          snprintf(buffer, 81, "%d", *(int*) cell);
          currentSize = strlen(buffer);
          if (columnWidths[col] < currentSize){
              columnWidths[col] = currentSize;
          }
          break;
        case DOUBLE:
          snprintf(buffer, 81, "%g", *(double*) cell);
          currentSize = strlen(buffer);
          if (columnWidths[col] < currentSize){
              columnWidths[col] = currentSize;
          }
          break;
        default:
          badType(s.column_types[col], "displaySpreadsheet");
      }
            
      if (s.column_types[col] == BOOL && (*(bool*) cell)) break;
      currentRow = currentRow->next;
    }
  }

  // Width of the spreadsheet
  int auxinicio=0;
  for(int i=0; i<s.columns; i++){
    auxinicio+=columnWidths[i];
  }

  // Printf the header
  printf("+");
  fill('-', auxinicio+s.columns-1);
  printf("+\n|");

  // Prints the name of each section (each collumn)
  for (int col = 0; col < s.columns; col++){
    int size=strlen(s.column_names[col]);
    printf("%s", s.column_names[col]);
    fill(' ', columnWidths[col]-size);
    printf("|");
  }
  printf("\n+");
  fill('-', auxinicio+s.columns-1);
  printf("+\n");


  Row *current = s.firstRow; // pointer starts by pointing to the first row
  while (current != NULL){     // while the row exists
    for (int col=0; col<s.columns; col++){ // advances to the next column
      printf("|");
      char *cell = current->entries + columnOffset(s, col); //cell points to the space in memory where the data is stored
      Type cellType = s.column_types[col];  // celltype recieves the type of the column
      // prints the information of the cell depending on its type
      printCell(cellType, cell, stdout);
      int cellSize;
      
      switch (cellType){
        case STRING:
          cellSize = strlen(cell)+2;
          break;
        case BOOL:
          cellSize = (*(bool*) cell) ? 10 : 5;
          break;
        case INT:
          snprintf(buffer, 81, "%d", *(int*) cell);
          cellSize = strlen(buffer);
          break;
        case DOUBLE:
          snprintf(buffer, 81, "%g", *(double*) cell);
          cellSize = strlen(buffer);
          break;
        default:
          badType(cellType, "displaySpreadsheet");
      }
      
      fill(' ', columnWidths[col]-cellSize);
    }
    printf("|\n");                   // \n to the next row
    current = current->next;  // pointer points to the next row
  }
  // prints the bottom header
  printf("+");
  fill('-', auxinicio + s.columns-1);
  printf("+\n");

  // frees the dynamic array
  for (int col = 0; col < s.columns; col++) {
      free(columnWidths[col]);
  }

}


/*
void ascendingSortByValue(Spreadsheet *s, int col) {
  int qntRows = s->rows;
  bool done = false;
  Row *aux = s->firstRow;
  bool comp;

  if (s->column_types[col] == STRING || s->column_types[col] == BOOL) return;

  while(!done) {
    done = true;
    for (int i = 0; i < qntRows-1; i++) {
      if (s->column_types[col] == INT) comp = *(int*)getCell(*s, i+1, col) < *(int*)getCell(*s, i, col);
      else comp = *(double*)getCell(*s, i+1, col) < *(double*)getCell(*s, i, col);
      if (comp) {
        if (i != 0) {
          Row *aux2 = getRow(*s, i+1)->next;
          Row *aux3 = getRow(*s, i);
          Row *aux4 = getRow(*s, i-1);
          aux4->next = getRow(*s, i+1);
          aux3->next = aux2;
          getRow(*s, i)->next = aux3;
          done = false;
        } else {
          Row *aux2 = aux;
          s->firstRow = getRow(*s, 1);
          aux2->next = s->firstRow->next;
          s->firstRow->next = aux2;
          done = false;
        }
      }
    }
    qntRows--;
  }
}
*/
