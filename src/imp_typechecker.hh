#ifndef IMP_TYPECHECKER
#define IMP_TYPECHECKER

#include <unordered_map>

#include "environment.hh"
#include "imp.hh"
#include "type_visitor.hh"

using namespace std;

class ImpTypeChecker : public TypeVisitor {
   public:
    ImpTypeChecker();

   private:
    int max_local_mem, max_mem;
    Environment<ImpType> env;

   public:
    void typecheck(Program*);
    void visit(Program*);
    void visit(Body*);
    void visit(VarDecList*);
    void visit(VarDec*);
    void visit(StatementList*);
    void visit(AssignStatement*);
    void visit(PrintStatement*);
    void visit(IfStatement*);
    void visit(WhileStatement*);

    int get_max_mem() { return max_mem; }

    ImpType visit(BinaryExp* e);
    ImpType visit(NumberExp* e);
    ImpType visit(IdExp* e);
    ImpType visit(ParenthExp* e);
    ImpType visit(CondExp* e);
};

#endif
