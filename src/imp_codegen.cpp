#include "imp_codegen.hh"

ImpCodeGen::ImpCodeGen(ImpTypeChecker* a) : analysis(a) {}

void ImpCodeGen::codegen(string label, string instr) {
    if (label != nolabel)
        code << label << ": ";
    code << instr << endl;
}

void ImpCodeGen::codegen(string label, string instr, int arg) {
    if (label != nolabel)
        code << label << ": ";
    code << instr << " " << arg << endl;
}

void ImpCodeGen::codegen(string label, string instr, string jmplabel) {
    if (label != nolabel)
        code << label << ": ";
    code << instr << " " << jmplabel << endl;
}

string ImpCodeGen::next_label() {
    string l = "L";
    string n = to_string(current_label++);
    l.append(n);
    return l;
}

string ImpCodeGen::get_flabel(string fname) {
    string l = "L";
    l.append(fname);
    return l;
}

void ImpCodeGen::codegen(Program* p, string outfname) {
    nolabel = "";
    current_label = 0;

    p->accept(this);
    ofstream outfile;
    outfile.open(outfname);
    outfile << code.str();
    outfile.close();

    return;
}

// Este codigo esta completo
void ImpCodeGen::visit(Program* p) {
    current_dir = 0;  // usado para generar nuebvas direcciones
    direcciones.add_level();
    process_global = true;
    p->var_decs->accept(this);
    process_global = false;

    mem_globals = current_dir;

    // codegen
    codegen("start", "skip");
    codegen(nolabel, "enter", mem_globals);
    codegen(nolabel, "alloc", mem_globals);
    codegen(nolabel, "mark");
    codegen(nolabel, "pusha", get_flabel("main"));
    codegen(nolabel, "call");
    codegen(nolabel, "halt");

    p->fun_decs->accept(this);
    direcciones.remove_level();
    return;
}

void ImpCodeGen::visit(Body* b) {
    // guardar direccion inicial current_dir
    int temp_dir = current_dir;

    direcciones.add_level();

    b->var_decs->accept(this);
    b->slist->accept(this);

    direcciones.remove_level();

    // restaurar dir
    current_dir = temp_dir;
    return;
}

void ImpCodeGen::visit(VarDecList* s) {
    list<VarDec*>::iterator it;
    for (it = s->vdlist.begin(); it != s->vdlist.end(); ++it) {
        (*it)->accept(this);
    }
    return;
}

// Se crea entrada para declaraciones de variables
void ImpCodeGen::visit(VarDec* vd) {
    list<string>::iterator it;
    for (it = vd->vars.begin(); it != vd->vars.end(); ++it) {
        current_dir++;
        // cout << "current_dir: " << current_dir << endl;
        VarEntry ventry;
        ventry.dir = current_dir;
        // cout << "current_dir pos: " << ventry.dir << endl;
        ventry.is_global = process_global;
        direcciones.add_var(*it, ventry);
    }
    return;
}

void ImpCodeGen::visit(FunDecList* s) {
    list<FunDec*>::iterator it;
    for (it = s->fdlist.begin(); it != s->fdlist.end(); ++it) {
        (*it)->accept(this);
    }
    return;
}

void ImpCodeGen::visit(FunDec* fd) {
    FEntry fentry = analysis->ftable.lookup(fd->fname);
    current_dir = 0;
    int m = fd->types.size();
    VarEntry ventry;

    // agregar direcciones de argumentos
    for (auto it = fd->vars.begin(); it != fd->vars.end(); ++it) {
        current_dir++;
        ventry.dir = current_dir - (m + 3);
        ventry.is_global = false;
        direcciones.add_var(*it, ventry);
    }

    // agregar direccion de return
    ventry.dir = -(m + 3);
    ventry.is_global = process_global;
    direcciones.add_var("return", ventry);

    // generar codigo para fundec

    num_params = m;

    codegen(get_flabel(fd->fname), "skip");
    codegen(nolabel, "enter", fentry.max_stack + fentry.mem_locals);
    codegen(nolabel, "alloc", fentry.mem_locals);

    // cout << fd->fname << " " << current_dir << endl;

    current_dir = 0;
    fd->body->accept(this);
    //  -- sacar comentarios para generar codigo del cuerpo

    return;
}

void ImpCodeGen::visit(StatementList* s) {
    list<Stm*>::iterator it;
    for (it = s->slist.begin(); it != s->slist.end(); ++it) {
        (*it)->accept(this);
    }
    return;
}

void ImpCodeGen::visit(AssignStatement* s) {
    s->rhs->accept(this);
    VarEntry ventry = direcciones.lookup(s->id);
    // generar codigo store/storer

    if (ventry.is_global)
        codegen(nolabel, "store", ventry.dir);
    else
        codegen(nolabel, "storer", ventry.dir);

    return;
}

void ImpCodeGen::visit(PrintStatement* s) {
    s->e->accept(this);
    code << "print" << endl;
    return;
}

void ImpCodeGen::visit(IfStatement* s) {
    string l1 = next_label();
    string l2 = next_label();

    s->cond->accept(this);
    codegen(nolabel, "jmpz", l1);
    s->tbody->accept(this);
    codegen(nolabel, "goto", l2);
    codegen(l1, "skip");
    if (s->fbody != NULL) {
        s->fbody->accept(this);
    }
    codegen(l2, "skip");

    return;
}

void ImpCodeGen::visit(WhileStatement* s) {
    string l1 = next_label();
    string l2 = next_label();

    codegen(l1, "skip");
    s->cond->accept(this);
    codegen(nolabel, "jmpz", l2);
    s->body->accept(this);
    codegen(nolabel, "goto", l1);
    codegen(l2, "skip");

    return;
}

void ImpCodeGen::visit(ReturnStatement* s) {
    // agregar codigo

    // codegen(nolabel, "loadr", direcciones.lookup("return").dir);
    // codegen(nolabel, "storer", -(num_params + 3));

    if (s->e != NULL) {
        s->e->accept(this);
        // codegen(nolabel, "loadr", num_params);
        codegen(nolabel, "storer", direcciones.lookup("return").dir);
    }

    codegen(nolabel, "return", num_params + 3);
    return;
}

void ImpCodeGen::visit(FCallStatement* s) {
    FEntry fentry = analysis->ftable.lookup(s->fname);
    ImpType ftype = fentry.ftype;

    // agregar codigo
    if (ftype.ttype != ImpType::VOID)
        codegen(nolabel, "alloc", 1);

    list<Exp*>::iterator it;

    for (it = s->args.begin(); it != s->args.end(); ++it) {
        (*it)->accept(this);
    }

    codegen(nolabel, "mark");
    codegen(nolabel, "pusha", get_flabel(s->fname));
    codegen(nolabel, "call");

    // descartar el valor de retorno
    if (ftype.ttype != ImpType::VOID)
        codegen(nolabel, "pop");

    return;
}

void ImpCodeGen::visit(ForStatement* s) {
    string l1 = next_label();
    string l2 = next_label();

    int temp_dir = current_dir;
    direcciones.add_level();

    // memoria para el iterador y el tope
    codegen(nolabel, "alloc", 2);
    current_dir += 2;

    VarEntry iter_entry;
    iter_entry.dir = current_dir - 1;
    iter_entry.is_global = false;
    direcciones.add_var(s->id, iter_entry);

    string end_id = "_" + s->id + "_end";
    VarEntry end_entry;
    end_entry.dir = current_dir;
    end_entry.is_global = false;
    direcciones.add_var(end_id, end_entry);

    // inicializar iterador y tope
    s->start->accept(this);
    codegen(nolabel, "storer", direcciones.lookup(s->id).dir);
    s->end->accept(this);
    codegen(nolabel, "storer", direcciones.lookup(end_id).dir);

    // bucle
    codegen(l1, "skip");

    // cond check
    codegen(nolabel, "loadr", direcciones.lookup(s->id).dir);
    codegen(nolabel, "loadr", direcciones.lookup(end_id).dir);
    codegen(nolabel, "le");
    codegen(nolabel, "jmpz", l2);

    s->body->accept(this);

    // incrementar iterador por 1
    codegen(nolabel, "loadr", direcciones.lookup(s->id).dir);
    codegen(nolabel, "push", 1);
    codegen(nolabel, "add");
    codegen(nolabel, "storer", direcciones.lookup(s->id).dir);

    codegen(nolabel, "goto", l1);

    // fin del bucle y limpiar memoria
    codegen(l2, "skip");
    codegen(nolabel, "pop");
    codegen(nolabel, "pop");

    direcciones.remove_level();

    current_dir = temp_dir;
    return;
}

int ImpCodeGen::visit(BinaryExp* e) {
    e->left->accept(this);
    e->right->accept(this);
    string op = "";
    switch (e->op) {
        case PLUS:
            op = "add";
            break;
        case MINUS:
            op = "sub";
            break;
        case MULT:
            op = "mul";
            break;
        case DIV:
            op = "div";
            break;
        case LT:
            op = "lt";
            break;
        case LTEQ:
            op = "le";
            break;
        case EQ:
            op = "eq";
            break;
        default:
            cout << "binop " << Exp::binopToString(e->op) << " not implemented"
                 << endl;
    }
    codegen(nolabel, op);
    return 0;
}

int ImpCodeGen::visit(NumberExp* e) {
    codegen(nolabel, "push ", e->value);
    return 0;
}

int ImpCodeGen::visit(TrueFalseExp* e) {
    codegen(nolabel, "push", e->value ? 1 : 0);

    return 0;
}

int ImpCodeGen::visit(IdExp* e) {
    VarEntry ventry = direcciones.lookup(e->id);
    if (ventry.is_global)
        codegen(nolabel, "load", ventry.dir);
    else
        codegen(nolabel, "loadr", ventry.dir);
    return 0;
}

int ImpCodeGen::visit(ParenthExp* ep) {
    ep->e->accept(this);
    return 0;
}

int ImpCodeGen::visit(CondExp* e) {
    string l1 = next_label();
    string l2 = next_label();

    e->cond->accept(this);
    codegen(nolabel, "jmpz", l1);
    e->etrue->accept(this);
    codegen(nolabel, "goto", l2);
    codegen(l1, "skip");
    e->efalse->accept(this);
    codegen(l2, "skip");
    return 0;
}

int ImpCodeGen::visit(FCallExp* e) {
    FEntry fentry = analysis->ftable.lookup(e->fname);
    ImpType ftype = fentry.ftype;

    // agregar codigo
    if (ftype.ttype != ImpType::VOID)
        codegen(nolabel, "alloc", 1);

    list<Exp*>::iterator it;

    for (it = e->args.begin(); it != e->args.end(); ++it) {
        (*it)->accept(this);
    }

    codegen(nolabel, "mark");
    codegen(nolabel, "pusha", get_flabel(e->fname));
    codegen(nolabel, "call");
    return 0;
}
