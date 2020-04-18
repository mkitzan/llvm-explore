; RUN: opt -load ../pass/libUnusedStorePass.so -legacy-unused-store -S %s  | FileCheck %s

define dso_local i32 @branchless_min(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = xor i32 %1, %0
  %6 = icmp sgt i32 %0, %1
  %7 = zext i1 %6 to i32
  %8 = sub nsw i32 0, %7
  %9 = and i32 %5, %8
  %10 = xor i32 %0, %9
  ret i32 %10
}

; Verify all unused stores are removed

; CHECK-LABEL: @branchless_min
; CHECK-NEXT:  [[REG_0:%[0-9]+]] = xor i32 [[ARG_1:%[0-9]+]], [[ARG_0:%[0-9]+]]
; CHECK-NEXT:  [[REG_1:%[0-9]+]] = icmp sgt i32 [[ARG_0]], [[ARG_1]]
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = zext i1 [[REG_1]] to i32
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = sub nsw i32 0, [[REG_2]]
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = and i32 [[REG_0]], [[REG_3]]
; CHECK-NEXT:  [[REG_5:%[0-9]+]] = xor i32 [[ARG_0]], [[REG_4]]
; CHECK-NEXT:  ret i32 [[REG_5]]
