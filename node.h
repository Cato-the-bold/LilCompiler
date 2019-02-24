#include <iostream>
#include <vector>
//#include <llvm/IR/Value.h>

using namespace std;

class CodeGenContext;
class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VarDeclList;

class Node {
public:
	virtual ~Node() {}
//	virtual llvm::Value* codeGen(CodeGenContext& context) { return NULL; }
};

class Expression : public Node {
};

class Statement : public Node {
};

class Integer : public Expression {
public:
	long long value;
	Integer(long long value) : value(value) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Double : public Expression {
public:
	double value;
	Double(double value) : value(value) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Identifier : public Expression {
public:
    std::string* name;
	Identifier(std::string* name) : name(name) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class MethodCall : public Expression {
public:
	const Identifier& id;
	ExpressionList arguments;
	MethodCall(const Identifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	MethodCall(const Identifier& id) : id(id) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class BinaryOperator : public Expression {
public:
	int op;
	Expression& lhs;
	Expression& rhs;
	BinaryOperator(Expression& lhs, int op, Expression& rhs) :
		lhs(lhs), rhs(rhs), op(op) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Assignment : public Expression {
public:
	Identifier& lhs;
	Expression& rhs;
	Assignment(Identifier& lhs, Expression& rhs) :
		lhs(lhs), rhs(rhs) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Block : public Statement {
public:
	StatementList statements;
	Block() { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class ExpressionStatement : public Statement {
public:
	Expression& expression;
	ExpressionStatement(Expression& expression) :
		expression(expression) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class ReturnStatement : public Statement {
public:
	Expression& expression;
	ReturnStatement(Expression& expression) :
		expression(expression) { }
//	virtual llvm::Value* codeGen(CodeGenContext& context);
};

//class ExternDeclaration : public Statement {
//public:
//    const Identifier& type;
//    const Identifier& id;
//    VariableList arguments;
//    ExternDeclaration(const Identifier& type, const Identifier& id,
//                      const VariableList& arguments) :
//            type(type), id(id), arguments(arguments) {}
//    virtual llvm::Value* codeGen(CodeGenContext& context);
//};

class VariableDeclaration : public Statement {
public:
    Identifier* type;
    std::vector<Identifier*>* var_list;
    VariableDeclaration(Identifier* type, std::vector<Identifier*>* var_list) :
            type(type), var_list(var_list) {  }
//    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Function : public Statement {
public:
    Identifier* type;
    Identifier* id;
    Block* arguments;
    Block* block;
	Function( Identifier* type, Identifier* id,
                        Block* arguments, Block* block) :
            type(type), id(id), arguments(arguments), block(block) { }
//    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Context {
public:
    std::vector<string*>* variables;

};