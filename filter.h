#pragma once

#include "spreadsheets.h"
#include "interface.h"

// An enum for the kinds of comparisons filtering queries may perform
// (TODO: Are these all the comparisons we will implement?)
typedef enum {
  GT,  // Greater than
  GTE, // Greater than or equal to
  LT,  // Less than
  LTE, // Less than or equal to
  EQ   // Equal to
} Comparison;

typedef struct {
  Type literalType;   // The type of the literal (INT, STRING, ...)
  union {
    bool boolValue;
    char stringValue[81];
    int intValue;
    double doubleValue;
  } literal; // And the literal itself. "literalType" informs which
             // of these should be accessed.
} Literal;

typedef struct {
  bool isLiteral; // Indicates whether a term refers to a column name,
                  // or is a constant literal
  union {
    char columnName[81];
    Literal constant;
  } expr; // The term's expression, which might
          // mention a column or contain a constant literal
} Term;

typedef struct {
  Term leftTerm;
  Comparison op;
  Term rightTerm;
} Filter;

bool inputToFilter(Spreadsheet s, char *buffer, Filter *cond);
void filterSpreadsheet(Spreadsheet *s, Filter cond);