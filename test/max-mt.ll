; RUN: opt -load ../build/libMemoryTransferPass.so -legacy-memory-transfer -S %s  | FileCheck %s

define dso_local i32 @max(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = icmp slt i32 %0, %1
  br i1 %5, label %6, label %8

6:
  %7 = load i32, i32* %4, align 4
  store i32 %7, i32* %3, align 4
  br label %8

8:
  %9 = load i32, i32* %3, align 4
  ret i32 %9
}

; Verify all memory transfers are removed

; CHECK-LABEL: @max
; CHECK-NEXT:  [[REG_0:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  [[REG_1:%[0-9]+]] = alloca i32, align 4
; CHECK-NEXT:  store i32 [[ARG_0:%[0-9]+]], i32* [[REG_0]], align 4
; CHECK-NEXT:  store i32 [[ARG_1:%[0-9]+]], i32* [[REG_1]], align 4
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = icmp slt i32 [[ARG_0]], [[ARG_1]]
; CHECK-NEXT:  br i1 [[REG_2]], label %[[LABEL_0:[0-9]+]], label %[[LABEL_1:[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[LABEL_0]]
; CHECK-NEXT:  store i32 [[ARG_1]], i32* [[REG_0]], align 4
; CHECK-NEXT:  br label %[[LABEL_1]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[LABEL_1]]
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = load i32, i32* [[REG_0]], align 4
; CHECK-NEXT:  ret i32 [[REG_4]]
