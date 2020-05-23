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

#ifndef KEYWORDS_H
#define KEYWORDS_H

#define KEYWORDS(F)\
	F(string) F(number) F(bool)									\
	F(int) F(short) F(long) F(float) F(byte) F(void)			\
	F(uint) F(ushort) F(ulong) F(double)						\
	F(true) F(false) F(null)									\
	F(if) F(else) F(elif) F(for) F(while)  F(do)			    \
	F(switch) F(case) F(default)								\
	F(let) F(var) F(func) F(dfunc) F(kernal)					\
	F(return) F(continue) F(break) F(try) F(catch) F(throw)		\
	F(import) F(typedef) F(extension) F(operator) F(extern)		\
	F(struct) F(class) F(enum) F(interface) F(namespace)						\
	F(new) F(get) F(set) F(init) F(delete)						\
	F(sizeof) 											\
	F(in) F(from) F(to) 

#define OPERATORS(F)\
	F(Or, "||") F(And, "&&") 															\
	F(Eq, "==") F(Ne, "!=") F(Le, "<=") F(Ge, ">=")										\
	F(Shl, "<<") F(Shr, ">>")															\
	F(Inc, "++") F(Dec, "--")															\
	F(Arrow,"=>")																		\
	F(AddAgn, "+=") F(SubAgn, "-=") F(MulAgn, "*=") F(DivAgn, "/=") F(ModAgn, "%=")		\
	F(BAndAgn, "&=") F(BXORAgn, "^=") F(BORAgn, "|=") F(ShlAgn, "<<=") F(ShrAgn, ">>=")	\
	

// operators that only appears alone
#define SINGEL_OP(F)					\
		F('~') F(';')					\
		F('{') F('}')					\
		F('(') F(')')					\
		F('[') F(']')					\
		F(',') F('?')					\
		F('.') F(':')

#define SPECIAL_OP										\
else if (peek == '=') {									\
if (*src == '=') { src++; token = new Token(Eq); }		\
else if(*src=='>') {src++; token = new Token(Arrow); }	\
else { token = new Token('='); }						\
return;}


// operators that only appears alone or before '='
#define ASSGIN_OP(F)					\
		F('=',Eq)						\
		F('!',Ne)						\
		F('*',MulAgn)					\
		F('%',ModAgn)					\
		F('^',BXORAgn)					\

// operators that can appears two together or follow a '='
#define ASSGIN_OR_REPEAT_OP(F)			\
		F('+', Inc,AddAgn)				\
		F('-', Dec, SubAgn)				\
		F('&', And, BAndAgn)			\
		F('|', Or, BORAgn)

// operators that can appears two together and follow a '='  
#define ASSGIN_AND_REPEAT_OP(F)			\
		F('<', Le, Shl, ShlAgn)			\
		F('>', Ge, Shr, ShrAgn)

#define OTHER_KEYWORDS Num = 128,Str, Id, NewLine,

enum {
	OTHER_KEYWORDS
#define ENUM(NAME) K_##NAME,
	KEYWORDS(ENUM)
#define ENUM(NAME,_) NAME,
	OPERATORS(ENUM)
#undef ENUM
};
#endif