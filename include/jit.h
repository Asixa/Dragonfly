#ifndef JUST_IN_TIME
#define JUST_IN_TIME

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include <memory>
using namespace llvm;
using namespace orc;
class JIT {
private:
	ExecutionSession ES;
	RTDyldObjectLinkingLayer ObjectLayer;
	IRCompileLayer CompileLayer;

	DataLayout DL;
	MangleAndInterner Mangle;
	ThreadSafeContext Ctx;

	JITDylib& MainJD;

public:
	JIT(JITTargetMachineBuilder JTMB, DataLayout DL):
		ObjectLayer(ES,[]() { return std::make_unique<SectionMemoryManager>(); }),
		CompileLayer(ES, ObjectLayer,std::make_unique<ConcurrentIRCompiler>(std::move(JTMB))),
		DL(std::move(DL)), Mangle(ES, this->DL),
		Ctx(std::make_unique<LLVMContext>()),
		MainJD(ES.createJITDylib("<main>")) {
		MainJD.addGenerator(cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix())));
	}

	static Expected<std::unique_ptr<JIT>> Create() {
		auto jtmb = JITTargetMachineBuilder::detectHost();
		if (!jtmb)return jtmb.takeError();
		auto DL = jtmb->getDefaultDataLayoutForTarget();
		if (!DL)
			return DL.takeError();
		return std::make_unique<JIT>(std::move(*jtmb), std::move(*DL));
	}

	const DataLayout& GetDataLayout() const { return DL; }

	LLVMContext& GetContext() { return *Ctx.getContext(); }

	Error AddModule(std::unique_ptr<Module> m) {
		return CompileLayer.add(MainJD, ThreadSafeModule(std::move(m), Ctx));
	}

	Expected<JITEvaluatedSymbol> Lookup(StringRef Name) {
		return ES.lookup({ &MainJD }, Mangle(Name.str()));
	}
};

#endif
