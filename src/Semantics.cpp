#include "Semantics.h"

/**
 * Maps a DataType to a string. 
 * 
 * @param type The DataType.
 * @return std::string The string that it relates to.
 */
std::string DataTypeToString(DataType type) {
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
 * @return std::string The string that it relates to.
 */
std::string StatementOperatorToString(OperationType opType, StatementType staType) {
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