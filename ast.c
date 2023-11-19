#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include "ast.h" 
#include <stdint.h> 
#define _CRT_SECURE_NO_WARNINGS 

ASTNode** allNodes;
uint64_t allNodesCount;


struct ControlFlowEdge* controlFlowEdges = NULL;
int edgeCount = 0;


ASTNode* createNode(char* type, ASTNode* left, ASTNode* right, char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->left = left;
    node->right = right;
    node->id = allNodesCount;
    char* buf = malloc((strlen(value) + 1) * sizeof(char));
    strcpy_s(buf, sizeof(buf), value);
    node->value = buf;
    allNodes[allNodesCount] = node;
    allNodesCount++;
    printf("created #%d %s (%d, %d) %s\n", node->id, type, (left ? left->id : -1), (right ? right->id : -1), value);
    return node;
}

GraphNode* createCfgNode(char* data) {
    GraphNode* node = malloc(sizeof(GraphNode));
    node->usl = NULL;
    node->busl= NULL;
    node->data = data;
    node->traverseTag = 0;
    return node;
}



void printNodeId(FILE* file, ASTNode* node) {
    fprintf(file, "<Node Id=\"%d\" Label=\"%s", node->id, node->type);
    if (strlen(node->value) > 0) {
        fprintf(file, ", Value: %s", node->value);
    }
    fprintf(file, "\"/>\n");
}

void printAST() {
    FILE* file = fopen("tree.dgml", "w+");
    fprintf(file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(file, "<DirectedGraph xmlns=\"http://schemas.microsoft.com/vs/2009/dgml\">\n");
    fprintf(file, "<Nodes>\n");
    for (int i = 0; i < allNodesCount; ++i) {
        //allNodes[i]->id = i;
        printNodeId(file, allNodes[i]);
    }
    fprintf(file, "</Nodes>\n");
    fprintf(file, "<Links>\n");
    // Print all links after all nodes are printed
    for (int i = 0; i < allNodesCount; ++i) {
        if (allNodes[i]->left) {
            fprintf(file, "<Link Source=\"%d\" Target=\"%d\"/>\n", allNodes[i]->id, allNodes[i]->left->id);
        }
        if (allNodes[i]->right) {
            fprintf(file, "<Link Source=\"%d\" Target=\"%d\"/>\n", allNodes[i]->id, allNodes[i]->right->id);
        }
    }
    fprintf(file, "</Links>\n");
    fprintf(file, "</DirectedGraph>");
    fclose(file);

   
}

GraphNode* drawBranch(GraphNode* curr, ASTNode* ast)
{
    printf("drawBranch for %s\n", ast->type);
    if (strcmp("if_statement", ast->type) == 0) {
        GraphNode* a = createCfgNode(ast->left->type);
        GraphNode* b = createCfgNode(NULL);
        GraphNode* c = createCfgNode(NULL);
        curr->busl = a;
        a->busl = b;
        a->usl = c;

        GraphNode* d =  drawBranch(b,ast->right->left);
        GraphNode* e = drawBranch(c, ast->right->right);
        GraphNode* f = createCfgNode(NULL);
        d->busl = f;
        e->busl = f;
        return f;

    }
    else if (strcmp("while", ast->type) == 0){
        GraphNode* a = createCfgNode(ast->left->type);
        GraphNode* b = createCfgNode(NULL);
        GraphNode* c = createCfgNode(NULL);
        curr->busl = a;
        a->usl = b;
        a->busl = c;

        GraphNode* d = drawBranch(b, ast->right);
        d->busl = a;
        return c;
    }
    else if (strcmp("expression_statement", ast->type) == 0) {
        GraphNode* a= createCfgNode(ast->left->type);
        curr->busl = a;
       
        return a;

    }
    else if (strcmp("compound_statement", ast->type) == 0) {
        GraphNode* a = drawBranch(curr, ast->left);
        return a;
    }
    else if (strcmp("block_item_list", ast->type) == 0) {
        GraphNode* a = drawBranch(curr, ast->left);
        GraphNode* b = drawBranch(a, ast->right);
        return b;
    }
    else {
        printf("unexpected control-flow statement %s\n", ast->type);
        return curr;
    }


}

void traverseForNode(FILE* f, int tag, GraphNode* node)
{
    if (node != NULL && node->traverseTag != tag) {
        node->traverseTag = tag;
        fprintf(f, "<Node Id=\"#%lx\" Label=\"#%lx&#10;%s\" />\n", (long)node, (long)node, node->data);
        traverseForNode(f, tag, node->busl);
        traverseForNode(f, tag, node->usl);
    }
}
void traverseForLinks(FILE* f, int tag, GraphNode* node)
{
    if (node != NULL && node->traverseTag != tag) {
        node->traverseTag = tag;
        if (node->busl != NULL) {
            fprintf(f, "<Link Source=\"#%lx\" Target=\"#%lx\"/>\n", (long)node, (long)node->busl);
            traverseForLinks(f, tag, node->busl);
        }
        if (node->usl != NULL) {
            fprintf(f, "<Link Source=\"#%lx\" Target=\"#%lx\"/>\n", (long)node, (long)node->usl);
            traverseForLinks(f, tag, node->usl);
        }
    }
}

void traverseForCfgDgml(GraphNode* node, char* fileName)
{
    FILE* file = fopen(fileName, "w+");
    fprintf(file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(file, "<DirectedGraph xmlns=\"http://schemas.microsoft.com/vs/2009/dgml\">\n");
    fprintf(file, "<Nodes>\n");
    traverseForNode(file, 3, node);
    fprintf(file, "</Nodes>\n");
    fprintf(file, "<Links>\n");
    traverseForLinks(file, 4, node);
    fprintf(file, "</Links>\n");
    fprintf(file, "</DirectedGraph>");
    fclose(file);

}

void traverseAST(ASTNode* node) {
    
    if (node) {
        
        printf("Type: %s\n", node->type);
        printf("Value: %s\n", node->value);
        printf("Id: %d\n", node->id);


        if (strcmp("body", node->type) == 0) {
           
            GraphNode* cfgStart = createCfgNode(node->right->type);
            printf("1: %s\n", node->right->type);
            drawBranch(cfgStart, node->right);
            traverseForCfgDgml(cfgStart, "control_flow_graph.dgml");
        }
        else {
            traverseAST(node->left);
            traverseAST(node->right);
        }
    }
}




