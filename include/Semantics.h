#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

/*
 * Representation of various elements related to the pseudocode  
 */

// Supported datatypes
typedef enum {
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_COUNT
} DataType;

// Supported operations
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQUAL,
    OP_BRANCH,
    OP_COUNT
} OperationType;

// How can expression is written. List of supported syntaxes for assigning a value to a variable
// The order matters, as the regular assignment is more common than the ones above and it will be detected first.
typedef enum {
    STATEMENT_COMPOUND_ASSIGNMENT,          // var[i] += var2[i]
    STATEMENT_ASSIGMENT_AFTER_OPERATION,    // var[i] = var[i] + var2[i] 
    STATEMENT_ASSIGNMENT,                   // var[i] = 1
    STATEMENT_COUNT
} StatementType;

// The representation of a variable
typedef struct {
    unsigned long address;  // The address of the variable. Stablished by the user
    std::string name;
    DataType type;          // The number of bytes the array 
} Variable;

// The representation of a for loop
typedef struct {
    Variable* iterator;
    long startValue;
    long limitValue;
    long step;
} ForLoop;

// The representation of an operation that will get interpreted.
typedef struct {
    // uintptr_t can contain both pointers to variables or unsigned long scalars. It can be casted in a C-like style safely
    uintptr_t dest;
    uintptr_t indexDest;
    uintptr_t op1;
    uintptr_t indexOp1;
    uintptr_t op2;
    uintptr_t indexOp2;

    bool isOp1Var, isOp2Var, isDestIndexVar, isIndexOp1Var, isIndexOp2Var;
    OperationType opType;
} Operation;

// Public functions
std::string DataTypeToString(DataType type);
std::string StatementOperatorToString(OperationType opType, StatementType staType);