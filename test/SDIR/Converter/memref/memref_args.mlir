// RUN: sdir-opt --convert-to-sdir %s
func private @kernel_2mm(%1: index, %arg6: memref<?x900xi32>){
  %c0 = arith.constant 0 : index
  %8 = memref.load %arg6[%c0, %1] : memref<?x900xi32>
  %c2 = arith.addi %8, %8 : i32
  memref.store %c2, %arg6[%1, %1] : memref<?x900xi32>
  return
}
