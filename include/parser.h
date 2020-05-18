// Copyright 2019 The Dragonfly Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>


#include "AST/expressions/expr.h"
#include "AST/expressions/number-const.h"
#include "AST/expressions/boolean-expr.h"
#include "AST/expressions/string-expr.h"
#include "AST/expressions/field.h"
#include "AST/expressions/function-call.h"
#include "AST/expressions/factor.h"
#include "AST/expressions/unary.h"
#include "AST/expressions/binary.h"
#include "AST/expressions/ternary.h"


#include "AST/declarations/declaration.h"
#include "AST/declarations/function-decl.h"
#include "AST/declarations/field-decl.h"
#include "AST/declarations/class-decl.h"
#include "AST/declarations/extension.h"

#include "AST/statements/statement.h"
#include "AST/statements/statements.h"
#include "AST/statements/if-stmt.h"
#include "AST/statements/empty-stmt.h"
#include "AST/statements/throw-stmt.h"
#include "AST/statements/return-stmt.h"
#include "AST/statements/break-stmt.h"
#include "AST/statements/continue-stmt.h"
#include "AST/statements/import-stmt.h"
#include "AST/statements/while-stmt.h"
#include "AST/statements/do-stmt.h"
#include "AST/statements/switch-stmt.h"
#include "AST/statements/for-stmt.h"
namespace parser {

    class Program {
        std::vector<std::shared_ptr<Statement>> statements;
        
		void ParseSingle();
    public:
		std::vector<std::shared_ptr<Declaration>> declarations;
		static std::shared_ptr<Program> Parse();
        void Gen();
    };

	std::shared_ptr<Program> Parse();

};

#endif
