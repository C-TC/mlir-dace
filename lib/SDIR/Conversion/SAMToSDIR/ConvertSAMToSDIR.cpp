#include "SDIR/Conversion/SAMToSDIR/PassDetail.h"
#include "SDIR/Conversion/SAMToSDIR/Passes.h"
#include "SDIR/Dialect/Dialect.h"
#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/SCF.h"

using namespace mlir;
using namespace sdir;
using namespace conversion;

struct SDIRTarget : public ConversionTarget {
  SDIRTarget(MLIRContext &ctx) : ConversionTarget(ctx) {
    // Every Op in the SDIR Dialect is legal
    addLegalDialect<SDIRDialect>();
    // Implicit top level module operation is legal
    // if it only contains a single SDFGNode or is empty
    // TODO: Add checks
    addDynamicallyLegalOp<ModuleOp>([](ModuleOp op) {
      return true;
      //(op.getOps().empty() || !op.getOps<SDFGNode>().empty());
    });
    // All other operations are illegal
    // markUnknownOpDynamicallyLegal([](Operation *op) { return false; });
  }
};

class FuncToSDFG : public OpRewritePattern<FuncOp> {
public:
  using OpRewritePattern<FuncOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(FuncOp op,
                                PatternRewriter &rewriter) const override {
    SDFGNode sdfg = SDFGNode::create(rewriter, op.getLoc(), op.getType());
    StateNode state = StateNode::create(rewriter, op.getLoc());
    sdfg.addEntryState(state);
    // TODO: Use rewriter
    state.body().takeBody(op.body());
    rewriter.eraseOp(op);
    return success();
  }
};

class TaskletTerminator : public OpRewritePattern<mlir::ReturnOp> {
public:
  using OpRewritePattern<mlir::ReturnOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(mlir::ReturnOp op,
                                PatternRewriter &rewriter) const override {
    if (TaskletNode tn = dyn_cast<TaskletNode>(op->getParentOp())) {
      sdir::ReturnOp::create(rewriter, op.getLoc(), op.getOperands());
      rewriter.eraseOp(op);
      return success();
    }

    return failure();
  }
};

class OpToTasklet : public RewritePattern {
public:
  OpToTasklet(PatternBenefit benefit, MLIRContext *context)
      : RewritePattern(MatchAnyOpTypeTag(), benefit, context) {}

  LogicalResult matchAndRewrite(Operation *op,
                                PatternRewriter &rewriter) const override {
    if (mlir::ReturnOp rop = dyn_cast<mlir::ReturnOp>(op)) {
      rewriter.eraseOp(op);
      return success();
    }

    // TODO: Check if there is a proper way of doing this
    if (op->getDialect()->getNamespace() == "arith" ||
        op->getDialect()->getNamespace() == "math") {
      if (TaskletNode task = dyn_cast<TaskletNode>(op->getParentOp())) {
        // Operation already in a tasklet
        return failure();
      }

      FunctionType ft = rewriter.getFunctionType(op->getOperandTypes(), {});
      TaskletNode task = TaskletNode::create(rewriter, op->getLoc(), ft);
      // rewriter.clone(*op);
      rewriter.eraseOp(op);

      // sdir::ReturnOp::create(rewriter, op->getLoc(), op->getResults());
      sdir::ReturnOp::create(rewriter, op->getLoc(), {});
      return success();
    }

    return failure();
  }
};

void populateSAMToSDIRConversionPatterns(RewritePatternSet &patterns) {
  MLIRContext *ctxt = patterns.getContext();
  patterns.add<FuncToSDFG>(ctxt);
  patterns.add<TaskletTerminator>(ctxt);
  patterns.add<OpToTasklet>(1, ctxt);
}

namespace {
struct SAMToSDIRPass : public SAMToSDIRPassBase<SAMToSDIRPass> {
  void runOnOperation() override;
};
} // namespace

void SAMToSDIRPass::runOnOperation() {
  // NOTE: Maybe change to a pass working on funcs?
  ModuleOp module = getOperation();

  RewritePatternSet patterns(&getContext());
  populateSAMToSDIRConversionPatterns(patterns);

  SDIRTarget target(getContext());
  if (failed(applyPartialConversion(module, target, std::move(patterns))))
    signalPassFailure();
}

std::unique_ptr<Pass> conversion::createSAMToSDIRPass() {
  return std::make_unique<SAMToSDIRPass>();
}
