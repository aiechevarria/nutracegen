#include "Parser.h"

/**
 * Checks if the unit contains the content string and returns a boolean. 
 * 
 * @param unit The text in which content will be searched
 * @param content The string that must be in unit
 * @return true if exists, false if not
 */
bool hasString(string unit, string content) {
    if (unit.find(content) == string::npos) return false;
    return true;
}

/**
 * Get all the lines that match a given regex
 * 
 * @param text 
 * @param pattern 
 * @return vector<string> 
 */
vector<string> getMatchingLines(const string& text, const string& pattern) {
    // Parse the regular expression and create a container for all the matches
    regex re(pattern);
    vector<string> matches;

    istringstream stream(text);
    string line;

    // Iterate over all the liens in the stream
    while (getline(stream, line)) {
        // If the line matches the regex, store it in the matches bin
        if (regex_search(line, re)) matches.push_back(line);
    }

    return matches;
}

/**
 * Extracts the content of the input file to a string.
 * 
 * @param inputPath The path to the file
 * @return string The parsed file
 */
string readFileToString(char inputPath[MAX_PATH_LENGTH]) {
    char buffer[BUFFER_SIZE];
    string result;
    size_t bytesRead;

    // Open the file
    FILE* file = fopen(inputPath, "r");
    
    // Validate the file has been opened
    if (!file) {
        throw runtime_error(ERROR_FILE_OPEN);
    }

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        result.append(buffer, bytesRead);
    }

    // Validate no error has happened
    if (ferror(file)) {
        result.clear();
        fclose(file);
        throw runtime_error(ERROR_FILE_READ);
    }

    // Validate the file is not empty
    if (result.empty())  {
        fclose(file);
        throw runtime_error(ERROR_FILE_EMPTY);
    }

    fclose(file);
    printf("%s", INFO_FILE_READ);
    return result;
}

/**
 * Stores the string in a filepath.
 * 
 * @param filePath The path to the file.
 * @param content The content to store.
 */
void writeStringToFile(char* filePath, string& content) {
    // Check the file path is not empty
    if (filePath[0] == '\0') throw runtime_error(ERROR_FILE_SAVE_EMPTY);

    FILE* file = fopen(filePath, "w");

    if (file == NULL) throw runtime_error(ERROR_FILE_SAVE_EMPTY);

    fwrite(content.c_str(), 1, content.size(), file);

    fclose(file);

    printf("%s", INFO_FILE_SAVED);
}

/**
 * Extracts the variables from the pseudocode.
 * @param text The text to extract the variables from
 * @param variables A vector that stores all variables in the order they have been parsed
 */
void parseVariables(string text, vector<Variable>* variables) {
    // For a given datatype, check if the text has any occurences
    // For instance, get all the lines that have "int "
    // Once all the matches have been fetched, crop the datatype and the space until a ; [ = or space is met. That is the variable name

    // Iterate over each datatype 
    for (int d = TYPE_CHAR; d < TYPE_COUNT; d++) {
        DataType type = static_cast<DataType>(d);
        string typeStr = DataTypeToString(type);

        // Match for datatype + space + variable name (until space, ;, =, or [)
        string pattern = typeStr + R"(\s+([a-zA-Z_]\w*))";

        // Get all the matching lines for this regex
        vector<string> lines = getMatchingLines(text, pattern);

        // Iterate over every line
        for (const auto& line : lines) {
            smatch match;

            // Split the line into groups
            if (regex_search(line, match, regex(pattern))) {
                if (match.size() >= 2) {
                    // Capture the name and store the variable
                    string varName = match[1];
                    variables->push_back(Variable{0, varName, (DataType) d});

                    if(debug) printf("Debug: Found variable with name=%s, type=%s\n", varName.c_str(), DataTypeToString(type).c_str());
                }
            }
        }
    }

    if (variables->empty()) throw runtime_error(ERROR_PARSE_NOVAR);
}

/**
 * Pre processes the code. Removes tabs, spaces and adapts for loops.
 * 
 * @param code Pointer to the code.
 */
void preProcessCode(string* code) {
    // Remove tabs and newlines
    code->erase(remove(code->begin(), code->end(), '\t'), code->end());
    code->erase(remove(code->begin(), code->end(), '\n'), code->end());

    // Collapse multiple spaces into one
    *code = regex_replace(*code, regex("\\s+"), " ");

    // Add brackets to bracketless for loops
    // Group 1: The for header: for(...)
    // Group 2: The single statement: ...;
    regex bracketlessFor(R"((for\s*\([^)]+\))\s*([^{;]+;))");
    
    // Replace "for(...) statement;" with "for(...) { statement; }"
    *code = regex_replace(*code, bracketlessFor, "$1 { $2 }");
}

/**
 * Fetches the next logical unit (statement or for) from the code.
 *
 * @param code A pointer to the code. Elements will get removed from it.
 * @return The extracted piece of code.
 */
string extractNextUnit(string* code) {
    // Check that some code is left to process first
    if (code->empty()) return "";

    string unit = "";
    
    // Check if it starts with for
    if (code->substr(0, 3) == "for") {
        // Find the first bracket
        size_t openBracePos = code->find('{');
        // If there are no brackets, print an error and return nothing. This case should have been handled by the preprocessor.
        if (openBracePos == string::npos) {
            printf(ERROR_PARSE_FOR);
            return "";
        }
        
        // Bracket counters
        int braceCount = 0;
        size_t endPos = 0;

        // Start scanning from the opening brace
        for (size_t i = openBracePos; i < code->length(); ++i) {
            // Count brace openings and closings
            if ((*code)[i] == '{') braceCount++;

            else if ((*code)[i] == '}') braceCount--;

            if (braceCount == 0) {
                endPos = i;
                break;
            }
        }

        // Extract everything from the start of for to the closing bracket
        unit = code->substr(0, endPos + 1);
        code->erase(0, endPos + 1);
    } else {
        // Standard statement: extract up to the first semicolon
        size_t semiColonPos = code->find(';');
        if (semiColonPos != string::npos) {
            unit = code->substr(0, semiColonPos + 1);
            code->erase(0, semiColonPos + 1);
        }
    }

    return unit;
}

/**
 * Extracts the information related to a single operand 
 * 
 * @param unit The isolated operand
 * @param ot The type of operand
 * @param op Pointer to the operation that contains this operand
 * @param vars The list of variables to search for variable pointers
 */
void extractOperandInformation(string unit, OperandType ot, Operation* op, vector<Variable>* vars) {
    size_t startBracket = unit.find("[");
    size_t endBracket = unit.find("]");
    
    // Check that closing brackets are opened beforehand
    if (endBracket != string::npos && startBracket == string::npos) throw runtime_error(ERROR_MISSING_OPEN_BRACKET + unit);
    
    // Check if the operator is a scalar by checking the first character
    if (isdigit(unit[0])) {
        // Extract the operand
        op->isOperandVar[ot] = false;
        op->operands[ot] = (uintptr_t) stoul(unit);
        
        // If the operator is a number, it cannot be indexed with brankets
        if (startBracket != string::npos) throw runtime_error(ERROR_INDEXING_SCALAR + unit);

        // Also init these so that the debug output does not look outrageous
        op->isIndexVar[ot] = false;
        op->indexes[ot] = (uintptr_t) 0;
    } else {
        // The operator is a variable
        // Save a pointer to the variable
        op->isOperandVar[ot] = true;
        op->operands[ot] = (uintptr_t) getVariableByName(vars, unit.substr(0, startBracket));

        if (startBracket == string::npos) {
            // If it is not indexed, add a ficticious 0 index
            op->isIndexVar[ot] = false;
            op->indexes[ot] = (uintptr_t) 0;
        } else {
            // If it is indexed
            // Fetch the string between [] and validate that it contains something
            if (endBracket == string::npos) throw runtime_error(ERROR_MISSING_CLOSE_BRACKET + unit);
            string index = unit.substr(startBracket + 1, endBracket - startBracket - 1);
            if (index.empty()) throw runtime_error(ERROR_MISSING_INDEX + unit);

            // Extract the index
            if (isdigit(index[0])) {
                // If the first index starts with a number, then the index is a number
                op->isIndexVar[ot] = false;
                op->indexes[ot] = (uintptr_t) stoul(index);
            } else {
                // If not, the index is a variable
                op->isIndexVar[ot] = true;
                op->indexes[ot] = (uintptr_t) getVariableByName(vars, index);
            }
        }
    }

    if (debug) printf("Debug:   Extracted %s, operand=0x%lu, isOperandVar=%d, index=0x%lu, isIndexVar=%d\n", OperandTypeToString(ot).c_str(), 
                        (unsigned long) op->operands[ot], op->isOperandVar[ot], (unsigned long) op->indexes[ot], op->isIndexVar[ot]);
}

/**
 * Processes an operation and stores it at a given index in the ops structure.
 * 
 * @param unit The whole string of the operation to process. 
 * @param ops Pointer to a vector in which operations will be stored.
 * @param vars Pointer to a vector with all the variables to process.
 * @param index The index in which to insert the operation. The caller is responsible of incrementing it afterwards
 */
void processOperation(string unit, vector<Operation>* ops, vector<Variable>* vars, int index) {
    Operation newOp;
    string dest, op1, op2;     // The extracted operand text
    size_t pos = string::npos; // The position that caused a match

    // If it starts with a datatype string, remove it
    for (int d = TYPE_CHAR; d < TYPE_COUNT; d++) {
        string pattern = DataTypeToString((DataType) d) + " "; 
        if (unit.rfind(pattern, 0) == 0) {
            unit.erase(0, pattern.length());
            break;
        }
    }

    // Remove all spaces and semicolons
    unit.erase(remove(unit.begin(), unit.end(), ' '), unit.end());
    unit.erase(remove(unit.begin(), unit.end(), ';'), unit.end());

    // If a datatype precedes the variable name, remove it
    for (int i = 0; i < TYPE_COUNT; i++) {
        string type = DataTypeToString((DataType) i);
        regex pattern("^" + type + " ");

        if (regex_search(unit, pattern)) {
            unit.erase(0, type.length() + 1);
        }
    }

    // Detect the type of operation
    if ((hasString("unit", "+") && hasString("unit", "=")) || hasString(unit, "++")) {             // Additions
        newOp.opType = OP_ADD;
    } else if ((hasString("unit", "-") && hasString("unit", "=")) || hasString(unit, "--")) {             // Additions
        newOp.opType = OP_SUB;
    } else if (hasString("unit", "*") && hasString("unit", "=")) {      // Multiplications
        newOp.opType = OP_MUL;
    } else if (hasString("unit", "/") && hasString("unit", "=")) {      // Divisions
        newOp.opType = OP_DIV;
    } else if (hasString("unit", "=")) {                           // Assignments
        newOp.opType = OP_EQUAL;
    } else if (debug) printf("Debug: Warning, unrecognized operation: %s\n", unit.c_str());

    if (debug) printf("Debug: Processing %s, %s\n", OperationTypeToString(newOp.opType).c_str(), unit.c_str());

    // Extract the operands
    // Operations can have 3 different statement types:
    // var[i] += var2[i]
    // var[i] = var1[i] + var2[i]   
    // var[i] = var2[i]             
    // Additionally, these can be indexed by another var, a scalar or by nothing

    // Iterate and try all the statements until one is found. 
    // This for loop works great if these are all the statements that we want to parse, but might be a bit limiting in the future
    for (int s = 0; s < STATEMENT_COUNT; s++) {
        pos = unit.find(StatementOperatorToString(newOp.opType, (StatementType) s));

        if (pos != string::npos) {
            if (s == STATEMENT_COMPOUND_ASSIGNMENT) {
                // Case 1. If the unit contains the corresponding compound assignment string (*=, for instance), we know that it is a compound statement
                // and we can extract the operands easily 
                dest = unit.substr(0, pos);     // The dest is at the right of the operator.
                op1 = unit.substr(0, pos);      // Since this is compound, one of the operands is the destination.
                op2 = unit.substr(pos + StatementOperatorToString(newOp.opType, STATEMENT_COMPOUND_ASSIGNMENT).length());
            } else if (s == STATEMENT_ASSIGMENT_AFTER_OPERATION) {
                // Case 2. There are two separate operands that have to be processed
                // Get the position of the equals
                size_t equalsPos = unit.find("=");

                dest = unit.substr(0, equalsPos);
                op1 = unit.substr(equalsPos + 1, equalsPos - pos - 1);
                op2 = unit.substr(pos + StatementOperatorToString(newOp.opType, STATEMENT_ASSIGMENT_AFTER_OPERATION).length());
            } else if (s == STATEMENT_ASSIGNMENT) {
                // Case 3. There is just a destination and an operator
                dest = unit.substr(0, pos);
                op1 = unit.substr(pos + StatementOperatorToString(newOp.opType, STATEMENT_ASSIGNMENT).length());
            }

            // Regardless of the statement, a match has been made, stop looking
            break;
        }
    }

    // An exception to process ++ and -- because the above does not :P
    if (hasString(unit, "++") || hasString(unit, "--")) {
        pos = unit.find("++");
        if (pos == string::npos) pos = unit.find("--");

        dest = unit.substr(0, pos);     // The dest is at the right of the operator.
        op1 = unit.substr(0, pos);      // Since this is compound, one of the operands is the destination.
        op2 = "1"; 
    }

    // Extract the information of each operand
    extractOperandInformation(dest, OPR_DESTINATION, &newOp, vars);
    extractOperandInformation(op1, OPR_OP1, &newOp, vars);
    if (!op2.empty()) extractOperandInformation(op2, OPR_OP2, &newOp, vars);

    // Insert the operation into the list at the given position
    ops->insert(ops->begin() + index, newOp);
}

/**
 * Processes a conditional and stores it at a given index in the ops structure.
 * Branch destinations point to index 0 by default. It is up to the caller to set the branch destination.
 * 
 * @param unit 
 * @param ops 
 * @param vars 
 * @param index 
 */
void processConditional(string unit, vector<Operation>* ops, vector<Variable>* vars, int index) {
    Operation newOp;
    string op1, op2;                                // The extracted operand text
    size_t opPosition;
    newOp.operands[OPR_DESTINATION] = 0;
    newOp.isIndexVar[OPR_DESTINATION] = false;      // Dest always point to a given position

    // Remove all spaces and semicolons
    unit.erase(remove(unit.begin(), unit.end(), ' '), unit.end());
    unit.erase(remove(unit.begin(), unit.end(), ';'), unit.end());

    // Detect the type of branch
    // Skip B_AL by starting at 1
    for (int b = 1; b < B_COUNT; b++) {
        if (hasString(unit, BranchTypeTypeToOperator((BranchType) b))) {
            newOp.bType = (BranchType) b;
            break;
        }
    }

    // Extract the operand strings
    opPosition = unit.find(BranchTypeTypeToOperator(newOp.bType));
    op1 = unit.substr(0, opPosition);
    op2 = unit.substr(opPosition + BranchTypeTypeToOperator(newOp.bType).length());

    // Process the operands
    extractOperandInformation(op1, OPR_OP1, &newOp, vars);
    extractOperandInformation(op2, OPR_OP2, &newOp, vars);

    // Insert the operation into the list at the given position
    ops->insert(ops->begin() + index, newOp);
}

/**
 * Processes a for loop and recursively processes every other instruction inside of it.
 * 
 * @param unit The whole for with all it's content. Content after the for should be cropped and handled by the caller 
 * @param ops Pointer to a vector in which operations will be stored.
 * @param vars Pointer to a vector with all the variables to process.
 * @param index The index in which to insert the operation. The caller is responsible of incrementing it afterwards
 */
void processForLoop(string unit, vector<Operation>* ops, vector<Variable>* vars, int index) {
    // For loops are decomposed into the following:
    // for (int i = 0; i < 10; i++)
    // 1. The iterator gets initialized (int i = 0). This happens only once
    // 2. The condition is checked and if it does not match, branch to 6 (i < 10)
    // 3. The code inside the for loop is run.
    // 4. The iterator is incremented. (i++);
    // 5. Branch to 2 always.
    // 6. More code or, perhaps, the end of the program.
    size_t semicolon, startBracket, endBracket;
    int startOfFor;
    Operation* forCondition;        // Stored to set the branch destination
    Operation forEnd;               // A branch to the for condition at the end of the for
    string forContent;

    // Fetch the for content
    startBracket = unit.find("{");
    endBracket = unit.find("}");
    if (startBracket == string::npos || endBracket == string::npos) throw runtime_error(ERROR_FOR_BRACKETS + unit);
    forContent = unit.substr(startBracket, endBracket - startBracket - 1);
    unit.erase(startBracket, endBracket - startBracket);

    // Remove the for start, at this point we already know this is a for
    if (unit.find("for(") != string::npos) unit.erase(0, 4);
    else if (unit.find("for (") != string::npos) unit.erase(0, 5);

    // Find the first semicolon
    semicolon = unit.find(";");
    if (semicolon == string::npos) throw runtime_error(ERROR_MALFORMED_FOR + unit);

    // Process the iterator initialization (step 1)
    processOperation(unit.substr(0, semicolon), ops, vars, index);
    unit.erase(0, semicolon);
    index++;
    startOfFor = index;

    // Find the second semicolon
    semicolon = unit.find(";");
    if (semicolon == string::npos) throw runtime_error(ERROR_MALFORMED_FOR + unit);

    // Process the condition (step 2)
    // TODO implement branch handling in process operation !!!! 
    processOperation(unit.substr(0, semicolon), ops, vars, index);
    forCondition = &ops->at(index);
    index++;

    // The code inside of the loop is processed recursively (step 3)
    processCode(forContent, ops, vars, index);
    index = ops->size();                    // An undefined number of elements has been added, update the index

    // The iterator gets incremented (step 4)
    semicolon = unit.find(";");
    if (semicolon == string::npos) throw runtime_error(ERROR_MALFORMED_FOR + unit);
    processOperation(unit.substr(0, semicolon), ops, vars, index);
    index++;

    // Add the branch to step 2 by hand (step 5)
    forEnd.bType = B_AL;
    forEnd.opType = OP_BRANCH;
    forEnd.operands[OPR_DESTINATION] = startOfFor;
    ops->insert(ops->begin() + index, forEnd);
    index++;

    // Set the for to branch to outside the for if the condition in step 2 is not met
    forCondition->operands[OPR_DESTINATION] = index; 
}

/**
 * Extracts all the operations from the code for interpreting them later.
 * 
 * @param code The preprocessed code. 
 * @param ops Pointer to a vector in which operations will be stored.
 * @param vars Pointer to a vector with all the variables to process.
 * @param index Index to start inserting the code instuctions.
 */
void processCode(string code, vector<Operation>* ops, vector<Variable>* vars, int index) {
    // Iterate over all the code lines
    while (!code.empty()) {
        // Extract the next piece of code to process
        string unit = extractNextUnit(&code);

        // Filter irrelevant lines
        // All memory operations that are worth simulating have an equals, comparison (in case of the for) or arithmetic operators, if it does not have it, skip it
        if (hasString(unit, "=") || hasString(unit, ">") || hasString(unit, "<") || 
            hasString(unit, "+") || hasString(unit, "-") || hasString(unit, "*") || hasString(unit, "/")) {

            // If the statement is a for, apply special processing to it 
            if (unit.substr(0, 3) == "for") {
                processForLoop(unit, ops, vars, index);
            } else {
                processOperation(unit, ops, vars, index);
            }

            index++;
        } else if (debug) printf("Debug: Skipping \t%s\n", unit.c_str());
    }
}