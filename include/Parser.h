#pragma once

#include "Misc.h"
#include "Semantics.h"

#define BUFFER_SIZE 4096

std::string readFileToString(char inputPath[MAX_PATH_LENGTH]);
bool writeStringToFile(char* filePath, std::string& content);
bool parseVariables(std::string text, std::vector<Variable>* variables);
void preProcessCode(std::string* code);
void processCode(std::string* code, std::vector<Operation>* ops, std::vector<Variable>* vars, int index);