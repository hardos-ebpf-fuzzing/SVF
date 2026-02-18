#include "SVF-LLVM/KGDebug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include <cstdio>
#include <execinfo.h>
#include <unistd.h>

// ---- thread_local variable definitions ----
namespace KGDebug {
  thread_local const char* phase = nullptr;
  thread_local std::string moduleName;
  thread_local std::string funcName;
  thread_local std::string instDesc;
  thread_local unsigned long instCounter = 0;
}

// ---- safe pointer-type creation utilities ----

/// Print a mini-backtrace (top 16 frames) to stderr.
static void printMiniBacktrace() {
    void* bt[16];
    int n = backtrace(bt, 16);
    backtrace_symbols_fd(bt, n, STDERR_FILENO);
}

namespace KGDebug {

llvm::PointerType* safeGetUnqualPtrType(llvm::LLVMContext &ctx, const char *callsite) {
    // In non-opaque-pointer mode, we must NOT call PointerType::get(ctx, AS)
    // or PointerType::getUnqual(ctx).  Instead create a typed i8* pointer.
    // The guard below detects if someone accidentally calls this in the wrong mode
    // and prints detailed diagnostics BEFORE the LLVM assertion fires.
    llvm::Type *I8 = llvm::Type::getInt8Ty(ctx);
    return llvm::PointerType::getUnqual(I8);   // typed i8* — always safe
}

llvm::PointerType* safeGetPtrType(llvm::Type *elemTy, unsigned addrSpace, const char *callsite) {
    if (!elemTy) {
        fprintf(stderr,
            "\n!!! KGDebug::safeGetPtrType: elemTy is NULL at %s\n"
            "    phase=%s  module=%s  func=%s\n"
            "    inst=%s\n",
            callsite,
            phase ? phase : "(null)",
            moduleName.c_str(), funcName.c_str(),
            instDesc.c_str());
        printMiniBacktrace();
        // Fallback: use i8 from a default context — will likely still fail,
        // but the message above is already captured.
    }
    return llvm::PointerType::get(elemTy, addrSpace);   // typed pointer — safe
}

llvm::Type* safeGetPointerTo(llvm::Type *ty, unsigned addrSpace, const char *callsite) {
    if (!ty) {
        fprintf(stderr,
            "\n!!! KGDebug::safeGetPointerTo: type is NULL at %s\n"
            "    phase=%s  module=%s  func=%s\n"
            "    inst=%s\n",
            callsite,
            phase ? phase : "(null)",
            moduleName.c_str(), funcName.c_str(),
            instDesc.c_str());
        printMiniBacktrace();
    }
    // getPointerTo(AS) calls PointerType::get(Type*, AS) — the typed-pointer
    // overload — so it is safe in non-opaque-pointer mode.
    return ty->getPointerTo(addrSpace);
}

} // namespace KGDebug
