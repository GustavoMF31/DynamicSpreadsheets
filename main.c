#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>

#include "spreadsheets.h"
#include "filter.h"

int main(){
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
  displaySpreadsheet(s2);

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
  return 0;
}
