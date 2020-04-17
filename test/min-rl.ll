
define dso_local i32 @min(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = icmp sgt i32 %0, %1
  br i1 %5, label %6, label %8

6:
  %7 = load i32, i32* %4, align 4
  store i32 %7, i32* %3, align 4
  br label %8

8:
  %9 = load i32, i32* %3, align 4
  ret i32 %9
}
