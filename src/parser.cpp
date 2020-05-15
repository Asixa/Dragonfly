#include "parser.h"
#include <sstream>

namespace parser {
    //********************************************************************************************************
    //*							ToString
    //********************************************************************************************************

    // implement all the ToString function for Expression nodes below.
    void Factor::ToString() { *Debugger::out << "[Factor]"; }
    
    void NumberConst::ToString() {
        switch (type) {
        case K_int: *Debugger::out << "[" << static_cast<int>(value) << "]";
            return;
        case K_float: *Debugger::out << "[" << value << "]", value;
            return;
        case K_double: *Debugger::out << "[" << value << "]";
            return;
        default: *Debugger::out << "[" << Lexer::Token::Name(type) << "]";
            return;
        }
    }

    void String::ToString() { *Debugger::out << "[\"" << value << "\"]"; }
    void Boolean::ToString() { *Debugger::out << "[" << (value ? "true" : "false") << "]"; }
    void Field::ToString() {
        *Debugger::out <<  name;
		if (child != nullptr) {
			*Debugger::out << ".";
			child->ToString();
		}
    }

  

    void FuncCall::ToString() {
		if (left != nullptr)left->ToString();
		*Debugger::out << name<<"(";
		for (auto& arg : args) {
            arg->ToString();
            *Debugger::out << ",";
        }
		*Debugger::out << ")";
		if (child != nullptr) {
			*Debugger::out << ".";
			child->ToString();
		}
        //
        //
        // *Debugger::out << "[CALL " << name << " ( ";
        // for (auto& arg : args) {
        //     arg->ToString();
        //     *Debugger::out << ",";
        // }
        // *Debugger::out << " )]\n";
    }

    void Unary::ToString() {
        *Debugger::out << "<" << Lexer::Token::Name(op) << ">";
        expr->ToString();
    }

    void Binary::ToString() {
        *Debugger::out << "(";
        LHS->ToString();
        *Debugger::out << " " << Lexer::Token::Name(op) << " ";
        RHS->ToString();
        *Debugger::out << ")";
    }

    void Ternary::ToString() {
        *Debugger::out << "[";
        a->ToString();
        *Debugger::out << "?";
        b->ToString();
        *Debugger::out << ":";
        c->ToString();
        *Debugger::out << "]";
    }


    //********************************************************************************************************
    //*							Parser
    //********************************************************************************************************

    std::shared_ptr<Statement> Statements::Parse() {
        while (Lexer::Check(NewLine)) {
            Lexer::Next();
            VERIFY
        }
        if (Lexer::Check('}')) return nullptr;
        const auto left = Statement::Parse();
        VERIFY
        const auto right = Statements::Parse();
        VERIFY
        if (right == nullptr)return left;
        return std::make_shared<Statements>(left, right);
    }



    std::shared_ptr<Expr> Unary::ParsePostfix() {
        const auto factor = ParsePrefix();
        VERIFY
        switch (Lexer::token->type) {

        case Inc:
        case Dec:
            Lexer::Next();
            VERIFY
            return std::make_shared<Unary>(factor, Lexer::token->type, false);
        default:
            return factor;
        }
    }

    std::shared_ptr<Expr> Unary::Parse() {
        return ParsePostfix();
    }

    std::shared_ptr<Expr> Unary::ParsePrefix() {
        switch (Lexer::token->type) {
        case NewLine:
            Debugger::AlertNewline();
            Debugger::Alert(L"unexpected EndOfLine");
            return nullptr;
        case '-':
        case '!':
        case Inc:
        case Dec: {
            Lexer::Next();
            VERIFY
            const auto parsed = Parse();
            VERIFY
            return std::make_shared<Unary>(parsed, Lexer::token->type, true);
        }
        default:
            return Factor::Parse();
        }

    }

    std::shared_ptr<Expr> Ternary::Parse() {
        const auto a = Binary::Sub7();
        VERIFY
        if (Lexer::token->type != '?')return a;
        Lexer::Next();
        VERIFY
        const auto b = Binary::Sub7();
        VERIFY
        Lexer::Match(':');
        VERIFY
        const auto c = Binary::Sub7();
        VERIFY
        return std::make_shared<Ternary>(a, b, c);
    }

    std::shared_ptr<Expr> Factor::Parse() {

        std::shared_ptr<Expr> factor;
        switch (Lexer::token->type) {
        case '(': {
            Lexer::Next();
            // const auto point = src;
            // auto is_lambda = false;
            // Find('(', ')');
            // Next();												VERIFY
            // PRINT("ahead token is :"<<Token::Name(token->type)<<"\n", );
            // is_lambda = token->type == Arrow;
            // 	
            // PRINT("isLambda:%d   ", is_lambda);
            // src = point;
            // Next();												VERIFY
            // PRINT("current token is :"<<Token::Name(token->type)<<"\n");
            // system("PAUSE");
            // if (is_lambda)
            // {
            //
            // }
            // else {

            factor = Binary::Parse();
            VERIFY
            Lexer::Match(')');
            VERIFY
            return factor;
            // }
        }
        case Str: {
            auto str = std::make_shared<String>(Lexer::string_val);
            Lexer::Next();
            VERIFY
            return str;
        }

        case Num: {
            const auto ty = Lexer::token->value;
            Lexer::Next();
            VERIFY
            return std::make_shared<NumberConst>(Lexer::number_val, ty);
        }
        case K_true:
            Lexer::Next();
            VERIFY
            return std::make_shared<Boolean>(true);
        case K_false:
            Lexer::Next();
            VERIFY
            return std::make_shared<Boolean>(false);


        case Id: {
			return Field::Parse();
        }
		case K_int: case K_short: case K_long: case K_float: case K_double:
		case K_uint: case K_ushort: case K_ulong: case K_string:
        default:
            Debugger::Alert(
                (std::wstringstream() << "unexpected \"" << Lexer::Token::Name(Lexer::token->type) << "\" ").str());
            return nullptr;
        }
    }

	std::shared_ptr<Field> Field::Parse() {
		auto field = ParsePostfix();
		auto root = field;
        while (Lexer::token->type=='.') {
			Lexer::Next();
            field->child = ParsePostfix();
			field = field->child;
        }
		return root;
    }
	std::shared_ptr<Field> Field::ParsePostfix() {
		auto name = Lexer::string_val;
		
		Lexer::Next();
		VERIFY

			if (Lexer::token->type == '(' || Lexer::token->type == '[')
			{
				std::shared_ptr<Field> field = nullptr;

				while (true) {
					bool br = false;
					switch (Lexer::token->type) {
					case '(': {
						auto child = field;
						field = FuncCall::Parse(name);
						if (!name.empty())name = L"";
						field->left = child;
						break;
					}
					case '[': {
						// array access
						break;
					}
					default:
						br = true;
						break;
					}
					if (br)break;
				}
				return field;
			}
			else {
			    return std::make_shared<Field>(name);
			}
	}

	std::shared_ptr<FuncCall> FuncCall::Parse(std::wstring f) {
		
		auto func_call = std::make_shared<FuncCall>(f);
		Lexer::Next(); VERIFY
			while (Lexer::token->type != ')') {
				func_call->args.push_back(Binary::Parse());
				VERIFY
					if (Lexer::Check(',')) Lexer::Match(',');
				VERIFY
			}
		Lexer::Match(')');
		VERIFY
			return func_call;
	}

    std::shared_ptr<FuncParam> FuncParam::Parse() {
        auto param = std::make_shared<FuncParam>();
        while (Lexer::token->type != ')') {
            param->size++;
            if (Lexer::CheckType()) {
                std::wstring t;
                t += static_cast<wchar_t>(Lexer::token->type);
                param->types.push_back(t);
                Lexer::Next();
                VERIFY
            }
            else if (Lexer::token->type == '.')		// Parse three dots '...' for variable argument.
            {
                Lexer::Next();
                VERIFY
                Lexer::Match('.');
                VERIFY
                Lexer::Match('.');
                VERIFY
                param->size--;
                param->isVarArg = true;
                return param;
            }
            else {
                param->types.push_back(Lexer::string_val);
                Lexer::Match(Id);
                VERIFY
            }

            param->names.push_back(Lexer::string_val);
            Lexer::Match(Id);
            VERIFY
            if (Lexer::Check(',')) Lexer::Match(',');
            VERIFY
        }
        return param;
    }

    std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext) {
        auto function = std::make_shared<FunctionDecl>();
        function->is_extern = ext;
        if (Lexer::Check(K_dfunc))function->differentiable = true;
        else if (Lexer::Check(K_kernal))function->kernal = true;
        Lexer::Next();


        VERIFY
        if (Lexer::Check(Id)) {
            function->name = Lexer::string_val;
            Lexer::Match(Id);
            VERIFY
        }
        Lexer::Match('(');
        VERIFY
        function->args = FuncParam::Parse();
        Lexer::Match(')');
        VERIFY

        function->return_type = '0';
        if (Lexer::Check(':')) {
            Lexer::Next();
            VERIFY
            if (Lexer::CheckType()) {
                function->return_type = static_cast<wchar_t>(Lexer::token->type);
                Lexer::Next();
                VERIFY
            }
            else {
                function->return_type = Lexer::string_val;
                Lexer::Match(Id);
                VERIFY
            }

        }

        while (Lexer::Check(NewLine))Lexer::Next();
        VERIFY
        if (ext) {
            *Debugger::out << "[Parsed] Extern function declaration\n";
            return function;
        }
        *Debugger::out << "[Parsed] Function declaration\n";
        Lexer::Match('{');
        VERIFY

        function->statements = Statements::Parse();
        while (Lexer::Check(NewLine))Lexer::Next();
        VERIFY

        Lexer::Match('}');
        VERIFY
        *Debugger::out << "[Parsed] Function end\n";
        return function;
    }

    std::shared_ptr<FieldDecl> FieldDecl::Parse(const bool is_const) {
        auto let = std::make_shared<FieldDecl>();
        let->constant = is_const;
        Lexer::Next();
        VERIFY
        let->name = Lexer::string_val;
        Lexer::Match(Id);
        if (Lexer::Check(':')) {
            Lexer::Next();
            VERIFY
            if (Lexer::CheckType()) {
                let->type = static_cast<wchar_t>(Lexer::token->type);
                Lexer::Next();
                VERIFY
            }
            else {
                let->type = Lexer::string_val;
                Lexer::Match(Id);
                VERIFY
            }
        }
        Lexer::Match('=');
        VERIFY
        let->value = Binary::Parse();
        VERIFY

        if (Lexer::Check(';')) Lexer::Next();
        else Lexer::Match(NewLine);
        VERIFY
        // PRINT("[Parsed] %s field declaration\n", is_const ? "Constant" : "Variable");let->value->ToString();PRINT("\n");
        return let;
    }

    std::shared_ptr<ClassDecl> ClassDecl::Parse() {
        auto instance = std::make_shared<ClassDecl>();
        Lexer::Next();
        VERIFY
        instance->name = Lexer::string_val;
        Lexer::Match(Id);

        if(Lexer::Check(':')) {
			Lexer::Next();
			instance->interfaces.push_back(Lexer::string_val);
            while (Lexer::Check(',')) {
				Lexer::Next();
				instance->interfaces.push_back(Lexer::string_val);
				Lexer::Match(Id);
            }
        }

        VERIFY
        while (Lexer::Check(NewLine))Lexer::Next();
        VERIFY
        Lexer::Match('{');
        VERIFY


        while (true) {
            while (Lexer::Check(NewLine))Lexer::Next();
            VERIFY
            if (Lexer::token->type == '}')break;
            switch (Lexer::token->type) {
            case K_init:
            case K_deinit:
            case K_func:
            case K_dfunc:
            case K_kernal:
                instance->functions.push_back(FunctionDecl::Parse());
                VERIFY
                break;
            default:
                instance->fields.push_back(Lexer::string_val);
                Lexer::Match(Id);
                VERIFY
                Lexer::Match(':');

                if (Lexer::CheckType()) {
                    std::wstring t;
                    t += static_cast<wchar_t>(Lexer::token->type);
                    instance->types.push_back(t);
                    Lexer::Next();
                    VERIFY
                }
                else {
                    instance->types.push_back(Lexer::string_val);
                    Lexer::Match(Id);
                    VERIFY
                }

                if (Lexer::Check(';')) Lexer::Match(';');
                else Lexer::Match(NewLine);
                while (Lexer::Check(NewLine))Lexer::Next();
                VERIFY
            }
        }
        Lexer::Match('}');
        VERIFY
        return instance;
    }

	std::shared_ptr<Extension> Extension::Parse() {
		auto instance = std::make_shared<Extension>();
		Lexer::Next();
		VERIFY
			instance->name = Lexer::string_val;
		Lexer::Match(Id);


		VERIFY
			while (Lexer::Check(NewLine))Lexer::Next();
		VERIFY

			const auto brackets = Lexer::Check('{');
		if (brackets) Lexer::Next();
		else Lexer::Match(Arrow);
		VERIFY
			while (true) {
				while (Lexer::Check(NewLine))Lexer::Next();
				VERIFY
					if (Lexer::token->type == '}')break;
				switch (Lexer::token->type) {
				case K_init:
				case K_deinit:
				case K_func:
				case K_dfunc:
				case K_kernal:
					instance->functions.push_back(FunctionDecl::Parse());
					VERIFY
						break;
				default:
					VERIFY
				}
			}
		if (brackets)Lexer::Match('}');
		VERIFY
			return instance;
	}

    std::shared_ptr<If> If::Parse() {
        const auto instance = std::make_shared<If>();
        Lexer::Next();
        Lexer::Match('(');
        instance->condition = Binary::Parse();
        Lexer::Match(')');
        if (Lexer::Check('{')) {
            Lexer::Next();
            instance->stmts = Statements::Parse();
            Lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();


        if (Lexer::Check(K_else)) {
            Lexer::Next();
            if (Lexer::Check('{')) {
                Lexer::Next();
                instance->else_stmts = Statements::Parse();
                Lexer::Match('}');
            }
            else instance->else_stmts = Statement::Parse();
        }
        else if (Lexer::Check(K_elif))instance->else_stmts = Parse();

        return instance;
    }

    std::shared_ptr<While> While::Parse() {
        const auto instance = std::make_shared<While>();
        Lexer::Next();
        Lexer::Match('(');
        instance->condition = Binary::Parse();
        Lexer::Match(')');
        if (Lexer::Check('{')) {
            Lexer::Next();
            instance->stmts = Statements::Parse();
            Lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();
        return instance;
    }

    std::shared_ptr<For> For::Parse() {
        const auto instance = std::make_shared<For>();
        Lexer::Next();
        Lexer::Match('(');
        instance->condition = Binary::Parse();
        Lexer::Match(')');
        if (Lexer::Check('{')) {
            Lexer::Next();
            instance->stmts = Statements::Parse();
            Lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();
        return instance;
    }

    std::shared_ptr<Do> Do::Parse() {
        const auto instance = std::make_shared<Do>();
        Lexer::Next();
        if (Lexer::Check('{')) {
            Lexer::Next();
            instance->stmts = Statements::Parse();
            Lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();

        Lexer::Match(K_do);
        Lexer::Match('(');
        instance->condition = Binary::Parse();
        Lexer::Match(')');
        return instance;
    }

    std::shared_ptr<Switch> Switch::Parse() {
		Lexer::Next();
		Lexer::Match('(');
		Lexer::Match(')');
		Lexer::Match('{');

		while (true) {
            break;
		}

		Lexer::Match('}');
		return nullptr;
    }

    std::shared_ptr<Throw> Throw::Parse() {
        Lexer::Next();
        auto instance = std::make_shared<Throw>();
        instance->value = Factor::Parse();
        return instance;
    }

    std::shared_ptr<Empty> Empty::Parse() {

        auto instance = std::make_shared<Empty>();
        instance->value = Binary::Parse();
        VERIFY
        if (Lexer::token->type == ';')Lexer::Next();
        else Lexer::Match(NewLine);
        VERIFY
        // PRINT("[Parsed] Expression \n");instance->value->ToString();
        return instance;
    }

    std::shared_ptr<Return> Return::Parse() {
        Lexer::Next();
        auto instance = std::make_shared<Return>();
        if (Lexer::token->type == ';' || Lexer::token->type == NewLine) {
            instance->value = nullptr;
            Lexer::Next();
            return instance;
        }
        instance->value = Binary::Parse();
        if (Lexer::token->type == ';')Lexer::Next();
        else Lexer::Match(NewLine);
        *Debugger::out << "[Parsed] Return Statement\n";
        return instance;
    }

    std::shared_ptr<Import> Import::Parse() {
        Lexer::Next();
        auto instance = std::make_shared<Import>();
        return instance;
    }

    std::shared_ptr<Break> Break::Parse() {
        Lexer::Next();
        auto instance = std::make_shared<Break>();
        return instance;
    }

    std::shared_ptr<Continue> Continue::Parse() {
        Lexer::Next();
        auto instance = std::make_shared<Continue>();
        return instance;
    }

    std::shared_ptr<Statement> Statement::Parse() {
        while (Lexer::Check(NewLine))Lexer::Next();
        VERIFY
        switch (Lexer::token->type) {
        case K_let:         return FieldDecl::Parse(true);
        case K_var:         return FieldDecl::Parse(false);
        case K_if:          return If::Parse();
        case K_return:      return Return::Parse();
        case K_do:          return For::Parse();
        case K_while:       return While::Parse();
		case K_for:         return For::Parse();
		case K_continue:    return Break::Parse();
		case K_break:       return Continue::Parse();
        default:            return Empty::Parse();
        }
    }

    void Program::ParseSingle() {
        switch (Lexer::token->type) {
        case NewLine: Lexer::Next(); break;
        case K_func:
        case K_dfunc:
        case K_kernal: declarations.push_back(FunctionDecl::Parse()); break;
        case K_extern: declarations.push_back(FunctionDecl::Parse(true)); break;
        case K_import: Import::Parse(); break;
        case K_class: declarations.push_back(ClassDecl::Parse()); break;
        default: statements.push_back(Statement::Parse());
        }
        while (Debugger::error_occurred) {
            Debugger::error_occurred = false;
            Lexer::MoveLine();
            Lexer::Next();
        }
    }

    std::shared_ptr<Program> Program::Parse() {
        auto program = std::make_shared<Program>();
        while (Lexer::peek > 0 && Lexer::token != nullptr) program->ParseSingle();
        return program;
    }

    std::shared_ptr<Program> Parse() {
        Lexer::Next();
        if (!Debugger::only_tokenize)return parser::Program::Parse();
        while (Lexer::peek > 0 && Lexer::token != nullptr) {
            *Debugger::out << "[" << Lexer::Token::Name(Lexer::token->type) << "] ";
            if (Lexer::Check(NewLine) || Lexer::Check(';'))*Debugger::out << "\n";
            Lexer::Next();
        }
        return nullptr;
    }


}
