#include "utils/core.hpp"

bool flag = false;
std::string filename;
std::vector<Token> tokens;
std::vector<Statement*> statements;
VM globalVM;
std::map<std::string, ClassDeclaration*> classes;
std::map<std::string, Value> consts;

void printBytecode(const Bytecode& bytecode) {
    switch (bytecode.op) {
        case LOAD_CONST:
            std::cout << "LOAD_CONST";
            break;
        case LOAD_VAR:
            std::cout << "LOAD_VAR";
            break;
        case STORE_VAR:
            std::cout << "STORE_VAR";
            break;
        case BINARY_OP:
            std::cout << "BINARY_OP";
            break;
        case JUMP_IF_FALSE:
            std::cout << "JUMP_IF_FALSE";
            break;
        case CALL_FUNCTION:
            std::cout << "CALL_FUNCTION";
            break;
        case JUMP:
            std::cout << "JUMP";
            break;
        case RETURN:
            std::cout << "RETURN";
            break;
        case BUILD_LIST:
            std::cout << "BUILD_LIST";
            break;
        case POP:
            std::cout << "POP";
            break;
        case LOAD_SUBSCRIPT:
            std::cout << "LOAD_SUBSCRIPT";
            break;
        case STORE_SUBSCRIPT:
            std::cout << "STORE_SUBSCRIPT";
            break;
        case CREATE_OBJECT:
            std::cout << "CREATE_OBJECT";
            break;
        case LOAD_MEMBER:
            std::cout << "LOAD_MEMBER";
            break;
        case STORE_MEMBER:
            std::cout << "STORE_MEMBER";
            break;
        // case CLEAR:
        //     std::cout << "CLEAR";
        //     break;
        case LABEL:
            std::cout << "LABEL";
            break;
        default:
            std::cout << "Unknown opcode";
            break;
    }
    printf(" ");
    Bytecode instr = bytecode;
    try{
        if (!std::get<std::string>(instr.operand).empty()) {
            std::cout << " " << std::get<std::string>(instr.operand);
        }
    } catch(...) {

    }

    try{
        if (std::get<BigNum>(instr.operand)!= 0) {
            std::cout << " " << std::get<BigNum>(instr.operand).get_ll();
        }
    } catch(...) {

    }

    try{
        if (!std::get<CallFunctionOperand>(instr.operand).funcName.empty()) {
            std::cout << " " << std::get<CallFunctionOperand>(instr.operand).funcName;
        }
    } catch(...) {

    }

    std::cout << std::endl;
}

void interpreters() {
    try {
        CodeGen codegen(classes, consts);
        codegen.setReplMode(true);
        BytecodeProgram mainProgram = codegen.generate(statements);

        auto newFuncs = codegen.getFunctions();
        for (const auto& f : newFuncs) {
            globalVM.functions[f.first] = f.second;
        }

        classes = codegen.getClasses();
        consts = codegen.getConstants();

        if (globalVM.frames.empty()) {
            globalVM.frames.push(VM::Frame(mainProgram));
        } else {
            VM::Frame& globalFrame = globalVM.frames.top();
            globalFrame.program = mainProgram;
            globalFrame.pc = 0;
        }

    //    for (auto bytecode : mainProgram) {
    //        printBytecode(bytecode);
    //    }

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

            if (tokens.size() > 0 && tokens[0].type == TOKEN_KEYWORD && 
                (tokens[0].value == "fn" || tokens[0].value == "while" || 
                 tokens[0].value == "for" || tokens[0].value == "if" || 
                 tokens[0].value == "class")) {
                flags = true;
                std::string command;
                while (true) {
                    printf("... ");
                    std::getline(std::cin, command);
                    if (command.empty() || 
                        (command.find("end") != std::string::npos && 
                         command.find_first_not_of(" \t") == command.find("end"))) {
                        order += "\n" + command;
                        break;
                    }
                    order += "\n" + command;
                }
            }

            if (flags) lexers(order);
            parsers();
            interpreters();
            if (!globalVM.operandStack.empty() && globalVM.operandStack.top().type != Value::NULL_TYPE) {
                Value topValue = globalVM.operandStack.top();
                std::cout << "\n=> ";
                printValue(topValue);
                std::cout << std::endl;
                globalVM.operandStack.pop();
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