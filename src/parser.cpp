#include "parser.h"

namespace parser {
    //********************************************************************************************************
    //*							ToString
    //********************************************************************************************************

    // implement all the ToString function for Expression nodes below.
    void Factor::ToString() { *debugger::out << "[Factor]"; }

    void NumberConst::ToString() {
        switch (type) {
        case K_int: *debugger::out << "[" << static_cast<int>(value) << "]";
            return;
        case K_float: *debugger::out << "[" << value << "]", value;
            return;
        case K_double: *debugger::out << "[" << value << "]";
            return;
        default: *debugger::out << "[" << lexer::Token::Name(type) << "]";
            return;
        }
    }

    void String::ToString() { *debugger::out << "[\"" << value << "\"]"; }
    void Boolean::ToString() { *debugger::out << "[" << (value ? "true" : "false") << "]"; }
    void Field::ToString() { *debugger::out << "[" << names[0] << "]"; }

    void FuncCall::ToString() {
        *debugger::out << "[CALL " << names[0] << " ( ";
        for (auto& arg : args) {
            arg->ToString();
            *debugger::out << ",";
        }
        *debugger::out << " )]\n";
    }

    void Unary::ToString() {
        *debugger::out << "<" << lexer::Token::Name(op) << ">";
        expr->ToString();
    }

    void Binary::ToString() {
        *debugger::out << "(";
        LHS->ToString();
        *debugger::out << " " << lexer::Token::Name(op) << " ";
        RHS->ToString();
        *debugger::out << ")";
    }

    void Ternary::ToString() {
        *debugger::out << "[";
        a->ToString();
        *debugger::out << "?";
        b->ToString();
        *debugger::out << ":";
        c->ToString();
        *debugger::out << "]";
    }


    //********************************************************************************************************
    //*							Parser
    //********************************************************************************************************

    std::shared_ptr<Statement> Statements::Parse() {
        while (lexer::Check(NewLine)) {
            lexer::Next();
            VERIFY
        }
        if (lexer::Check('}')) return nullptr;
        const auto left = Statement::Parse();
        VERIFY
        const auto right = Statements::Parse();
        VERIFY
        if (right == nullptr)return left;
        return std::make_shared<Statements>(left, right);
    }

    std::shared_ptr<FuncCall> FuncCall::Parse(std::vector<std::wstring> f) {
        auto func_call = std::make_shared<FuncCall>(f);
        lexer::Next();
        VERIFY
        while (lexer::token->type != ')') {
            func_call->args.push_back(Binary::Parse());
            VERIFY
            if (lexer::Check(',')) lexer::Match(',');
            VERIFY
        }
        lexer::Match(')');
        VERIFY
        return func_call;
    }

    std::shared_ptr<Expr> Unary::ParsePostfix() {
        const auto factor = ParsePrefix();
        VERIFY
        switch (lexer::token->type) {

        case Inc:
        case Dec:
            lexer::Next();
            VERIFY
            return std::make_shared<Unary>(factor, lexer::token->type, false);
        default:
            return factor;
        }
    }

    std::shared_ptr<Expr> Unary::Parse() {
        return ParsePostfix();
    }

    std::shared_ptr<Expr> Unary::ParsePrefix() {
        switch (lexer::token->type) {
        case NewLine:
            debugger::AlertNewline();
            debugger::Alert(L"unexpected EndOfLine");
            return nullptr;
        case '-':
        case '!':
        case Inc:
        case Dec: {
            lexer::Next();
            VERIFY
            const auto parsed = Parse();
            VERIFY
            return std::make_shared<Unary>(parsed, lexer::token->type, true);
        }
        default:
            return Factor::Parse();
        }

    }

    std::shared_ptr<Expr> Ternary::Parse() {
        const auto a = Binary::Sub7();
        VERIFY
        if (lexer::token->type != '?')return a;
        lexer::Next();
        VERIFY
        const auto b = Binary::Sub7();
        VERIFY
        lexer::Match(':');
        VERIFY
        const auto c = Binary::Sub7();
        VERIFY
        return std::make_shared<Ternary>(a, b, c);
    }

    std::shared_ptr<Expr> Factor::Parse() {

        std::shared_ptr<Expr> factor;
        switch (lexer::token->type) {
        case '(': {
            lexer::Next();
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
            lexer::Match(')');
            VERIFY
            return factor;
            // }
        }
        case Str: {
            auto str = std::make_shared<String>(lexer::string_val);
            lexer::Next();
            VERIFY
            return str;
        }

        case Num: {
            const auto ty = lexer::token->value;
            lexer::Next();
            VERIFY
            return std::make_shared<NumberConst>(lexer::number_val, ty);
        }
        case K_true:
            lexer::Next();
            VERIFY
            return std::make_shared<Boolean>(true);
        case K_false:
            lexer::Next();
            VERIFY
            return std::make_shared<Boolean>(false);
        case K_int: case K_short: case K_long: case K_float: case K_double:
        case K_uint: case K_ushort: case K_ulong: case K_string:
        case Id: {
            // const auto id = string_val;
            std::vector<std::wstring> names;
            names.push_back(lexer::string_val);
            lexer::Next();
            VERIFY
            const auto type = lexer::token->type;
            while (lexer::token->type == '.') {
                lexer::Next();
                VERIFY
                names.push_back(lexer::string_val);
                lexer::Match(Id);
                VERIFY
            }
            if (lexer::Check('('))return FuncCall::Parse(names);
            else if (lexer::Check('[')) {
                VERIFY
            }
            else return std::make_shared<Field>(names);
        }
        default:
            debugger::Alert(
                (std::wstringstream() << "unexpected \"" << lexer::Token::Name(lexer::token->type) << "\" ").str());
            return nullptr;
        }
    }


    std::shared_ptr<FuncParam> FuncParam::Parse() {
        auto param = std::make_shared<FuncParam>();
        while (lexer::token->type != ')') {
            param->size++;
            if (lexer::CheckType()) {
                std::wstring t;
                t += static_cast<wchar_t>(lexer::token->type);
                param->types.push_back(t);
                lexer::Next();
                VERIFY
            }
            else if (lexer::token->type == '.')		// Parse three dots '...' for variable argument.
            {
                lexer::Next();
                VERIFY
                lexer::Match('.');
                VERIFY
                lexer::Match('.');
                VERIFY
                param->size--;
                param->isVarArg = true;
                return param;
            }
            else {
                param->types.push_back(lexer::string_val);
                lexer::Match(Id);
                VERIFY
            }

            param->names.push_back(lexer::string_val);
            lexer::Match(Id);
            VERIFY
            if (lexer::Check(',')) lexer::Match(',');
            VERIFY
        }
        return param;
    }

    std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext) {
        auto function = std::make_shared<FunctionDecl>();
        function->is_extern = ext;
        if (lexer::Check(K_dfunc))function->differentiable = true;
        else if (lexer::Check(K_kernal))function->kernal = true;
        lexer::Next();


        VERIFY
        if (lexer::Check(Id)) {
            function->name = lexer::string_val;
            lexer::Match(Id);
            VERIFY
        }
        lexer::Match('(');
        VERIFY
        function->args = FuncParam::Parse();
        lexer::Match(')');
        VERIFY

        function->return_type = '0';
        if (lexer::Check(':')) {
            lexer::Next();
            VERIFY
            if (lexer::CheckType()) {
                function->return_type = static_cast<wchar_t>(lexer::token->type);
                lexer::Next();
                VERIFY
            }
            else {
                function->return_type = lexer::string_val;
                lexer::Match(Id);
                VERIFY
            }

        }

        while (lexer::Check(NewLine))lexer::Next();
        VERIFY
        if (ext) {
            *debugger::out << "[Parsed] Extern function declaration\n";
            return function;
        }
        *debugger::out << "[Parsed] Function declaration\n";
        lexer::Match('{');
        VERIFY

        function->statements = Statements::Parse();
        while (lexer::Check(NewLine))lexer::Next();
        VERIFY

        lexer::Match('}');
        VERIFY
        *debugger::out << "[Parsed] Function end\n";
        return function;
    }

    std::shared_ptr<FieldDecl> FieldDecl::Parse(const bool is_const) {
        auto let = std::make_shared<FieldDecl>();
        let->constant = is_const;
        lexer::Next();
        VERIFY
        let->name = lexer::string_val;
        lexer::Match(Id);
        if (lexer::Check(':')) {
            lexer::Next();
            VERIFY
            if (lexer::CheckType()) {
                let->type = static_cast<wchar_t>(lexer::token->type);
                lexer::Next();
                VERIFY
            }
            else {
                let->type = lexer::string_val;
                lexer::Match(Id);
                VERIFY
            }
        }
        lexer::Match('=');
        VERIFY
        let->value = Binary::Parse();
        VERIFY

        if (lexer::Check(';')) lexer::Next();
        else lexer::Match(NewLine);
        VERIFY
        // PRINT("[Parsed] %s field declaration\n", is_const ? "Constant" : "Variable");let->value->ToString();PRINT("\n");
        return let;
    }

    std::shared_ptr<ClassDecl> ClassDecl::Parse() {
        auto instance = std::make_shared<ClassDecl>();
        lexer::Next();
        VERIFY
        instance->name = lexer::string_val;
        lexer::Match(Id);
        VERIFY
        while (lexer::Check(NewLine))lexer::Next();
        VERIFY
        lexer::Match('{');
        VERIFY


        while (true) {
            while (lexer::Check(NewLine))lexer::Next();
            VERIFY
            if (lexer::token->type == '}')break;
            switch (lexer::token->type) {
            case K_init:
            case K_deinit:
            case K_func:
            case K_dfunc:
            case K_kernal:
                instance->functions.push_back(FunctionDecl::Parse());
                VERIFY
                break;
            default:
                instance->fields.push_back(lexer::string_val);
                lexer::Match(Id);
                VERIFY
                lexer::Match(':');

                if (lexer::CheckType()) {
                    std::wstring t;
                    t += static_cast<wchar_t>(lexer::token->type);
                    instance->types.push_back(t);
                    lexer::Next();
                    VERIFY
                }
                else {
                    instance->types.push_back(lexer::string_val);
                    lexer::Match(Id);
                    VERIFY
                }

                if (lexer::Check(';')) lexer::Match(';');
                else lexer::Match(NewLine);
                while (lexer::Check(NewLine))lexer::Next();
                VERIFY
            }
        }
        lexer::Match('}');
        VERIFY
        return instance;
    }

    std::shared_ptr<If> If::Parse() {
        const auto instance = std::make_shared<If>();
        lexer::Next();
        lexer::Match('(');
        instance->condition = Binary::Parse();
        lexer::Match(')');
        if (lexer::Check('{')) {
            lexer::Next();
            instance->stmts = Statements::Parse();
            lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();


        if (lexer::Check(K_else)) {
            lexer::Next();
            if (lexer::Check('{')) {
                lexer::Next();
                instance->else_stmts = Statements::Parse();
                lexer::Match('}');
            }
            else instance->else_stmts = Statement::Parse();
        }
        else if (lexer::Check(K_elif))instance->else_stmts = Parse();

        return instance;
    }

    std::shared_ptr<While> While::Parse() {
        const auto instance = std::make_shared<While>();
        lexer::Next();
        lexer::Match('(');
        instance->condition = Binary::Parse();
        lexer::Match(')');
        if (lexer::Check('{')) {
            lexer::Next();
            instance->stmts = Statements::Parse();
            lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();
        return instance;
    }

    std::shared_ptr<For> For::Parse() {
        const auto instance = std::make_shared<For>();
        lexer::Next();
        lexer::Match('(');
        instance->condition = Binary::Parse();
        lexer::Match(')');
        if (lexer::Check('{')) {
            lexer::Next();
            instance->stmts = Statements::Parse();
            lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();
        return instance;
    }

    std::shared_ptr<Do> Do::Parse() {
        const auto instance = std::make_shared<Do>();
        lexer::Next();
        if (lexer::Check('{')) {
            lexer::Next();
            instance->stmts = Statements::Parse();
            lexer::Match('}');
        }
        else instance->stmts = Statement::Parse();

        lexer::Match(K_do);
        lexer::Match('(');
        instance->condition = Binary::Parse();
        lexer::Match(')');
        return instance;
    }

    std::shared_ptr<Throw> Throw::Parse() {
        lexer::Next();
        auto instance = std::make_shared<Throw>();
        instance->value = Factor::Parse();
        return instance;
    }

    std::shared_ptr<Empty> Empty::Parse() {

        auto instance = std::make_shared<Empty>();
        instance->value = Binary::Parse();
        VERIFY
        if (lexer::token->type == ';')lexer::Next();
        else lexer::Match(NewLine);
        VERIFY
        // PRINT("[Parsed] Expression \n");instance->value->ToString();
        return instance;
    }

    std::shared_ptr<Return> Return::Parse() {
        lexer::Next();
        auto instance = std::make_shared<Return>();
        if (lexer::token->type == ';' || lexer::token->type == NewLine) {
            instance->value = nullptr;
            lexer::Next();
            return instance;
        }
        instance->value = Binary::Parse();
        if (lexer::token->type == ';')lexer::Next();
        else lexer::Match(NewLine);
        *debugger::out << "[Parsed] Return Statement\n";
        return instance;
    }

    std::shared_ptr<Import> Import::Parse() {
        lexer::Next();
        auto instance = std::make_shared<Import>();
        return instance;
    }

    std::shared_ptr<Break> Break::Parse() {
        lexer::Next();
        auto instance = std::make_shared<Break>();
        return instance;
    }

    std::shared_ptr<Continue> Continue::Parse() {
        lexer::Next();
        auto instance = std::make_shared<Continue>();
        return instance;
    }

    std::shared_ptr<Statement> Statement::Parse() {
        while (lexer::Check(NewLine))lexer::Next();
        VERIFY
        switch (lexer::token->type) {
        case K_let: return FieldDecl::Parse(true);
        case K_var: return FieldDecl::Parse(false);
        case K_if: return If::Parse();
        case K_return: return Return::Parse();
        case K_do: return For::Parse();
        case K_while: return While::Parse();
        case K_for: return For::Parse();
        default:
            return Empty::Parse();
        }
    }

    void Program::ParseSingle() {
        switch (lexer::token->type) {
        case NewLine: lexer::Next();
            break;
        case K_func:
        case K_dfunc:
        case K_kernal: declarations.push_back(FunctionDecl::Parse());
            break;
        case K_extern: declarations.push_back(FunctionDecl::Parse(true));
            break;
        case K_import: Import::Parse();
            break;
        case K_class: declarations.push_back(ClassDecl::Parse());
            break;
        default: statements.push_back(Statement::Parse());
        }
        while (debugger::error_occurred) {
            debugger::error_occurred = false;
            lexer::MoveLine();
            lexer::Next();
        }
    }

    std::shared_ptr<Program> Program::Parse() {
        auto program = std::make_shared<Program>();
        while (lexer::peek > 0 && lexer::token != nullptr) program->ParseSingle();
        return program;
    }

    std::shared_ptr<Program> Parse() {
        lexer::Next();
        if (!debugger::only_tokenize)return parser::Program::Parse();
        while (lexer::peek > 0 && lexer::token != nullptr) {
            *debugger::out << "[" << lexer::Token::Name(lexer::token->type) << "] ";
            if (lexer::Check(NewLine) || lexer::Check(';'))*debugger::out << "\n";
            lexer::Next();
        }
        return nullptr;
    }


}
