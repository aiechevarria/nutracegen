#pragma once

#include "Misc.h"
#include "Semantics.h"
#include "Parser.h"

/*
 * Takes code operations, processes them and puts results in the trace.
 */

void interpretCode(std::string code, std::string* trace, std::vector<Variable>* variables, GeneratorSettings* settings);

// Compound assignment operators
void addOperands(std::string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 += v2 
void subOperands(std::string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 -= v2
void mulOperands(std::string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 *= v2
void divOperands(std::string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);         // v1 /= v1

// Equality
void equalOperands(std::string* trace, GeneratorSettings settings, Variable v1, Variable v2, long scalar, bool useVar = true);       // v1 = v2