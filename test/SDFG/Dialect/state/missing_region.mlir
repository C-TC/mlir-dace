// RUN: not sdfg-opt %s 2>&1 | FileCheck %s
// CHECK: expected '{' to begin a region

sdfg.sdfg{entry=@state_0} @sdfg_0 {
    sdfg.state @state_0
}
