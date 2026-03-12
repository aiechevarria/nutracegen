#include "Semantics.h"

/**
 * Maps a DataType to a string. 
 * 
 * @param type The DataType.
 * @return string The string that it relates to.
 */
string DataTypeToString(DataType type) {
    switch (type)
    {
        case TYPE_CHAR:   return "char";
        case TYPE_SHORT:  return "short";
        case TYPE_INT:    return "int";
        case TYPE_LONG:   return "long";
        case TYPE_FLOAT:  return "float";
        case TYPE_DOUBLE: return "double";
        default:          return "unknown";
    }
}

/**
 * Gets the operator string for an OperationType with a given StatementType
 * 
 * @param opType The OperationType.
 * @param staType The StatementType.
 * @return string The string that it relates to.
 */
string StatementOperatorToString(OperationType opType, StatementType staType) {
    if (staType == STATEMENT_ASSIGNMENT) return "=";

    switch (opType)
    {
        case OP_ADD:
            if (staType == STATEMENT_COMPOUND_ASSIGNMENT) return "+=";
            if (staType == STATEMENT_ASSIGMENT_AFTER_OPERATION) return "+";
        case OP_SUB:
            if (staType == STATEMENT_COMPOUND_ASSIGNMENT) return "-=";
            if (staType == STATEMENT_ASSIGMENT_AFTER_OPERATION) return "-";
        case OP_MUL:
            if (staType == STATEMENT_COMPOUND_ASSIGNMENT) return "*=";
            if (staType == STATEMENT_ASSIGMENT_AFTER_OPERATION) return "*";
        case OP_DIV:
            if (staType == STATEMENT_COMPOUND_ASSIGNMENT) return "/=";
            if (staType == STATEMENT_ASSIGMENT_AFTER_OPERATION) return "/";
        default:          return "?";
    }
}

/**
 * Maps an OperandType to a string. 
 * 
 * @param type The Operand
 * @return string The string that it relates to.
 */
string OperandTypeToString(OperandType type) {
    switch (type)
    {
        case OPR_DESTINATION:   return "Dest";
        case OPR_OP1:           return "OPR1";
        case OPR_OP2:           return "OPR2";
        default:                return "unknown";
    }
}

/**
 * Find a variable based on it's name.
 * 
 * @param vars The list of variables that have been identified.
 * @param name The name of the variable to find.
 * @return Variable* Pointer to the variable if found, nullptr if error.
 */
Variable* getVariableByName(vector<Variable>* vars, string name) {
    for (auto& v : *vars) {
        if(name == v.name) return &v;
    }

    return nullptr;
}