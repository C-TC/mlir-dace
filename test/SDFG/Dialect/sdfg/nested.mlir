// RUN: sdfg-opt %s | sdfg-opt | FileCheck %s

// CHECK: module
// CHECK: sdfg.sdfg 
// CHECK-SAME: {{@[a-zA-Z0-9_]*}}
sdfg.sdfg{entry=@state_0} @sdfg_0 {
    // CHECK: sdfg.state {{@[a-zA-Z0-9_]*}}
    sdfg.state @state_0{
        // CHECK: sdfg.sdfg
        // CHECK-SAME: {{@[a-zA-Z0-9_]*}}
        sdfg.sdfg{entry=@state_1} @sdfg_1 {
            // CHECK: sdfg.state {{@[a-zA-Z0-9_]*}}
            sdfg.state @state_1{

            }
        }
    }
} 
