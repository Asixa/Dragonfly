#ifndef FUNCTION_DECL_H
#define FUNCTION_DECL_H

#include "frontend/lexer.h"
#include "AST/declarations/declaration.h"
#include "namespace-decl.h"

#define MAKE_FLAGS_ENUM(TEnum, TUnder)                                                                                             \
TEnum  operator~  ( TEnum  a          ) { return static_cast<TEnum> (~static_cast<TUnder> (a)                           ); }  \
TEnum  operator|  ( TEnum  a, TEnum b ) { return static_cast<TEnum> ( static_cast<TUnder> (a) |  static_cast<TUnder>(b) ); }  \
TEnum  operator&  ( TEnum  a, TEnum b ) { return static_cast<TEnum> ( static_cast<TUnder> (a) &  static_cast<TUnder>(b) ); }  \
TEnum  operator^  ( TEnum  a, TEnum b ) { return static_cast<TEnum> ( static_cast<TUnder> (a) ^  static_cast<TUnder>(b) ); }  \
TEnum& operator|= ( TEnum& a, TEnum b ) { a = static_cast<TEnum>(static_cast<TUnder>(a) | static_cast<TUnder>(b) ); return a; }  \
TEnum& operator&= ( TEnum& a, TEnum b ) { a = static_cast<TEnum>(static_cast<TUnder>(a) & static_cast<TUnder>(b) ); return a; }  \
TEnum& operator^= ( TEnum& a, TEnum b ) { a = static_cast<TEnum>(static_cast<TUnder>(a) ^ static_cast<TUnder>(b) ); return a; }

namespace AST {
  
	namespace decl {
		class FieldDecl;
		class FieldList;
		/**
		 * \brief class for matching function's parameters' declaration ; \n
		 * `` NAME:TYPE,NAME:TYPE...``\n
		 */
		// class FuncParam {
		// public:
		// 	int size = 0;
		// 	bool is_var_arg = false;
		// 	std::vector<std::shared_ptr<FieldDecl>> fields;
		//     // std::vector<std::string> names;
		// 	// std::vector<std::shared_ptr<AST::Type>> types;
		// 	std::vector<int>generic_id;
		// 	static std::shared_ptr<FuncParam> Parse();
		// 	FuncParam() {
		// 		size = 0;
		// 	}
		// 	FuncParam(const std::shared_ptr<FuncParam> copy) {
		// 		*this = *copy;
		// 		// for (auto i : copy->types)types.push_back(i);
		// 	}
		// 	std::string ToString();
		// };

		// enum FunctionFlags
		// {
		// 	kNone = 0,
		//
		// 	kGenericTemplate = 32,
		// 	kGenericInstance = 64
		// };
		// MAKE_FLAGS_ENUM(FunctionFlags, FunctionFlags)

		/**
		 * \brief class for matching function definition. \n
		 * ``func NAME (FuncParam) {...} ``\n
		 * ``dfunc NAME (FuncParam) {...} ``\n
		 * ``kernel NAME (FuncParam) {...}`` \n
		 * ``func CLASS::NAME (FuncParam) {...}`` \n
		 */
		//
	

		class FunctionDecl final : public Declaration {
			std::shared_ptr<NestedName> nested_name;  //Deprecated after called AnalysisHeader
		public:

			int keyword;
			bool is_generic_template = false;   //if true, then generic_instance_info must be null
			std::shared_ptr<FieldList> generic_instance_info; // only Instantiated function has this info
			
			// bool differentiable = false,
			// 	kernal = false,
			// 	is_extern = false,
			// 	extension = false;
			// int generic_return = -1;
			// bool is_template = false;
			//
			
			std::string name;
			// std::string func_postfix;
			// std::string header_name;
			// std::string full_name;
			std::shared_ptr<AST::Type> return_type;
			// llvm::StructType* parent_type = nullptr;
            // std::shared_ptr<AST::Type>parent_type = nullptr;

			// std::vector<int>generic_arguments;
			std::shared_ptr<FieldList> generic;
			std::shared_ptr<FieldList> args;
			std::shared_ptr<Statement> statements;





			FunctionDecl();
			FunctionDecl(std::shared_ptr < FunctionDecl> copy);


			static std::shared_ptr<FunctionDecl> CreateInit(const std::shared_ptr<FieldList>& init_field);

			// void SetInternal(llvm::StructType* type);

			void AnalysisHeader(std::shared_ptr<DFContext>) override;
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
			void GenHeader(std::shared_ptr<DFContext>) override;

			void PassGeneric(std::shared_ptr<FieldList> generic_instance, std::shared_ptr<FieldList> generic_decl=nullptr);
			std::shared_ptr<FunctionDecl> InstantiateTemplate(std::shared_ptr<DFContext> context, std::shared_ptr<FieldList> param);

			static std::shared_ptr<FunctionDecl> Parse(bool ext = false);
			std::string GetName() override;
		};
	}
}
#endif