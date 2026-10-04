// Generates the 68k side of the OpenGL bridge from glFuncs.txt.
//
// Every OpenGL function becomes a plain C wrapper (_glXxx) that packs its
// arguments into the twelve register slots d1-d7/a1-a5 and calls qt_call(),
// which loads the registers and triggers the WinUAE native call trap.
// Doubles take two slots (low word first, as the x86 host expects them).
//
// Usage: glgen [directory]   (reads and writes files in that directory)

#include <fstream>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>

using std::string;
using std::ofstream;

static const int registerSlots = 12;

std::ifstream glFuncs;

class Type {
	string type;
public:
	void parse() {
		glFuncs >> type;
	}
	void print(ofstream& out) {
		out << "GL" << type;
	}
	bool isVoid() { return type == "void"; }
	bool isDouble() { return (type == "double") || (type == "clampd"); }
	bool isFloat() { return (type == "float") || (type == "clampf"); }
	bool isPointer() { return type.find('*') != string::npos; }
};

class FunctionName {
	string name;
public:
	void parse() {
		char c;
		for (glFuncs >> c; c != '('; glFuncs >> c) name += c;
		glFuncs.putback(c);
	}
	void print(ofstream& out) {
		out << name;
	}
};

class ArgumentName {
	string name;
public:
	void parse() {
		char c;
		for (glFuncs >> c; c != ',' && c != ')'; glFuncs >> c) name += c;
		glFuncs.putback(c);
	}
	void print(ofstream& out) {
		out << name;
	}
};

class ArgumentList {
	std::list<Type*> types;
	std::list<ArgumentName*> names;
public:
	void parse() {
		char c;
		glFuncs >> c;
		if (c != '(') throw std::runtime_error("( expected");
		glFuncs >> c;
		if (c == ')') return;
		glFuncs.putback(c);
		for (;;) {
			Type* type = new Type;
			ArgumentName* name = new ArgumentName;
			type->parse();
			name->parse();
			types.push_back(type);
			names.push_back(name);
			glFuncs >> c;
			if (c == ',') continue;
			if (c == ')') break;
			throw std::runtime_error(", or ) expected");
		}
	}
	void print(ofstream& out) {
		out << '(';
		if (types.empty()) out << "void";
		else {
			std::list<Type*>::iterator typeit = types.begin();
			std::list<ArgumentName*>::iterator nameit = names.begin();
			for (;;) {
				(*typeit)->print(out);
				out << " ";
				(*nameit)->print(out);
				++typeit; ++nameit;
				if (typeit != types.end()) out << ", ";
				else break;
			}
		}
		out << ')';
	}
	int slots() {
		int count = 0;
		for (std::list<Type*>::iterator it = types.begin(); it != types.end(); ++it) count += (*it)->isDouble() ? 2 : 1;
		return count;
	}
	void registers(ofstream& out) {
		out << "{";
		std::list<Type*>::iterator typeit = types.begin();
		std::list<ArgumentName*>::iterator nameit = names.begin();
		for (; typeit != types.end(); ++typeit, ++nameit) {
			if (typeit != types.begin()) out << ", ";
			if ((*typeit)->isDouble()) {
				out << "qt_dlo("; (*nameit)->print(out); out << "), ";
				out << "qt_dhi("; (*nameit)->print(out); out << ")";
			}
			else if ((*typeit)->isFloat()) {
				out << "qt_f2l("; (*nameit)->print(out); out << ")";
			}
			else {
				out << "(ULONG) "; (*nameit)->print(out);
			}
		}
		out << "}";
	}
};

class Function {
	Type returnType;
	FunctionName functionName;
	ArgumentList argumentList;
public:
	void parse() {
		returnType.parse();
		functionName.parse();
		argumentList.parse();
		string s;
		glFuncs >> s;
		if (s != ";") throw std::runtime_error("; expected");
	}
	void staticDeclare(ofstream& glc) {
		glc << "qtfn_";
		functionName.print(glc);
		glc << "," << std::endl;
	}
	void DLLfunc(ofstream& glc) {
		glc << "qtfn_";
		functionName.print(glc);
		glc << " = DLLfunc(gl, \"gl";
		functionName.print(glc);
		glc << "\");" << std::endl;
	}
	void definitions(ofstream& glc) {
		returnType.print(glc);
		glc << " _gl";
		functionName.print(glc);
		argumentList.print(glc);
		glc << " {" << std::endl;
		if (argumentList.slots() > registerSlots) {
			// Does not fit into the register slots of the native call trap.
			glc << "\t/* more than " << registerSlots << " register slots - not forwarded */" << std::endl;
			if (!returnType.isVoid()) glc << "\treturn 0;" << std::endl;
			glc << "}" << std::endl;
			return;
		}
		glc << "\tULONG qt_r[" << registerSlots << "] = ";
		argumentList.registers(glc);
		glc << ";" << std::endl << "\t";
		if (!returnType.isVoid()) {
			glc << "return (";
			returnType.print(glc);
			glc << ") ";
		}
		glc << "qt_call(qtfn_";
		functionName.print(glc);
		glc << ", qt_r);" << std::endl << "}" << std::endl;
	}
	void declarations(ofstream& glh) {
		returnType.print(glh);
		glh << " _gl";
		functionName.print(glh);
		argumentList.print(glh);
		glh << ";" << std::endl;
	}
};

int main(int argc, char** argv) {
	string dir = argc > 1 ? string(argv[1]) + "/" : string();
	glFuncs.open((dir + "glFuncs.txt").c_str());
	if (!glFuncs) {
		std::cerr << "Could not open " << dir << "glFuncs.txt" << std::endl;
		return 1;
	}

	std::list<Function*> functions;
	typedef std::list<Function*>::iterator iterator;
	try {
		for (glFuncs >> std::ws; glFuncs.peek() != std::char_traits<char>::eof(); glFuncs >> std::ws) {
			Function* function = new Function;
			function->parse();
			functions.push_back(function);
		}
	}
	catch (std::exception& e) {
		std::cerr << "glFuncs.txt: " << e.what() << " (after " << functions.size() << " functions)" << std::endl;
		return 1;
	}

	ofstream glstatic((dir + "glstatichandles.auto.c").c_str());
	for (iterator it = functions.begin(); it != functions.end(); ++it) (*it)->staticDeclare(glstatic);

	ofstream gldll((dir + "glDLLfunc.auto.c").c_str());
	for (iterator it = functions.begin(); it != functions.end(); ++it) (*it)->DLLfunc(gldll);

	ofstream gldef((dir + "gldefinitions.auto.c").c_str());
	for (iterator it = functions.begin(); it != functions.end(); ++it) (*it)->definitions(gldef);

	ofstream glh((dir + "gldeclarations.auto.h").c_str());
	for (iterator it = functions.begin(); it != functions.end(); ++it) (*it)->declarations(glh);

	std::cout << functions.size() << " functions generated" << std::endl;
	return 0;
}
