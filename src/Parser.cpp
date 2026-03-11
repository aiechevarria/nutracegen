#include "Parser.h"

/**
 * Get all the lines that match a given regex
 * 
 * @param text 
 * @param pattern 
 * @return std::vector<std::string> 
 */
std::vector<std::string> getMatchingLines(const std::string& text, const std::string& pattern) {
    // Parse the regular expression and create a container for all the matches
    std::regex re(pattern);
    std::vector<std::string> matches;

    std::istringstream stream(text);
    std::string line;

    // Iterate over all the liens in the stream
    while (std::getline(stream, line)) {
        // If the line matches the regex, store it in the matches bin
        if (std::regex_search(line, re)) matches.push_back(line);
    }

    return matches;
}

/**
 * Extracts the content of the input file to a string.
 * 
 * @param inputPath The path to the file
 * @return std::string The parsed file
 */
std::string readFileToString(char inputPath[MAX_PATH_LENGTH]) {
    char buffer[BUFFER_SIZE];
    std::string result;
    size_t bytesRead;

    // Open the file
    FILE* file = fopen(inputPath, "r");
    
    // Validate the file has been opened
    if (!file) {
        printf(ERROR_FILE_OPEN);
        return result;
    }

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        result.append(buffer, bytesRead);
    }

    // Validate no error has happened
    if (ferror(file)) {
        printf(ERROR_FILE_READ);
        result.clear();
        fclose(file);
        return result;
    }

    // Validate the file is not empty
    if (result.empty()) printf(ERROR_FILE_EMPTY);

    fclose(file);
    return result;
}

/**
 * Stores the string in a filepath.
 * 
 * @param filePath The path to the file.
 * @param content The content to store.
 * @return true if ok, false if error
 */
bool writeStringToFile(char* filePath, std::string& content) {
    FILE* file = fopen(filePath, "w");

    if (file == NULL) return false;

    fwrite(content.c_str(), 1, content.size(), file);

    fclose(file);
    return true;
}

/**
 * Extracts the variables from the pseudocode.
 * @param text The text to extract the variables from
 * @param variables A vector that stores all variables in the order they have been parsed
 * @return True if the process suceeded, false if it failed.
 */
bool parseVariables(std::string text, std::vector<Variable>* variables) {
    // For a given datatype, check if the text has any occurences
    // For instance, get all the lines that have "int "
    // Once all the matches have been fetched, crop the datatype and the space until a ; [ = or space is met. That is the variable name

    // Iterate over each datatype 
    for (int d = TYPE_CHAR; d < TYPE_COUNT; d++) {
        DataType type = static_cast<DataType>(d);
        std::string typeStr = DataTypeToString(type);

        // Match for datatype + space + variable name (until space, ;, =, or [)
        std::string pattern = typeStr + R"(\s+([a-zA-Z_]\w*))";

        // Get all the matching lines for this regex
        std::vector<std::string> lines = getMatchingLines(text, pattern);

        // Iterate over every line
        for (const auto& line : lines) {
            std::smatch match;

            // Split the line into groups
            if (std::regex_search(line, match, std::regex(pattern))) {
                if (match.size() >= 2) {
                    // Capture the name and store the variable
                    std::string varName = match[1];
                    variables->push_back(Variable{0, varName, (DataType) d});

                    if(debug) printf("Debug: Found variable with name=%s, type=%s\n", varName.c_str(), DataTypeToString(type).c_str());
                }
            }
        }
    }

    if (variables->empty()) {
        printf(ERROR_PARSE_NOVAR);
        return false;
    } 

    return true;
}

/**
 * Pre processes the code. Removes tabs, spaces and adapts for loops.
 * 
 * @param code Pointer to the code.
 */
void preProcessCode(std::string* code) {
    // Remove tabs and newlines
    code->erase(std::remove(code->begin(), code->end(), '\t'), code->end());
    code->erase(std::remove(code->begin(), code->end(), '\n'), code->end());

    // Collapse multiple spaces into one
    *code = std::regex_replace(*code, std::regex("\\s+"), " ");

    // 3. Add brackets to bracketless for-loops
    // This regex looks for: for(...) [not a {] [anything up to a ;]
    // Group 1: The for header: for(...)
    // Group 2: The single statement: ...;
    std::regex bracketlessFor(R"((for\s*\([^)]+\))\s*([^{;]+;))");
    
    // Replace "for(...) statement;" with "for(...) { statement; }"
    *code = std::regex_replace(*code, bracketlessFor, "$1 { $2 }");
}

/**
 * Fetches the next logical unit (statement or for) from the code.
 *
 * @param code A pointer to the code. Elements will get removed from it.
 * @return The extracted piece of code.
 */
std::string extractNextUnit(std::string* code) {
    // Check that some code is left to process first
    if (code->empty()) return "";

    std::string unit = "";
    
    // Check if it starts with for
    if (code->substr(0, 3) == "for") {
        // Find the first bracket
        size_t openBracePos = code->find('{');
        // If there are no brackets, print an error and return nothing. This case should have been handled by the preprocessor.
        if (openBracePos == std::string::npos) {
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
        if (semiColonPos != std::string::npos) {
            unit = code->substr(0, semiColonPos + 1);
            code->erase(0, semiColonPos + 1);
        }
    }

    return unit;
}

/**
 * Extracts the actual operands of the operation.
 * 
 * @param unit The whole string of the operation to process. 
 * @param op Pointer to a the operation that will store the result. It has to be prepopulated with the type of operation
 * @param vars Pointer to a vector with all the variables to process.
 */
void extractOperands(std::string unit, Operation* op, std::vector<Variable>* vars) {
    std::string dest, op1, op2;     // The extracted operand text
    size_t pos = std::string::npos; // The position that caused a match
    Variable* destination;

    // Operations can have 3 different statement types:
    // var[i] += var2[i]
    // var[i] = var1[i] + var2[i]   
    // var[i] = var2[i]             
    // Additionally, these can be indexed by another var, a scalar or by nothing

    // Iterate and try all the statements until one is found. 
    // This for loop works great if these are all the statements that we want to parse, but might be a bit limiting in the future
    for (int s = 0; s < STATEMENT_COUNT; s++) {
        pos = unit.find(StatementOperatorToString(op->opType, (StatementType) s));

        if (pos != std::string::npos) {
            if (s == STATEMENT_COMPOUND_ASSIGNMENT) {
                // Case 1. If the unit contains the corresponding compound assignment string (*=, for instance), we know that it is a compound statement
                // and we can extract the operands easily 
                dest = unit.substr(0, pos);     // The dest is at the right of the operator.
                op1 = unit.substr(0, pos);      // Since this is compound, one of the operands is the destination.
                op2 = unit.substr(pos + StatementOperatorToString(op->opType, STATEMENT_COMPOUND_ASSIGNMENT).length());
            } else if (s == STATEMENT_ASSIGMENT_AFTER_OPERATION) {
                // Case 2. There are two separate operands that have to be processed
                // Get the position of the equals
                size_t equalsPos = unit.find("=");

                dest = unit.substr(0, equalsPos);
                op1 = unit.substr(equalsPos + 1, pos);
                op2 = unit.substr(pos + StatementOperatorToString(op->opType, STATEMENT_ASSIGMENT_AFTER_OPERATION).length());
            } else if (s == STATEMENT_ASSIGNMENT) {
                // Case 3. There is just a destination and an operator
                dest = unit.substr(0, pos);
                op1 = unit.substr(pos + StatementOperatorToString(op->opType, STATEMENT_ASSIGNMENT).length());
            }

            // Regardless of the statement, a match has been made, stop looking
            break;
        }
    }


    // Once the operand strings have been extracted, parse them and store them in the variable. 
    // For the destination and first operand, get a pointer to the variable and the index (if any)

    // For the second operand, do the same but check if it exists first.


}

/**
 * Processes an operation and stores it at a given index in the ops structure.
 * 
 * @param unit The whole string of the operation to process. 
 * @param ops Pointer to a vector in which operations will be stored.
 * @param vars Pointer to a vector with all the variables to process.
 * @param index The index in which to insert the operation.
 */
void processOperation(std::string unit, std::vector<Operation>* ops, std::vector<Variable>* vars, int index) {
    Operation newOp;

    // If a datatype precedes the variable name, remove it
    for (int i = 0; i < TYPE_COUNT; i++) {
        std::string type = DataTypeToString((DataType) i);
        std::regex pattern("^" + type + " ");

        if (std::regex_search(unit, pattern)) {
            unit.erase(0, type.length() + 1);
        }
    }

    // Detect the type of operation
    if (unit.find("+") && unit.find("=")) {             // Additions
        newOp.opType = OP_ADD;
    } else if (unit.find("-") && unit.find("=")) {      // Substractions
        newOp.opType = OP_SUB;
    } else if (unit.find("*") && unit.find("=")) {      // Multiplications
        newOp.opType = OP_MUL;
    } else if (unit.find("/") && unit.find("=")) {      // Divisions
        newOp.opType = OP_DIV;
    } else if (unit.find("=")) {                           // Assignments
        newOp.opType = OP_EQUAL;
    } else if (debug) printf("Debug: Warning, unrecognized operation: %s\n", unit.c_str());

    // Extract the operands

    // Insert into the queue
}

/**
 * Processes a for loop and recursively processes every other instruction inside of it.
 */
void processForLoop(std::string code, std::vector<Operation>* ops, int index) {
    // // Regex breakdown:
    // // for\s*\(\s*(\w+)\s*=\s*(\d+)\s*;      matches 'for ( i = 0 ;'
    // // \s*\1\s*<\s*(\d+)\s*;                 matches 'i < 10 ;'
    // // \s*(\w+\+\+| \+\+\w+)\s*\)            matches 'i++)'
    // std::regex forPattern(R"(for\s*\(\s*(\w+)\s*=\s*(\d+)\s*;\s*\1\s*<\s*(\d+)\s*;\s*(\w+\+\+|\+\+\w+)\s*\))");
    // std::smatch matches;

    // if (std::regex_search(code, matches, forPattern)) {
    //     ForLoop details;
    //     details.iteratorName  = matches[1].str();
    //     details.startValue    = matches[2].str();
    //     details.limitValue    = matches[3].str();
    //     details.step          = matches[4].str();

    //     std::cout << "--- Loop Extraction Found ---\n";
    //     std::cout << "Iterator: " << details.iteratorName << "\n";
    //     std::cout << "Start:    " << details.startValue << "\n";
    //     std::cout << "Limit:    " << details.limitValue << "\n";
    //     std::cout << "Step:     " << details.step << "\n";
    // } else {
    //     std::cout << "No standard for-loop pattern matched." << std::endl;
    // }
}

/**
 * Extracts all the operations from the code for interpreting them later.
 * 
 * @param code The preprocessed code. 
 * @param ops Pointer to a vector in which operations will be stored.
 * @param vars Pointer to a vector with all the variables to process.
 * @param index Index to start inserting the code instuctions.
 */
void processCode(std::string* code, std::vector<Operation>* ops, std::vector<Variable>* vars, int index) {
    // Iterate over all the code lines
    while (!code.empty()) {
        // Extract the next piece of code to process
        std::string unit = extractNextUnit(&code);

        // All memory operations that are worth simulating have an equals, if it does not have it, skip it
        if (unit.find("=") != std::string::npos) {
            if (debug) printf("Debug: Processing %s\n", unit.c_str());

            // If the statement is not a for, apply special processing to it 
            if (unit.substr(0, 3) != "for") {
            }

        } else if (debug) printf("Debug: Skipping %s\n", unit.c_str());
    }


}

// void extractForLoopDetails(const std::string& code, const std::string<> std::vector<Variable>* variables) {


// }