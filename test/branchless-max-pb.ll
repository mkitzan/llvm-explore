
define dso_local i32 @branchless_max(i32 %0, i32 %1) #0 {
  %3 = xor i32 %1, %0
  %4 = icmp slt i32 %0, %1
  %5 = zext i1 %4 to i32
  %6 = sub nsw i32 0, %5
  %7 = and i32 %3, %6
  %8 = xor i32 %0, %7
  ret i32 %8
}
