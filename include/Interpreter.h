#pragma once

#include "Misc.h"
#include "Semantics.h"
#include "Parser.h"

/*
 * Takes code operations, processes them and puts results in the trace.
 */

void interpretCode(string code, string* trace, vector<Variable>* variables, GeneratorSettings* settings);

// Compound assignment operators
void addOperands(string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 += v2 
void subOperands(string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 -= v2
void mulOperands(string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 *= v2
void divOperands(string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 /= v1

// Equality
void equalOperands(string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);       // v1 = v2