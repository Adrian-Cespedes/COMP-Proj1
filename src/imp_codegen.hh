#ifndef IMP_CODEGEN
#define IMP_CODEGEN

#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "environment.hh"
#include "imp.hh"
#include "imp_visitor.hh"

class ImpCodeGen : public ImpVisitor {
   public:
    void codegen(Program*, string outfname, int _mem_size);
    void visit(Program*);
    void visit(Body*);
    void visit(VarDecList*);
    void visit(VarDec*);
    void visit(StatementList*);
    void visit(AssignStatement*);
    void visit(PrintStatement*);
    void visit(IfStatement*);
    void visit(WhileStatement*);

    int visit(BinaryExp* e);
    int visit(NumberExp* e);
    int visit(IdExp* e);
    int visit(ParenthExp* e);
    int visit(CondExp* e);

   private:
    std::ostringstream code;
    string nolabel;
    int mem_size;
    int current_label;
    Environment<int> direcciones;
    int siguiente_direccion;
    void codegen(string label, string instr);
    void codegen(string label, string instr, int arg);
    void codegen(string label, string instr, string jmplabel);
    string next_label();
};

#endif
