#ifndef SDIR_TaskletOps
#define SDIR_TaskletOps

//===----------------------------------------------------------------------===//
// ReturnOp
//===----------------------------------------------------------------------===//

def SDIR_ReturnOp : SDIR_Op<"return", [
    HasParent<"TaskletNode">,
    Terminator
]> {
    let summary = "return operation";
    let description = [{
        The "return" operation represents a return operation within a function.
        The operation takes an optional operand and produces no results.
        The operand type must match the signature of the function that contains
        the operation. 
        For example:

        ```mlir
        func @foo() -> tensor<2xf64> {
            ...
            sdir.return %0 : tensor<2xf64>
        }
        ```
    }];

    let arguments = (ins Variadic<AnyType>:$input);
}

//===----------------------------------------------------------------------===//
// CallOp
//===----------------------------------------------------------------------===//

def SDIR_CallOp : SDIR_Op<"call", [
    ParentOneOf<["StateNode", "MapNode", "ConsumeNode"]>,
    CallOpInterface, 
    MemRefsNormalizable,
    DeclareOpInterfaceMethods<SymbolUserOpInterface>
]> {
    let summary = "call operation";
    let description = [{
        The `call` operation represents a direct call to a tasklet function that 
        is within the same symbol scope as the call. The operands and result 
        types of the call must match the specified function type. 
        For example:

        ```mlir
        %2 = sdir.call @add(%0, %1) : (f32, f32) -> f32
        ```
    }];

    let arguments = (ins FlatSymbolRefAttr:$callee, 
                        Variadic<AnyType>:$operands);
    let results = (outs Variadic<AnyType>);

    let extraClassDeclaration = [{
        StateNode getParentState();
        TaskletNode getTasklet();
        SDFGNode getSDFG();
        bool callsTasklet();
        StringRef getCallee() { return callee(); }

        FunctionType getCalleeType() {
            return FunctionType::get(getContext(), getOperandTypes(), 
                                    getResultTypes());
        }

        /// Get the argument operands to the called function.
        operand_range getArgOperands() {
            return {arg_operand_begin(), arg_operand_end()};
        }

        operand_iterator arg_operand_begin() { return operand_begin(); }
        operand_iterator arg_operand_end() { return operand_end(); }

        /// Return the callee of this operation.
        CallInterfaceCallable getCallableForCallee() {
            return (*this)->getAttrOfType<SymbolRefAttr>("callee");
        }
    }];
}

//===----------------------------------------------------------------------===//
// LibCallOp
//===----------------------------------------------------------------------===//

def SDIR_LibCallOp : SDIR_Op<"libcall", [
    ParentOneOf<["StateNode", "MapNode", "ConsumeNode"]>,
    CallOpInterface
]> {
    let summary = "library call operation";
    let description = [{
        The `libcall` operation represents a direct call to a library function. 
        For example:

        ```mlir
        %2 = sdir.libcall "dace.libraries.blas.nodes.Dot" (%0, %1) : (f32, f32) -> f32
        ```
    }];

    let arguments = (ins StrAttr:$callee, Variadic<AnyType>:$operands);
    let results = (outs Variadic<AnyType>);

    let extraClassDeclaration = [{
        operand_range getArgOperands() {
            return {operand_begin(), operand_end()};
        }

        CallInterfaceCallable getCallableForCallee() {
            return (*this)->getAttrOfType<SymbolRefAttr>("callee");
        }
    }];
}

#endif // SDIR_TaskletOps
