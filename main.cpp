#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstring>

#include "utils/hash.h"
#include "symtab.h"
#include "expr.h"
#include "stmt.h"

using namespace std;

extern int yyparse();

extern FILE *yyin;

char **inputs = NULL;

extern symbol *cross_link;

//vector<string*> symbols(Block *stmt){
//    vector<string*> answer;
//    if(stmt) {
//        for (auto stmt : stmt->statements) {
//            if (stmt && dynamic_cast<VariableDeclaration *>(stmt) != nullptr) {
//                VariableDeclaration *decl = dynamic_cast<VariableDeclaration *>(stmt);
//                vector < Identifier * > *var_list = decl->var_list;
//                for (auto id : *var_list) {
//                    answer.push_back(id->name);
//                }
//            }
//        }
//    }
//    return answer;
//}
//
//void output(vector<string*> l){
//    for (auto s : l) {
//        cout << *s << "," ;
//    }
//}
//
//void print_func(Function* decl){
//    string *name = decl->id->name;
//    cout << "\nFunction " << *name << endl;
//
//    cout << "\tParameters: ";
//    vector < string * > paras = symbols(decl->arguments);
//    output(paras);
//    cout << endl;
//
//    cout << "\tLocal vars: ";
//    vector < string * > locals = symbols(decl->block);
//    output(locals);
//    cout << endl;
//}
//
//void print_info(Block *program){
//    vector<string*> globals = symbols(program);
//    cout <<  "Global vars: " ;
//    output(globals);
//    cout << endl;
//
//    if(program) {
//        for (auto stmt : program->statements) {
//            if (stmt && dynamic_cast<Function *>(stmt) != nullptr) {
//                Function *decl = dynamic_cast<Function *>(stmt);
//                print_func(decl);
//            }
//        }
//    }
//}

void init_environment() {
    Symbol_tab = maketab(257, (unsigned (*)(void *)) hash_pjw, (int (*)(void *, void *)) strcmp);
    Struct_tab = maketab(257, (unsigned (*)(void *)) hash_pjw, (int (*)(void *, void *)) strcmp);
    Class_tab = maketab(257, (unsigned (*)(void *)) hash_pjw, (int (*)(void *, void *)) strcmp);
}

int main(int argc, char **argv) {

    init_environment();

    int flag = 0;

    if (argc <= 2) {
        printf("Usage: compile -p file1 file2 ... \n");
        exit(0);
    }

    inputs = &argv[2];
    yyin = fopen(inputs[0], "r");

    if (strcmp(argv[1], "-t") == 0) {
        yyparse();
//        print_info(programBlock);

    } else if (strcmp(argv[1], "-l") == 0) {
        printf("Lexical Analyzer:\n");


    } else if (strcmp(argv[1], "-p") == 0) {

    } else if (strcmp(argv[1], "-i") == 0) {
        printf("Intermediate code generator :\n");
    } else if (strcmp(argv[1], "-c") == 0) {
        printf("Target code generator:\n");
    }


    return 0;
}