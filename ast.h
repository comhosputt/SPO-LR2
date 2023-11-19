#pragma once

#include "util.h"


typedef struct ASTNode {
	char* type;
	struct ASTNode* left;
	struct ASTNode* right;
	char* value;
	int id;
} ASTNode;

void printAST();

ASTNode* createNode(char* type, ASTNode* left, ASTNode* right, char* value);



typedef struct GraphNode {
	int traverseTag;
    struct GraphNode* usl;
    struct GraphNode* busl;
	char* data;
} GraphNode;
