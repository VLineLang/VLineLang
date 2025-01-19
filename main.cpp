#include "core/core.hpp"

bool flag = false;
std::string filename;
long long maxDepth = -1;

Interpreter interpreter;

void runs(const std::string& source) {
	Lexer lexer(source);
	std::vector<Token> tokens = lexer.tokenize();

	Parser parser(tokens);
	std::vector<Statement*> statements = parser.parse();

	if (maxDepth < 0) interpreter.interpret(statements);
    else interpreter.interpret(statements, maxDepth);
}

signed main(int argc, char **argv) {
	if (argc > 1) {
		bool used[argc];
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
                        used[i + 1] = true;
                    }
                }
			} else {
				if (!flag && !used[i]) {
					filename = std::string(argv[i]);
					flag = true;
				}
			}
			used[i] = true;
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
			runs(order);
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
			commands += command+"\n";

		runs(commands);

		inputFile.close();
	}

	return 0;
}
