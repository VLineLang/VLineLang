#include "core/core.hpp"

bool flag = false; //是否作为REPL运行(默认为false)
std::string filename; //运行的文件名
long long maxDepth = -1; //最大递归深度（-1为无限制）

std::vector<Token> tokens; //token列表
std::vector<Statement *> statements; //语句列表
VM globalVM; //全局虚拟机

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
    //解释器函数
    try {
        CodeGen codegen; //初始化字节码生成器
        codegen.setReplMode(true);
        BytecodeProgram mainProgram = codegen.generate(statements); //生成字节码

        std::unordered_map<std::string, FunctionDeclaration *> newFuncs = codegen.getFunctions(); //获得新建函数
        for (const auto &pair: newFuncs) {
            //遍历新建的函数
            globalVM.functions[pair.first] = pair.second; //函数名转为字节码
            //            for (auto pg : pair.second->bytecode) {
            //                debugBytecode(pg);
            //            }
        }


        if (globalVM.frames.empty()) {
            globalVM.frames.emplace_back(mainProgram); //将程序push进VM的栈中
        } else {
            VM::Frame &globalFrame = globalVM.frames.back(); //将最顶层改为当前程序
            globalFrame.program = mainProgram;
            globalFrame.pc = 0;
        }

        //        for (auto pg : globalVM.frames.back().program) {
        //            debugBytecode(pg);  // debug
        //        }

        globalVM.execute(); //执行栈中代码
    } catch (const std::runtime_error &e) {
        //当解释器出现错误时 输出runtime_error以调试
        std::cerr << e.what() << std::endl; //输出错误信息
        if (flag) exit(1);
    }
}

void lexers(std::string command) {
    //语法分析器函数
    try {
        tokens.clear(); //清空token表
        Lexer lexer(command); //以输入命令来初始化语法分析器
        tokens = lexer.tokenize(); //获得转换得到的tokens
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        if (flag) {
            exit(1);
        }
        tokens.clear();
    }
}

void parsers() {
    //转换器函数 将程序代码转换成AST(抽象语法树)
    try {
        statements.clear(); //清空语句列表
        Parser parser(tokens); //以输入命令来初始化AST转换器
        statements = parser.parse(); //获得转换得到的语句
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        if (flag) {
            exit(1);
        }
        statements.clear();
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        //如果运行参数大于1 解析参数
        std::vector<bool> used(argc, false); //创建文件名列表
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-' && argv[i][1] == '-') {
                //如果以--开头 则作为执行参数
                std::string op = std::string(argv[i]).substr(2);
                if (op == "out") {
                    // 标准输出的目标输出文件名
                    if (i + 1 >= argc) throw std::runtime_error("Can't open file (empty filename)");
                    else {
                        char *fn = argv[i + 1];
                        freopen(fn, "w", stdout);
                        used[i + 1] = true; //将该参数标记为解析过
                    }
                }
                if (op == "in") {
                    // 标准输入的目标输出文件名
                    if (i + 1 >= argc) throw std::runtime_error("Can't open file (empty filename)");
                    else {
                        char *fn = argv[i + 1];
                        freopen(fn, "r", stdin);
                        used[i + 1] = true; //将该参数标记为解析过
                    }
                }
                if (op == "depth") {
                    //递归深度
                    if (i + 1 >= argc) throw std::runtime_error("Can't set max depth (empty value)");
                    else {
                        maxDepth = std::atoi(argv[i + 1]);
                        if (maxDepth > 0) globalVM.setDepthLimit(maxDepth);
                        used[i + 1] = true; //将该参数标记为解析过
                    }
                }
            } else if (!used[i]) {
                //找到
                if (!flag) {
                    filename = std::string(argv[i]);
                    flag = true;
                }
                used[i] = true;
            }
        }
    }

    if (flag) {
        //打开文件 创建输入流
        std::ifstream inputFile;
        inputFile.open(filename);
        if (!inputFile.is_open()) {
            inputFile.close();
            throw std::runtime_error("Can't open file \"" + filename + "\" to run.");
        }

        //读出文件
        std::string command, commands;

        while (getline(inputFile, command))
            commands += command + "\n";
        //执行程序
        lexers(commands);
        parsers();
        interpreters();

        inputFile.close();
    } else {
        //显示欢迎语句
        printf("VLine Compiler %s (publish on %s) [%s]\n", VLINE_VERSION, VLINE_PUBLISH, VLINE_COMPILER);
        printf("Type `quit` to exit or type `__version__` to get VLine compiler version.\n");
        std::string order; //每一行的输入
        while (true) {
            //输入命令
            printf("\n>>> ");
            std::getline(std::cin, order);

            //特殊指令
            if (order == "quit") break;
            if (order == "sheep") {
                printf("VLine Interpreter - sheep edition");
                continue;
            }
            if (order == "__version__") {
                printf("%s", VLINE_VERSION);
                continue;
            }

            lexers(order);

            bool flags = false;

            //如果当前token为 "{" 那么允许输入多行
            if (tokens.size() > 1 && tokens[tokens.size() - 2].type == TOKEN_PUNCTUATION && tokens[tokens.size() - 2].
                value == "{") {
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
            parsers(); //转换代码
            interpreters(); //执行代码
            //输出返回值
            if (!globalVM.operandStack.empty()) {
                //获得调用栈的返回值并且弹出该值
                Value topValue = globalVM.operandStack.back();
                printf("=> ");
                printValue(topValue);
                printf("\n");
                globalVM.operandStack.pop_back();
            } else {
                //没有返回值输出null
                printf("=> null\n");
            }
        }
    }

    return 0;
}
