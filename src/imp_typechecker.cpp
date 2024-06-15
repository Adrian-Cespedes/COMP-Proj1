#include "imp_typechecker.hh"

ImpTypeChecker::ImpTypeChecker() {
    max_local_mem = 0;
    max_mem = 0;
}

void ImpTypeChecker::typecheck(Program* p) {
    env.clear();
    p->accept(this);
    cout << "Max memory needed: " << max_mem << endl;
    return;
}

void ImpTypeChecker::visit(Program* p) {
    p->body->accept(this);
    return;
}

void ImpTypeChecker::visit(Body* b) {
    int temp = max_local_mem;
    env.add_level();
    b->var_decs->accept(this);
    b->slist->accept(this);
    env.remove_level();
    if (max_local_mem > max_mem)
        max_mem = max_local_mem;

    max_local_mem = temp;
    return;
}

void ImpTypeChecker::visit(VarDecList* decs) {
    list<VarDec*>::iterator it;
    for (it = decs->vdlist.begin(); it != decs->vdlist.end(); ++it) {
        (*it)->accept(this);
    }
    return;
}

// Solo se aceptan ints
void ImpTypeChecker::visit(VarDec* vd) {
    ImpType type = ImpValue::get_basic_type(vd->type);
    if (type == NOTYPE) {
        cout << "Error: unknown type " << vd->type << endl;
        exit(1);
    }

    list<string>::iterator it;
    for (it = vd->vars.begin(); it != vd->vars.end(); ++it) {
        max_local_mem++;
        env.add_var(*it, type);
    }
    return;
}

void ImpTypeChecker::visit(StatementList* s) {
    list<Stm*>::iterator it;
    for (it = s->slist.begin(); it != s->slist.end(); ++it) {
        (*it)->accept(this);
    }
    return;
}

// Se verifica que la variable este declarada y que el tipo sea el mismo
// de la expresion
void ImpTypeChecker::visit(AssignStatement* s) {
    ImpType type = s->rhs->accept(this);
    if (!env.check(s->id)) {
        cout << "Error: variable " << s->id << " not declared" << endl;
        exit(0);
    }

    ImpType vtype = env.lookup(s->id);
    if (vtype != type) {
        cout << "Error: type mismatch in assignment" << endl;
        exit(0);
    }

    return;
}

void ImpTypeChecker::visit(PrintStatement* s) {
    s->e->accept(this);
    return;
}

void ImpTypeChecker::visit(IfStatement* s) {
    s->cond->accept(this);
    s->tbody->accept(this);
    if (s->fbody != NULL)
        s->fbody->accept(this);
    return;
}

// Se verifica que la condicion sea booleana
void ImpTypeChecker::visit(WhileStatement* s) {
    ImpType tcond = s->cond->accept(this);
    if (tcond != TBOOL) {
        cout << "Error: condition in while statement must be boolean" << endl;
        exit(0);
    }

    s->body->accept(this);
    return;
}

void ImpTypeChecker::visit(DoWhileStatement* s) {
    ImpType tcond = s->cond->accept(this);
    if (tcond != TBOOL) {
        cout << "Error: condition in do-while statement must be boolean" << endl;
        exit(0);
    }

    s->body->accept(this);
    return;
}

// Se verifica que los operandos sean enteros
ImpType ImpTypeChecker::visit(BinaryExp* e) {
    ImpType t1 = e->left->accept(this);
    ImpType t2 = e->right->accept(this);
    if (t1 != TINT || t2 != TINT) {
        cout << "Error: binary operation with non-integer operands" << endl;
        exit(0);
    }

    ImpType result;
    switch (e->op) {
        case PLUS:
        case MINUS:
        case MULT:
        case DIV:
        case EXP:
            result = TINT;
            break;
        case LT:
        case LTEQ:
        case EQ:
            result = TBOOL;
            break;
    }
    return result;
}

ImpType ImpTypeChecker::visit(NumberExp* e) {
    ImpType t = TINT;  // ??
    return t;
}

// Se verifica que la variable este declarada
ImpType ImpTypeChecker::visit(IdExp* e) {
    if (!env.check(e->id)) {
        cout << "Error: variable " << e->id << " not declared" << endl;
        exit(0);
    }

    return env.lookup(e->id);
}

ImpType ImpTypeChecker::visit(ParenthExp* ep) {
    return ep->e->accept(this);
}

// Se verifica que la condicion sea booleana y que las expresiones
// verdadera y falsa tengan el mismo tipo
ImpType ImpTypeChecker::visit(CondExp* e) {
    ImpType btype = e->cond->accept(this);
    if (btype != TBOOL) {
        cout << "Error: condition in conditional expression must be boolean"
             << endl;
        exit(0);
    }

    ImpType ttype = e->etrue->accept(this);
    ImpType ftype = e->efalse->accept(this);

    if (ttype != ftype) {
        cout << "Error: type mismatch in conditional expression" << endl;
        exit(0);
    }

    return ttype;
}
