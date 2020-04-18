; RUN: opt -load ../pass/libUnusedStorePass.so -legacy-unused-store -S %s  | FileCheck %s

define dso_local i32 @branchless_max(i32 %0, i32 %1) #0 {
  %3 = xor i32 %1, %0
  %4 = icmp slt i32 %0, %1
  %5 = zext i1 %4 to i32
  %6 = sub nsw i32 0, %5
  %7 = and i32 %3, %6
  %8 = xor i32 %0, %7
  ret i32 %8
}

; Verify all unused stores are removed

; CHECK-LABEL: @branchless_max
; CHECK-NEXT:  [[REG_0:%[0-9]+]] = xor i32 [[ARG_1:%[0-9]+]], [[ARG_0:%[0-9]+]]
; CHECK-NEXT:  [[REG_1:%[0-9]+]] = icmp slt i32 [[ARG_0]], [[ARG_1]]
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = zext i1 [[REG_1]] to i32
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = sub nsw i32 0, [[REG_2]]
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = and i32 [[REG_0]], [[REG_3]]
; CHECK-NEXT:  [[REG_5:%[0-9]+]] = xor i32 [[ARG_0]], [[REG_4]]
; CHECK-NEXT:  ret i32 [[REG_5]]
