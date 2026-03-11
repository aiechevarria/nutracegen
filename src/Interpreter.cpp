#include "Interpreter.h"
#include "Parser.h"
#include "Semantics.h"

/**
 * Parses the code and generates trace.
 * 
 * @param code The code. 
 * @param trace Pointer to the string that will store the trace
 * @param variables Pointer to the list of variables and their configurations
 * @param settings Pointer to the settings of the generator
 */
void interpretCode(std::string code, std::string* trace, std::vector<Variable>* variables, GeneratorSettings* settings) {
    // Clear the memory map
    std::unordered_map<unsigned long, unsigned long> memMap;    // Memory map to store the results of operations
    std::vector<Operation> opList;                              // The list of operations that will get interpreted

    // Print the starting comment to the trace
    trace->append("# Trace generated with ");
    trace->append(APP_NAME);
    trace->append(" ");
    trace->append(APP_VERSION);
    trace->append("\n");

    // Preprocess the code prior to extracting units
    preProcessCode(&code);

    if (debug) {
        printf("Debug: Pre-processed output:\n");
        printf("%s\n\n", code.c_str());
        fflush(stdout);
    }
    
    // Process the code and extract the operations
    processCode(&code, &opList, 0);
}
