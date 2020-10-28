#ifndef NAMESPACE_DECL_H
#define NAMESPACE_DECL_H
#include "declaration.h"

namespace AST {
	namespace decl {
		class Namespace final : public Declaration {
		public:
			std::string name;
			std::vector<std::shared_ptr<Declaration>> declarations;
			void AnalysisHeader(std::shared_ptr<DFContext>) override;
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
			void GenHeader(std::shared_ptr<DFContext>) override;
			static std::shared_ptr<Namespace>Parse();
			void ParseSingle();
		};


		class Name {

		public:
			enum { kClass, kFunction, kNamespace };
			int type = kClass;
			std::vector<std::string>names;
			static std::shared_ptr<Name>Parse(int ty);

			std::string GetFunctionName();
			std::string GetClassName();
			std::string GetNamespace();
			std::string GetFullName();
			std::string GetFullNameWithoutFunc();
			void Set(std::string s);
			void Verify();
		};
	}
}
#endif
