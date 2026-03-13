#pragma once

#include "Misc.h"

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
    OP_END,         // End of the interpretation
    OP_COUNT
} OperationType;

// Branch types. processConditional() relies on the order of this enum, please do not move it !
typedef enum {
    B_AL,           // Branch always.
    B_EQ,           // ==
    B_NE,           // !=
    B_GE,           // >=
    B_LE,           // <=
    B_GT,           // >
    B_LT,           // <
    B_COUNT
} BranchType;

// The number and name of operators in an instruction
typedef enum {
    OPR_DESTINATION,
    OPR_OP1,
    OPR_OP2,
    OPR_COUNT
} OperandType;

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
    string name;
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
    OperationType opType;
    BranchType bType;       // Only for branches

    // uintptr_t can contain both pointers to variables or unsigned long scalars. It can be casted in a C-like style safely
    // An instruction can have 2 (dest, op1) or 3 (dest, op1, op2) operands
    uintptr_t operands[OPR_COUNT];
    
    // If an operand contains a pointer to a variable, it can be indexed. If it is not indexed, a 0 should be added at all times
    uintptr_t indexes[OPR_COUNT];

    // The operands and indexes can be vars
    bool isOperandVar[OPR_COUNT];
    bool isIndexVar[OPR_COUNT];
} Operation;

// Public functions
string DataTypeToString(DataType type);
string OperationTypeToString(OperationType type);
string OperandTypeToString(OperandType type);
string BranchTypeTypeToString(BranchType type);
string BranchTypeTypeToOperator(BranchType type);
string StatementOperatorToString(OperationType opType, StatementType staType);
Variable* getVariableByName(vector<Variable>* vars, string name);