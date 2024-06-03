CC=g++

SRC_IMP=imp_test.cpp imp.cpp imp_parser.cpp imp_value.cpp imp_printer.cpp imp_interpreter.cpp

SRC_COMP=imp_compiler.cpp imp.cpp imp_parser.cpp imp_printer.cpp imp_typechecker.cpp imp_value.cpp imp_interpreter.cpp imp_codegen.cpp

SRC_SVM=svm_run.cpp svm_parser.cpp svm.cpp

imp: $(SRC_IMP)
	$(CC) -o imp  $(SRC_IMP)

compiler: $(SRC_IgMP)
	$(CC) -o compile  $(SRC_COMP)

svm: $(SRC_SVM)
	$(CC) -o svm  $(SRC_SVM)

CC = g++

SRC_DIR = src
EXMPL_DIR = examples
BIN_DIR = bin

SRC_IMP = $(SRC_DIR)/imp_test.cpp $(SRC_DIR)/imp.cpp $(SRC_DIR)/imp_parser.cpp $(SRC_DIR)/imp_value.cpp $(SRC_DIR)/imp_printer.cpp $(SRC_DIR)/imp_interpreter.cpp
SRC_COMP = $(SRC_DIR)/imp_compiler.cpp $(SRC_DIR)/imp.cpp $(SRC_DIR)/imp_parser.cpp $(SRC_DIR)/imp_printer.cpp $(SRC_DIR)/imp_typechecker.cpp $(SRC_DIR)/imp_value.cpp $(SRC_DIR)/imp_interpreter.cpp $(SRC_DIR)/imp_codegen.cpp
SRC_SVM = $(SRC_DIR)/svm_run.cpp $(SRC_DIR)/svm_parser.cpp $(SRC_DIR)/svm.cpp

# Crear directorio bin si no existe
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Regla para compilar imp
imp: $(BIN_DIR) $(SRC_IMP)
	$(CC) -o $(BIN_DIR)/imp $(SRC_IMP)

# Regla para compilar compiler
compiler: $(BIN_DIR) $(SRC_COMP)
	$(CC) -o $(BIN_DIR)/compile $(SRC_COMP)

# Regla para compilar svm
svm: $(BIN_DIR) $(SRC_SVM)
	$(CC) -o $(BIN_DIR)/svm $(SRC_SVM)

# Limpieza de archivos compilados
clean:
	rm -rf $(BIN_DIR)/*
