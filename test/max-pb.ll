
define dso_local i32 @max(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  %4 = icmp slt i32 %0, %1
  br i1 %4, label %5, label %6

5:
  store i32 %1, i32* %3, align 4
  br label %6

6:
  %7 = load i32, i32* %3, align 4
  ret i32 %7
}
