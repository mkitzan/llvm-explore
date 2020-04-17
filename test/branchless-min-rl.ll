; RUN: opt -load ../build/libRedundantLoadPass.so -legacy-redundant-load -S %s  | FileCheck %s

define dso_local i32 @branchless_min(i32, i32) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = load i32, i32* %3, align 4
  %6 = load i32, i32* %4, align 4
  %7 = load i32, i32* %3, align 4
  %8 = xor i32 %6, %7
  %9 = load i32, i32* %3, align 4
  %10 = load i32, i32* %4, align 4
  %11 = icmp sgt i32 %9, %10
  %12 = zext i1 %11 to i32
  %13 = sub nsw i32 0, %12
  %14 = and i32 %8, %13
  %15 = xor i32 %5, %14
  ret i32 %15
}

; Verify all redundant loads are removed

; CHECK-LABEL: @branchless_min
; CHECK-NEXT:  [[REG_0:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  [[REG_1:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  store i32 [[ARG_0:%[0-9]+]], i32* [[REG_0]], align 4
; CHECK-NEXT:  store i32 [[ARG_1:%[0-9]+]], i32* [[REG_1]], align 4
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = xor i32 [[ARG_1]], [[ARG_0]]
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = icmp sgt i32 [[ARG_0]], [[ARG_1]]
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = zext i1 [[REG_3]] to i32
; CHECK-NEXT:  [[REG_5:%[0-9]+]] = sub nsw i32 0, [[REG_4]]
; CHECK-NEXT:  [[REG_6:%[0-9]+]] = and i32 [[REG_2]], [[REG_5]]
; CHECK-NEXT:  [[REG_7:%[0-9]+]] = xor i32 [[ARG_0]], [[REG_6]]
; CHECK-NEXT:  ret i32 [[REG_7]]
