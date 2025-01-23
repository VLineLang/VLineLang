#include "core/core.hpp"

bool flag = false;
std::string filename;
long long maxDepth = -1;

std::vector<Token> tokens;
std::vector<Statement*> statements;
VM globalVM;


void interpreters() {
    try {
        CodeGen codegen;
        codegen.setReplMode(true);
        BytecodeProgram mainProgram = codegen.generate(statements);

        auto newFuncs = codegen.getFunctions();
        for (const auto& pair : newFuncs) {
            globalVM.functions[pair.first] = pair.second;
        }


        if (globalVM.frames.empty()) {
            globalVM.frames.push(VM::Frame(mainProgram));
        } else {

            VM::Frame& globalFrame = globalVM.frames.top();
            globalFrame.program = mainProgram;
            globalFrame.pc = 0;
        }

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
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        if (flag) {
            exit(1);
        }
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
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        if (flag) {
            exit(1);
        }
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
                Value topValue = globalVM.operandStack.top();
                std::cout << "=> ";
                printValue(topValue);
                std::cout << std::endl;
                globalVM.operandStack.pop();
            } else {
                std::cout << "=> null" << std::endl;
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