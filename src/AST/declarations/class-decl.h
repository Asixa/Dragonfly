#ifndef CLASS_DECL_H
#define CLASS_DECL_H
#include "declaration.h"
#include "AST/expressions/expr.h"
#include "frontend/lexer.h"
#include "function-decl.h"

namespace AST {
	namespace decl {
		class FieldDecl {
		public:
            std::string name;
            std::shared_ptr<AST::Type> type;
			bool used_generic;
			// static std::shared_ptr<FieldDecl> Parse();
			FieldDecl(std::string name, std::shared_ptr<AST::Type> type) :name(name), type(type),used_generic(false) {}

		};


        class FieldList {
        public:
			std::vector<FieldDecl>fields;
			std::string ToGenericString();

        };

     

        /**
		 * \brief  class for matching class declaration.\n
		 * ``class  A{...}``\n
		 * ``class  A<T>{...}``\n
		 * ``struct A{...}``\n
		 * ``interface A{...}``
		 */
		class ClassDecl final : public Declaration {
		public:
			enum { kInterface, kClass, kStruct };
			std::shared_ptr<FieldList> generic_info;
			int category = kClass;      ///< represents the type of this struct decalartion,[class] or [struct] or [interface]. default is [class].
			bool is_template = false;   ///< True if this class is 'Generic'.
			// bool one_line = false;      ///< True if this class contains only one statement.
			// std::string full_name;      ///< name as NAMESPACE::CLASS::CLASS, usually managled.
			std::string name;           ///< name as CLASS, without the namespace etc.
			// std::string postfix;           ///< name as CLASS, without the namespace etc.

			std::shared_ptr<AST::Type> base_type;                               ///< return type of this class.
			//std::vector<std::string> fields;                        ///< a list contains all fields'names, mapped with types
			//std::vector<std::shared_ptr<AST::Type>> types;                           ///< a list contains all fields' type, mapped with fields
			std::vector<std::shared_ptr<FieldDecl>>fields;

			std::vector<std::shared_ptr<AST::Type>> interfaces;                      ///< a list contains all interfaces' type, empty if there no interface used.
			std::vector<std::shared_ptr<Declaration>> declarations; ///< a list contains all sub declaration like enums, sub classes.
			std::vector<std::shared_ptr<FunctionDecl>> functions;   ///< a list contains all sub functions
			std::vector<std::shared_ptr<FunctionDecl>> constructor; ///< a list contains all constructors, \n constructors overloading is allowed, therefore it is a list.
			std::shared_ptr<GenericParam> generic;                  ///< a struct represents all generic types, ``.names`` is empty if this is not a generic class. \n eg. ``class A<T,V>{}``. then this list is {T,V}
			std::shared_ptr<FunctionDecl> destructor;               ///< a decl represents the destructor. null if there is no destuctor.


			ClassDecl() { base_type = std::make_shared<AST::Type>(); }
            explicit ClassDecl(ClassDecl*);


		    static std::shared_ptr<ClassDecl> Parse(int type = kClass);

			void Gen(std::shared_ptr<DFContext>) override;
			void GenHeader(std::shared_ptr<DFContext>) override;
			void Analysis(std::shared_ptr<DFContext>) override;
			void AnalysisHeader(std::shared_ptr<DFContext>) override;
			std::string GetName() override;

            /** 
			 * \brief Instatiate an class from the Template.
			 * \param context the DFContext object that requried to generate IR.
			 * \param param the generic types will be replaced by this list of types after instantiation.
			 */
			void InstantiateTemplate(std::shared_ptr<DFContext> context, std::shared_ptr<FieldList> param);

			std::shared_ptr<CustomType>GetType();
		};

	
	}
}
#endif