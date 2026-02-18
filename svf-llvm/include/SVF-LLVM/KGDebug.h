#ifndef KGDEBUG_H
#define KGDEBUG_H

#include <string>

// Forward declarations — avoid pulling in heavy LLVM headers in every TU.
namespace llvm {
  class LLVMContext;
  class PointerType;
  class Type;
}

// Lightweight breadcrumb tracking for crash diagnostics.
// Updated at key points in the SVF pipeline; read by the SIGABRT handler in KallGraph.cpp.
namespace KGDebug {
  extern thread_local const char* phase;
  extern thread_local std::string moduleName;
  extern thread_local std::string funcName;
  extern thread_local std::string instDesc;
  extern thread_local unsigned long instCounter;   // total instructions visited so far

  // ---------- safe pointer-type helpers ----------
  // These always use the *typed-pointer* LLVM APIs (PointerType::get(Type*,AS))
  // so they work correctly when opaque pointers are disabled.
  // Each prints detailed diagnostics if something looks wrong.

  /// Create an i8* (address-space 0) pointer type — safe replacement for
  /// PointerType::getUnqual(LLVMContext&).
  llvm::PointerType* safeGetUnqualPtrType(llvm::LLVMContext &ctx,
                                           const char *callsite = "unknown");

  /// Create a typed pointer: elemTy* in the given address space.
  /// Safe replacement for PointerType::get(Type*, unsigned).
  llvm::PointerType* safeGetPtrType(llvm::Type *elemTy,
                                     unsigned addrSpace,
                                     const char *callsite = "unknown");

  /// Safe wrapper around Type::getPointerTo(AS).
  /// Prints diagnostics if the type is null.
  llvm::Type* safeGetPointerTo(llvm::Type *ty,
                                unsigned addrSpace,
                                const char *callsite = "unknown");

  // ---------- signal handler (defined in KallGraph.cpp) ----------
  void install_handler();
}

#endif // KGDEBUG_H
