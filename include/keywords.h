
#ifndef SYNTEX
#define SYNTEX

#define KEYWORDS(F)\
	F(string) F(number) F(bool)									\
	F(int) F(short) F(long) F(float) F(byte)					\
	F(uint) F(ushort) F(ulong) F(double)						\
	F(true) F(false) 											\
	F(if) F(else) F(elif) F(for) F(while)  F(repeat)			\
	F(switch) F(case) F(default)								\
	F(let) F(var) F(func) F(dfunc) F(kernal)					\
	F(return) F(continue) F(break) F(try) F(catch) F(throw)		\
	F(import) F(typedef) F(extension) F(operator) F(extern)		\
	F(struct) F(class) F(enum) F(interface)						\
	F(new) F(get) F(set) F(init) F(deinit)						\
	F(sizeof) F(print)											\
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

#define CHINESE(a) CR(a,L'\u2E80',L'\u2FD5')|| CR(a,L'\u3190', L'\u319f')|| CR(a,L'\u3400', L'\u4DBF')||CR(a,L'\u4E00',L'\u9FCC')||CR(a,L'\uF900',L'\uFAAD')


#endif