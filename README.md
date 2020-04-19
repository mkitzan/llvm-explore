# LLVM Explore

**Fisher Price Baby's First: LLVM Middle-end Optimization Pipeline**

This repo contains work and notes created while studying LLVM compiler infrastructure in preparation for compiler engineer job and onsite interview.

The boilerplate templates for the passes, cmake files, and lit config files were taken from [llvm-tutor](https://github.com/banach-space/llvm-tutor). In addition to diving into the [LLVM Docs](http://llvm.org/doxygen/), I referenced heavily two talks from the 2019 LLVM Developer's Meeting: [Getting Started with LLVM](https://www.youtube.com/watch?v=3QQuhL-dSys) and [Writing and LLVM Pass 101](https://www.youtube.com/watch?v=ar7cJl2aBuU).

## Pipeline Overview

This is a mini-pipeline to optimize four simple C functions:

-	[`int min(int value, int min)`](https://github.com/mkitzan/llvm-explore/blob/master/input/c/min.c)
-	[`int max(int value, int max)`](https://github.com/mkitzan/llvm-explore/blob/master/input/c/max.c)
-	[`int branchless_min(int value, int min)`](https://github.com/mkitzan/llvm-explore/blob/master/input/c/branchless_min.c)
-	[`int branchless_max(int value, int max)`](https://github.com/mkitzan/llvm-explore/blob/master/input/c/branchless_max.c)

The un-optimized LLVM IR for these functions were riddled with unnecessary memory accesses.
The goal of the pipeline is to eliminate all the memory accesses and collapse primitive conditional branches into `select` IR instructions. The final output of the pipeline should be very close to the emitted IR when the functions are compiled with an optimization flag greater than 0.

The pipeline is composed of four passes where each optimization pass sets up the IR for the next optimization pass.
The four passes are the following and are meant to execute in the following order:

-	[`RedundantLoadPass`](https://github.com/mkitzan/llvm-explore/blob/master/pass/RedundantLoadPass.cpp)
-	[`MemoryTransferPass`](https://github.com/mkitzan/llvm-explore/blob/master/pass/MemoryTransferPass.cpp)
-	[`PrimitiveBranchPass`](https://github.com/mkitzan/llvm-explore/blob/master/pass/PrimitiveBranchPass.cpp)
-	[`UnusedStorePass`](https://github.com/mkitzan/llvm-explore/blob/master/pass/UnusedStorePass.cpp)

After executing the pipeline, the IR output of `min` and `max` matches the IR output of Clang 9 with optimization enabled. However, `branchless_min` and `branchless_max` do not match the optimized output of Clang 9. The branchless functions use "clever" tricks to eliminate branching at the C level which obfuscates the actual behavior of the code. This obfuscation makes it harder to pattern match and optimize as a result.

A full `lit` test suite exists for the passes in the `test` subdirectory. The test suite must be run from the `test` subdirectory created in the cmake build directory.

## Redundant Load Pass

In the un-optimized IR from `clang-9`, the following pattern is pervasive within basic blocks of the three functions:

```llvm
%3 = alloca i32, align 4
...
store i32 %0, i32* %3, align 4
...
%5 = load i32, i32* %3, align 4
...
%7 = icmp sgt i32 %5, %6
```

Memory is allocated at the top of the basic block, and a virtual register is stored at the memory only to be loaded back into a new virtual register which is then used.
Where this pattern occurs within a basic block the `load` is redundant.
The users of the `load`'s def can be swapped to use the def of the value stored.
We can't make any assumption yet about removing the `store` and `alloca` themselves, because another basic block may `load` the value.

`RedundantLoadPass` implements this optimization by caching the `ptr` and most recently stored `value` at `ptr` into a map, and when a `load` opcode is found the map is searched. If the `load`'s `ptr` is cached, then the `load` can be elided with the `value` known to be stored at the `ptr`.

After the `RedundantLoadPass` is run the all previous instances of the target pattern will be replaced to look like the following:

```llvm
%3 = alloca i32, align 4
...
store i32 %0, i32* %3, align 4
...
%5 = icmp sgt i32 %0, %1
```

## Memory Transfer Pass

In the `max` and `min` IR functions, there exists a pattern where a virtual register's value is transferred through memory to a basic block with a single predecessor basic block. The pattern looks like the following in IR:

```llvm
  store i32 %1, i32* %4, align 4
  ...
  br i1 %5, label %6, label %8

6:
  %7 = load i32, i32* %4, align 4  
  store i32 %7, i32* %3, align 4
```

If the basic block who the data is transferred to had multiple predecessors then the value could be derived from either predecessor. Because the value is transferred to a basic block with a single predecessor, the transferred value can be deduced from the `store` in the parent basic block.

`MemoryTransferPass` implements this optimization by caching the `ptr` and most recently stored `value` at `ptr` into a map, and when a `br` opcode is found the branch successors with a single predecessor are searched. While searching the successor basic blocks, if a `load` occurs from one of the cached `ptr`s the `load` is elided with the `value` known to be stored at the `ptr`.

After the `MemoryTransferPass` is run the all previous instances of the target pattern will be replaced to look like the following:

```llvm
  store i32 %1, i32* %4, align 4
  ...
  br i1 %5, label %6, label %7

6:
  store i32 %1, i32* %3, align 4
```

## Primitive Branch Pass

In the `max` and `min` IR functions, there exists a pattern where a branch is used simply to store a value in memory which the memory is loaded in a trailing basic block. This pattern essentially simulates a `select` instruction. The pattern looks like the following in IR:

```llvm
  store i32 %0, i32* %3, align 4
  ...
  br i1 %5, label %6, label %7

6:
  store i32 %1, i32* %3, align 4
  br label %7

7:
  %8 = load i32, i32* %3, align 4
```

Value `%0` is stored by default in the memory location `%3`. If the predicate value `%5` is `true`, then `%3` is updated with the value of `%1`. The branching paths merge on a basic block which reads memory location `%3`. This behavior is the exactly what the `select` IR instruction performs.

In order to match instances of this pattern the following criteria must be met:

-	The root basic block must have a conditional branch with successors `sX` and `sY`
-	The root basic block must store a default value into a memory location `mem`
-	Branch successor, `sX` must only `store` to `mem` before branching to `sY`
-	`sY` must not `store` into `mem` before loading `mem`

If the criteria is met, the two value defs must be determined and a `select` instruction created with them and the branch's condition value. The primitive branch basic block, `sX`, must be erased and the trailing basic block, `sY`, be merged into the root basic block. The original branch and `load` which existed must be removed, and the `load`'s users replaced to use the new `select` instruction.

After the `PrimitiveBranchPass` is run the all previous instances of the target pattern will be replaced to look like the following:

```llvm
  ...
  select i1 %5, i32 %1, i32 %0
  ...
```

## Unused Store Pass

After having run the previous three passes, there are a number of `store` and `alloca` instructions to which no further users exist. The pattern looks like the following in IR:

```llvm
%4 = alloca i32, align 4
...
store i32 %1, i32* %4, align 4
```

A `ptr` defined by an `alloca` instruction local to an IR function where only `store` instruction are users can be eliminated entirely, because the memory goes unread through the lifetime of the memory. If the `ptr` is used in any other way besides a `store` then we can not eliminate the `ptr` definition and users.

`UnusedStorePass` implements this optimization by identifying every `store` to a `ptr` with no other users besides `store` instructions. The identified `store`s are pruned from their basic block parents. This pruning will by matter of course, deplete the users of the `ptr`'s definition (an `alloca` instruction). If a `store` was pruned, the basic block is then pruned of `alloca` instructions with no users.

After the `UnusedStorePass` is run the all previous instances of the target pattern will be eliminated.

## Before and After

`min` function before optimization as generated by Clang 9 with no optimization enabled.

```llvm
define dso_local i32 @min(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = load i32, i32* %3, align 4
  %6 = load i32, i32* %4, align 4
  %7 = icmp sgt i32 %5, %6
  br i1 %7, label %8, label %10

8:
  %9 = load i32, i32* %4, align 4
  store i32 %9, i32* %3, align 4
  br label %10

10:
  %11 = load i32, i32* %3, align 4
  ret i32 %11
}
```

`min` function after being processed by this middle-end optimization pipeline.

```llvm
define dso_local i32 @min(i32 %0, i32 %1) #0 {
  %3 = icmp sgt i32 %0, %1
  %4 = select i1 %3, i32 %1, i32 %0
  ret i32 %4
}
```
