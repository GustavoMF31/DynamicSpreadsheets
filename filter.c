#include<stdbool.h>
#include<string.h>
#include<stdio.h>

#include "filter.h"
#include "spreadsheets.h"

// Determines the value associated to a term, looking up, if necessary,
// the data in the mentioned spreadsheet column
Literal evaluate(Spreadsheet s, char *entries, Term t){
  if (t.isLiteral) return t.expr.constant;
  int i = columnIndex(s, t.expr.columnName);
  // TODO: Handle errors (i == -1)

  Literal result;
  result.literalType = s.column_types[i];
  char *data = entries + columnOffset(s, i);
  switch (result.literalType) {
    case BOOL:
      result.literal.boolValue = *(bool*) data;
      break;
    case STRING:
      memcpy(result.literal.stringValue, data, 81);
      break;
    case INT:
      result.literal.intValue = *(int*) data;
      break;
    case DOUBLE:
      result.literal.doubleValue = *(double*) data;
      break;
    default:
      badType(result.literalType, "evaluate");
  }

  return result;
}

// Converts a literal, which may not be of Type STRING, to a double
double literalAsDouble(Literal a){
  double x;
  switch (a.literalType) {
    case BOOL:
      x = (double) a.literal.boolValue; break;
    case INT:
      x = (double) a.literal.intValue; break;
    case DOUBLE:
      x = a.literal.doubleValue; break;
    case STRING:
    default:
      badType(a.literalType, "literalAsDouble");
  }

  return x;
}

// Compares two literals according to their types and the comparison
// function specified
bool compare(Literal a, Literal b, Comparison op){
  if (a.literalType == STRING && b.literalType == STRING){
    int result = strcmp(a.literal.stringValue, b.literal.stringValue);
    if (result == 0) return op == EQ || op == GTE || op == LTE;
    if (result < 0) return op == LT || op == LTE;
    if (result > 0) return op == GT || op == GTE;
  }
  // TODO: What if only one of them is a string?

  // For ease of implementation, other datatypes may be converted to
  // doubles for comparison (TODO: consider the consequences of this)
  double c = literalAsDouble(a);
  double d = literalAsDouble(b);

  if (c < d) return op == LT || op == LTE;
  if (c > d) return op == GT || op == GTE;
  else return op == EQ || op == GTE || op == LTE;
}

// Checks if a row satisfies the condition specified by a filter
bool checkCondition(Spreadsheet s, char *entries, Filter cond){
  // Evaluate the terms
  Literal a = evaluate(s, entries, cond.leftTerm);
  Literal b = evaluate(s, entries, cond.rightTerm);

  // Then compare them
  return compare(a, b, cond.op);
}

// Removes every entry of a spredsheet that doesn't satisfy a filter
void filterSpreadsheet(Spreadsheet *s, Filter cond){

  // Iterate over the list of rows
  for (Row **row = &s->firstRow; *row != NULL; ){
    // If a row passes the filter, we skip it,
    // but, otherwise, it gets deleted.
    if (checkCondition(*s, (*row)->entries, cond)) row = &(*row)->next;
    else deleteRow(s, row, *row);
  }
}
