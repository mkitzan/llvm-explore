; RUN: opt -load ../pass/libUnusedStorePass.so -legacy-unused-store -S %s  | FileCheck %s

define dso_local i32 @max(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = icmp slt i32 %0, %1
  %6 = select i1 %5, i32 %1, i32 %0
  ret i32 %6
}

; Verify all unused stores are removed

; CHECK-LABEL: @max
; CHECK-NEXT:  [[REG_0:%[0-9]+]] = icmp slt i32 [[ARG_0:%[0-9]+]], [[ARG_1:%[0-9]+]]
; CHECK-NEXT:  [[REG_1:%[0-9]+]] = select i1 [[REG_0]], i32 [[ARG_1]], i32 [[ARG_0]]
; CHECK-NEXT:  ret i32 [[REG_1]]