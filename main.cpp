#include "core/core.hpp"

bool flag = false;
std::string filename;
long long maxDepth = -1;

std::vector<Token> tokens;
std::vector<Statement*> statements;
VM globalVM;

//void debugBytecode(const Bytecode& bytecode) {
//    switch (bytecode.op) {
//        case LOAD_CONST:
//            std::cout << "LOAD_CONST";
//            if (std::holds_alternative<double>(bytecode.operand)) {
//                std::cout << " (Value: " << std::get<double>(bytecode.operand) << ")";
//            } else if (std::holds_alternative<std::string>(bytecode.operand)) {
//                std::cout << " (Value: " << std::get<std::string>(bytecode.operand) << ")";
//            }
//            break;
//        case LOAD_VAR:
//            std::cout << "LOAD_VAR";
//            if (std::holds_alternative<std::string>(bytecode.operand)) {
//                std::cout << " (Variable: " << std::get<std::string>(bytecode.operand) << ")";
//            }
//            break;
//        case STORE_VAR:
//            std::cout << "STORE_VAR";
//            if (std::holds_alternative<std::string>(bytecode.operand)) {
//                std::cout << " (Variable: " << std::get<std::string>(bytecode.operand) << ")";
//            }
//            break;
//        case BINARY_OP:
//            std::cout << "BINARY_OP";
//            if (std::holds_alternative<CompareOp>(bytecode.operand)) {
//                CompareOp cmpOp = std::get<CompareOp>(bytecode.operand);
//                std::cout << " (Compare Op: ";
//                switch (cmpOp) {
//                    case CMP_LT: std::cout << "<"; break;
//                    case CMP_LE: std::cout << "<="; break;
//                    case CMP_EQ: std::cout << "=="; break;
//                    case CMP_NE: std::cout << "!="; break;
//                    case CMP_GT: std::cout << ">"; break;
//                    case CMP_GE: std::cout << ">="; break;
//                }
//                std::cout << ")";
//            }
//            break;
//        case JUMP_IF_FALSE:
//            std::cout << "JUMP_IF_FALSE";
//            if (std::holds_alternative<int>(bytecode.operand)) {
//                std::cout << " (Jump Offset: " << std::get<int>(bytecode.operand) << ")";
//            }
//            break;
//        case CALL_FUNCTION:
//            std::cout << "CALL_FUNCTION";
//            if (std::holds_alternative<CallFunctionOperand>(bytecode.operand)) {
//                CallFunctionOperand callOp = std::get<CallFunctionOperand>(bytecode.operand);
//                std::cout << " (Function: " << callOp.funcName << ", Arg Count: " << callOp.argCount << ")";
//            }
//            break;
//        case JUMP:
//            std::cout << "JUMP";
//            if (std::holds_alternative<int>(bytecode.operand)) {
//                std::cout << " (Jump Offset: " << std::get<int>(bytecode.operand) << ")";
//            }
//            break;
//        case RETURN:
//            std::cout << "RETURN";
//            break;
//        case BUILD_LIST:
//            std::cout << "BUILD_LIST";
//            if (std::holds_alternative<int>(bytecode.operand)) {
//                std::cout << " (List Size: " << std::get<int>(bytecode.operand) << ")";
//            }
//            break;
//        case GET_ITER:
//            std::cout << "GET_ITER";
//            break;
//        case FOR_ITER:
//            std::cout << "FOR_ITER";
//            if (std::holds_alternative<int>(bytecode.operand)) {
//                std::cout << " (Iter Offset: " << std::get<int>(bytecode.operand) << ")";
//            }
//            break;
//        case POP:
//            std::cout << "POP";
//            break;
//        case LOAD_SUBSCRIPT:
//            std::cout << "LOAD_SUBSCRIPT";
//            break;
//        case STORE_SUBSCRIPT:
//            std::cout << "STORE_SUBSCRIPT";
//            break;
//        default:
//            std::cout << "UNKNOWN_OP";
//            break;
//    }
//    std::cout << std::endl;
//}

void interpreters() {
    try {
        CodeGen codegen;
        codegen.setReplMode(true);
        BytecodeProgram mainProgram = codegen.generate(statements);

        auto newFuncs = codegen.getFunctions();
        for (const auto& pair : newFuncs) {
            globalVM.functions[pair.first] = pair.second;
//            for (auto pg : pair.second->bytecode) {
//                debugBytecode(pg);
//            }
        }


        if (globalVM.frames.empty()) {
            globalVM.frames.push_back(VM::Frame(mainProgram));
        } else {
            VM::Frame& globalFrame = globalVM.frames.back();
            globalFrame.program = mainProgram;
            globalFrame.pc = 0;
        }

//        for (auto pg : globalVM.frames.back().program) {
//            debugBytecode(pg);  // debug
//        }

        globalVM.execute();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        if (flag) exit(1);
    }
}

void lexers(std::string command) {
    try {
        tokens.clear();
        Lexer lexer(command);
        tokens = lexer.tokenize();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        if (flag) {
            exit(1);
        }
        tokens.clear();
    }
}

void parsers() {
    try {
        statements.clear();
        Parser parser(tokens);
        statements = parser.parse();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        if (flag) {
            exit(1);
        }
        statements.clear();
    }
}

signed main(int argc, char *argv[]) {
    if (argc > 1) {
        std::vector<bool> used(argc, false);
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-' && argv[i][1] == '-') {
                std::string op = std::string(argv[i]).substr(2);
                if (op == "out") {
                    if (i + 1 >= argc) throw std::runtime_error("Can't open file (empty filename)");
                    else {
                        char* fn = argv[i + 1];
                        freopen(fn, "w", stdout);
                        used[i + 1] = true;
                    }
                }
                if (op == "in") {
                    if (i + 1 >= argc) throw std::runtime_error("Can't open file (empty filename)");
                    else {
                        char* fn = argv[i + 1];
                        freopen(fn, "r", stdin);
                        used[i + 1] = true;
                    }
                }
                if (op == "depth") {
                    if (i + 1 >= argc) throw std::runtime_error("Can't set max depth (empty value)");
                    else {
                        maxDepth = std::atoi(argv[i + 1]);
                        if (maxDepth > 0) globalVM.setDepthLimit(maxDepth);
                        used[i + 1] = true;
                    }
                }
            } else if (!used[i]) {
                if (!flag) {
                    filename = std::string(argv[i]);
                    flag = true;
                }
                used[i] = true;
            }
        }
    }

    if (!flag) {
        printf("VLine Compiler %s (publish on %s) [%s]\n", VLINE_VERSION, VLINE_PUBLISH, VLINE_COMPILER);
        std::string order;
        printf("Type `quit` to exit or type `__version__` to get VLine compiler version.\n");
        while(true) {
            printf("\n>>> ");
            std::getline(std::cin, order);
            if(order=="quit")break;
            if(order=="__version__") {
                printf("%s", VLINE_VERSION);
                continue;
            }

            lexers(order);

            bool flags = false;

            if (tokens.size() > 1 && tokens[tokens.size()-2].type == TOKEN_PUNCTUATION && tokens[tokens.size()-2].value == "{") {
                flags = true;
                std::string command;
                while (true) {
                    printf("... ");
                    std::getline(std::cin, command);
                    if (command.empty()) {
                        break;
                    }
                    order += "\n" + command;
                }
            }

            if (flags) lexers(order);
            parsers();
            interpreters();
            if (!globalVM.operandStack.empty()) {
                Value topValue = globalVM.operandStack.back();
                printf("\n=> ");
                printValue(topValue);
                printf("\n");
                globalVM.operandStack.pop_back();
            } else {
                printf("\n=> null\n");
            }
        }
    } else {
        std::ifstream inputFile;
        inputFile.open(filename);
        if (!inputFile.is_open()) {
            inputFile.close();
            throw std::runtime_error("Can't open file \"" + filename + "\" to run.");
        }

        std::string command, commands;

        while (getline(inputFile, command))
            commands += command + "\n";

        lexers(commands);
        parsers();
        interpreters();

        inputFile.close();
    }

    return 0;
}