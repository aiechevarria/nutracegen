#pragma once

#include "Misc.h"
#include "Semantics.h"
#include "Parser.h"

// In order to avoid infinite loops blocking the program, a maximum interpretation time is established.
// If the interpretation takes longer, we assume that the code contains an infinite loop and terminate the interpretation prematurely to avod locks
// This can be incremented if the code is too long
#define INTERPRETER_TIMEOUT_MS 1000

/*
 * Takes code operations, processes them and puts results in the trace.
 */

// Groups the memory, trace and settings for ease of use
typedef struct {
    unordered_map<uint64_t, uint64_t>* memMap;
    InterpreterSettings settings;
    string* trace;
} TraceData;

void interpretCode(string code, string& trace, vector<Operation>& ops, vector<Variable>& variables, InterpreterSettings settings);