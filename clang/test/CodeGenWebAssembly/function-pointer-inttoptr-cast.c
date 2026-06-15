// REQUIRES: webassembly-registered-target
// RUN: %clang_cc1 -triple wasm32-unknown-unknown -emit-llvm -O0 -fwasm-fix-function-bitcasts -o - %s | FileCheck %s

typedef __UINTPTR_TYPE__ uintptr_t;
typedef unsigned long long wide_uint_t;

typedef void (*OneArgFunc)(void *);
typedef void (*TwoArgFunc)(void *, void *);

// CHECK: @__wasm_runtime_wrapper_vi_to_vii_fptr = internal thread_local global ptr null

void my_one_arg_func(void *ptr) {}

void library_function(TwoArgFunc func, void *data) {
  func(data, (void *)0);
}

// CHECK-LABEL: @direct_roundtrip
// CHECK: call void @library_function(ptr noundef @__my_one_arg_func_vii, ptr noundef %{{.*}})
void direct_roundtrip(void *data) {
  library_function((TwoArgFunc)(uintptr_t)my_one_arg_func, data);
}

// CHECK-LABEL: @direct_roundtrip_with_int_cast
// CHECK: call void @library_function(ptr noundef @__my_one_arg_func_vii, ptr noundef %{{.*}})
void direct_roundtrip_with_int_cast(void *data) {
  library_function((TwoArgFunc)(wide_uint_t)(uintptr_t)my_one_arg_func, data);
}

// CHECK-LABEL: @runtime_roundtrip
// CHECK: ptrtoint ptr %{{.*}} to i32
// CHECK: store ptr %{{.*}}, ptr @__wasm_runtime_wrapper_vi_to_vii_fptr
// CHECK: call void @library_function(ptr noundef @__wasm_runtime_wrapper_vi_to_vii, ptr noundef %{{.*}})
void runtime_roundtrip(OneArgFunc fp, void *data) {
  library_function((TwoArgFunc)(uintptr_t)fp, data);
}

// CHECK-LABEL: @runtime_roundtrip_with_int_cast
// CHECK: ptrtoint ptr %{{.*}} to i32
// CHECK: zext i32 %{{.*}} to i64
// CHECK: trunc i64 %{{.*}} to i32
// CHECK: store ptr %{{.*}}, ptr @__wasm_runtime_wrapper_vi_to_vii_fptr
// CHECK: call void @library_function(ptr noundef @__wasm_runtime_wrapper_vi_to_vii, ptr noundef %{{.*}})
void runtime_roundtrip_with_int_cast(OneArgFunc fp, void *data) {
  library_function((TwoArgFunc)(wide_uint_t)(uintptr_t)fp, data);
}

// CHECK-LABEL: @nonrecoverable_roundtrip
// CHECK: ptrtoint ptr %{{.*}} to i32
// CHECK: add i32 %{{.*}}, 1
// CHECK: inttoptr i32 %{{.*}} to ptr
// CHECK: call void @library_function(ptr noundef %{{.*}}, ptr noundef %{{.*}})
void nonrecoverable_roundtrip(OneArgFunc fp, void *data) {
  library_function((TwoArgFunc)((uintptr_t)fp + 1), data);
}
