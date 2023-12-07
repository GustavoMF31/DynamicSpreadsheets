#include<stdbool.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

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

// Transforms a textual expression to a valid Filter variable and its attributes
bool inputToFilter(Spreadsheet s, char *buffer, Filter *cond){
  char leftTerm[81], operator[81], rightTerm[81];
  bool leftTermFound = false, rightTermFound = false;
  Type leftTermType, rightTermType;

  // Improved input that accepts field names with spaces
  sscanf(buffer, "%[^><=] %[><=] %[^\n]", leftTerm, operator, rightTerm);
  leftTerm[strlen(leftTerm)-1] = '\0';
                      
  // Iterating on the column names to find out if the left term and/or the right term exist
  for (int i = 0; i < s.columns; i++){
    if (strcmp(leftTerm, s.column_names[i]) == 0){
      leftTermFound = true;
      leftTermType = s.column_types[i];
    }
    if (strcmp(rightTerm, s.column_names[i]) == 0){
      rightTermFound = true;
      rightTermType = s.column_types[i];
    }
  }
  
  // Taking care of the disparities between the types of the left and right terms
  if (!leftTermFound && !rightTermFound){
    printf("\nDeve haver pelo menos um campo no filtro.\n\n");
    return false;
  }
  else if (leftTermFound && rightTermFound && leftTermType != rightTermType){
    printf("\nOs tipos dos campos inseridos são incompatíveis!\n\n");
    return false;
  }
  else if ((!rightTermFound && (leftTermType == INT) && (atoi(rightTerm) == 0) && (strcmp(rightTerm, "0") != 0)) ||
          (!leftTermFound && (rightTermType == INT) && (atoi(leftTerm) == 0) && (strcmp(leftTerm, "0") != 0))){
    printf("\nO valor do literal deveria ser um inteiro!\n\n");
    return false;
  }
  else if ((!rightTermFound && (leftTermType == DOUBLE) && (atof(rightTerm) == 0) && (strcmp(rightTerm, "0") != 0)) ||
          (!leftTermFound && (rightTermType == DOUBLE) && (atof(leftTerm) == 0) && (strcmp(leftTerm, "0") != 0))){
    printf("\nO valor do literal deveria ser um racional!\n\n");
    return false;
  }
  else if ((!rightTermFound && (leftTermType == BOOL) && (strcmp(rightTerm, "verdadeiro") != 0) && (strcmp(rightTerm, "falso") != 0)) || 
          (!leftTermFound && (rightTermType == BOOL) && (strcmp(leftTerm, "verdadeiro") != 0) && (strcmp(leftTerm, "falso") != 0))){
    printf("\nO valor do literal deveria ser um booleano!\n\n");
    return false;
  }
  else if ((!rightTermFound && (leftTermType == STRING) && (rightTerm[0] != '"') && (rightTerm[strlen(rightTerm)-1] != '"')) || 
          (!leftTermFound && (rightTermType == STRING) && (leftTerm[0] != '"') && (leftTerm[strlen(leftTerm)-1] != '"'))){
    printf("\nO valor do literal deveria ser uma string delimitada por aspas duplas!\n\n");
    return false;
  }

  if (leftTermFound){ // If the left term is a column
    cond->leftTerm.isLiteral = false;
    strncpy(cond->leftTerm.expr.columnName, leftTerm, 81);
  }
  else { // Else, it is a literal
    cond->leftTerm.isLiteral = true;
    cond->leftTerm.expr.constant.literalType = rightTermType;
    switch (cond->leftTerm.expr.constant.literalType){
      case INT:
        cond->leftTerm.expr.constant.literal.intValue = atoi(leftTerm);
        break;
      case DOUBLE:
        cond->leftTerm.expr.constant.literal.doubleValue = atof(leftTerm);
        break;
      case STRING:
        leftTerm[strlen(leftTerm)-1] = '\0';
        sscanf(leftTerm, "\"%s", leftTerm);
        strncpy(cond->leftTerm.expr.constant.literal.stringValue, leftTerm, 81);
        break;
      case BOOL:
        cond->leftTerm.expr.constant.literal.boolValue = (strcmp(leftTerm, "verdadeiro") == 0) ? true : false;
        break;
      default:
        badType(rightTermType, "inputToFilter");
    }
  }

  if (rightTermFound){ // If the right term is a column
    cond->rightTerm.isLiteral = false;
    strncpy(cond->rightTerm.expr.columnName, rightTerm, 81);
  }
  else { // Else, it is a literal
    cond->rightTerm.isLiteral = true;
    cond->rightTerm.expr.constant.literalType = leftTermType;
    switch (cond->rightTerm.expr.constant.literalType){
      case INT:
        cond->rightTerm.expr.constant.literal.intValue = atoi(rightTerm);
        break;
      case DOUBLE:
        cond->rightTerm.expr.constant.literal.doubleValue = atof(rightTerm);
        break;
      case STRING:
        rightTerm[strlen(rightTerm)-1] = '\0';
        sscanf(rightTerm, "\"%s", rightTerm);
        strncpy(cond->rightTerm.expr.constant.literal.stringValue, rightTerm, 81);
        break;
      case BOOL:
        cond->rightTerm.expr.constant.literal.boolValue = (strcmp(rightTerm, "verdadeiro") == 0) ? true : false;
        break;
      default:
        badType(leftTermType, "inputToFilter");
    }
  }

  // Checking the operator
  if (strcmp(operator, ">") == 0) cond->op = GT;
  else if (strcmp(operator, ">=") == 0) cond->op = GTE;
  else if (strcmp(operator, "<") == 0) cond->op = LT;
  else if (strcmp(operator, "<=") == 0) cond->op = LTE;
  else if (strcmp(operator, "=") == 0) cond->op = EQ;
  else {
    printf("\nO operador inserido é inválido.\n\n");
    return false;
  }

  if (cond->op != EQ && (leftTermType == BOOL || rightTermType == BOOL)){
    printf("\nSomente são aceitas comparações de igualdade para o tipo booleano.\n\n");
    return false;
  }

  return true;
}

// Removes every entry of a spreadsheet that doesn't satisfy a filter
void filterSpreadsheet(Spreadsheet *s, Filter cond){
  // Iterate over the list of rows
  for (Row **row = &s->firstRow; *row != NULL;){
    // If a row passes the filter, we skip it,
    // but, otherwise, it gets deleted.
    if (checkCondition(*s, (*row)->entries, cond)) row = &(*row)->next;
    else deleteRow(s, row, *row);
  }
}