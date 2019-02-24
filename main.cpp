#include <iostream>
#include <vector>
#include <cstring>
#include <string>

#include "node.h"

using namespace std;

extern int yyparse();
extern FILE *yyin;

char **inputs = NULL;

Block* programBlock = new Block();

vector<string*> symbols(Block *stmt){
    vector<string*> answer;
    if(stmt) {
        for (auto stmt : stmt->statements) {
            if (stmt && dynamic_cast<VariableDeclaration *>(stmt) != nullptr) {
                VariableDeclaration *decl = dynamic_cast<VariableDeclaration *>(stmt);
                vector < Identifier * > *var_list = decl->var_list;
                for (auto id : *var_list) {
                    answer.push_back(id->name);
                }
            }
        }
    }
    return answer;
}

void output(vector<string*> l){
    for (auto s : l) {
        cout << *s << "," ;
    }
}

void print_func(Function* decl){
    string *name = decl->id->name;
    cout << "\nFunction " << *name << endl;

    cout << "\tParameters: ";
    vector < string * > paras = symbols(decl->arguments);
    output(paras);
    cout << endl;

    cout << "\tLocal vars: ";
    vector < string * > locals = symbols(decl->block);
    output(locals);
    cout << endl;
}

void print_info(Block *program){
    vector<string*> globals = symbols(program);
    cout <<  "Global vars: " ;
    output(globals);
    cout << endl;

    if(program) {
        for (auto stmt : program->statements) {
            if (stmt && dynamic_cast<Function *>(stmt) != nullptr) {
                Function *decl = dynamic_cast<Function *>(stmt);
                print_func(decl);
            }
        }
    }
}

int main(int argc, char **argv)
{

	int flag = 0;

	if(argc<=2){
        printf("Usage: compile -p file1 file2 ... \n");
        exit(0);
	}

    inputs = &argv[2];
    yyin = fopen(inputs[0], "r");

    if (strcmp(argv[1],"-p")==0) {
        printf("Parser:\n");

        yyparse();

        print_info(programBlock);

    } else if (strcmp(argv[1],"-l")==0) {
        printf("Lexical Analyzer:\n");



    } else if (strcmp(argv[1],"-t")==0) {
        printf("Type checker:\n");
    } else if (strcmp(argv[1],"-i")==0) {
        printf("Intermediate code generator :\n");
    } else if (strcmp(argv[1],"-c")==0) {
        printf("Target code generator:\n");
    }




	
	return 0;
}