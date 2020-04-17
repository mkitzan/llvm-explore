; RUN: opt -load ../build/libRedundantLoadPass.so -legacy-redundant-load -S %s  | FileCheck %s

define dso_local i32 @max(i32, i32) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = load i32, i32* %3, align 4
  %6 = load i32, i32* %4, align 4
  %7 = icmp slt i32 %5, %6
  br i1 %7, label %8, label %10

8:
  %9 = load i32, i32* %4, align 4
  store i32 %9, i32* %3, align 4
  br label %10

10:
  %11 = load i32, i32* %3, align 4
  ret i32 %11
}

; Verify all redundant loads are removed

; CHECK-LABEL: @max
; CHECK-NEXT:  [[REG_0:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  [[REG_1:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  store i32 [[ARG_0:%[0-9]+]], i32* [[REG_0]], align 4
; CHECK-NEXT:  store i32 [[ARG_1:%[0-9]+]], i32* [[REG_1]], align 4
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = icmp slt i32 [[ARG_0]], [[ARG_1]]
; CHECK-NEXT:  br i1 [[REG_2]], label %[[LABEL_0:[0-9]+]], label %[[LABEL_1:[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[LABEL_0]]
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = load i32, i32* [[REG_1]], align 4
; CHECK-NEXT:  store i32 [[REG_3]], i32* [[REG_0]], align 4
; CHECK-NEXT:  br label %[[LABEL_1]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[LABEL_1]]
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = load i32, i32* [[REG_0]], align 4
; CHECK-NEXT:  ret i32 [[REG_4]]
