; RUN: opt -load ../pass/libPrimitiveBranchPass.so -legacy-primitive-branch -S %s  | FileCheck %s

define dso_local i32 @min(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = icmp sgt i32 %0, %1
  br i1 %5, label %6, label %7

6:
  store i32 %1, i32* %3, align 4
  br label %7

7:
  %8 = load i32, i32* %3, align 4
  ret i32 %8
}

; Verify all primitive branches are removed

; CHECK-LABEL: @min
; CHECK-NEXT:  [[REG_0:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  [[REG_1:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  store i32 [[ARG_0:%[0-9]+]], i32* [[REG_0]], align 4
; CHECK-NEXT:  store i32 [[ARG_1:%[0-9]+]], i32* [[REG_1]], align 4
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = icmp sgt i32 [[ARG_0]], [[ARG_1]]
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = select i1 [[REG_2]], i32 [[ARG_1]], i32 [[ARG_0]]
; CHECK-NEXT:  ret i32 [[REG_3]]
