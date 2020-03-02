# LLVM Explore

**Fisher Price Baby's First: LLVM Middle-end Optimization Pipeline**

The boilerplate template for the passes and cmake file were taken from [llvm-tutor](https://github.com/banach-space/llvm-tutor).
In addition to diving into the [LLVM Docs](http://llvm.org/doxygen/), I referenced heavily two talks from the 2019 LLVM Developer's Meeting: [Getting Started with LLVM](https://www.youtube.com/watch?v=3QQuhL-dSys) and [Writing and LLVM Pass 101](https://www.youtube.com/watch?v=ar7cJl2aBuU).

## Overview

This is a mini-pipeline to optimize three simple C functions:

-	[`int min(int value, int min);`](https://github.com/mkitzan/llvm-explore/blob/master/test/clamp.c#L2)
-	[`int max(int value, int max);`](https://github.com/mkitzan/llvm-explore/blob/master/test/clamp.c#L12)
-	[`int clamp(int value, int m, int M);`](https://github.com/mkitzan/llvm-explore/blob/master/test/clamp.c#L22)

The un-optimized LLVM IR for these functions were riddled with unnecessary memory accesses.
The goal of the pipeline is to eliminate all the memory accesses and collapse primitive conditional branches into `select` IR instructions. The final output of the pipeline should be very close to the emitted IR when the functions are compiled with an optimization flag greater than 0.

The pipeline is composed of four passes where each optimization pass sets up the IR for the next optimization pass.
The four passes are the following and are meant to execute in the following order:

-	[`RedundantLoadPass`](https://github.com/mkitzan/llvm-explore/blob/master/pass/RedundantLoadPass.cpp)
-	[`MemoryTransferPass`](https://github.com/mkitzan/llvm-explore/blob/master/pass/MemoryTransferPass.cpp)
-	[`UnusedStorePass`](https://github.com/mkitzan/llvm-explore/blob/master/pass/UnusedStorePass.cpp)
-	`PrimitiveBranchPass` (not yet implemented)

`lit` tests for each pass are planned to be implemented (later today...).

## Redundant Load Pass

In the un-optimized IR from `clang-9`, the following pattern is pervasive within basic blocks of the three functions:

```
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

After the `RedundantLoadPass` is run the all previous instance of the target pattern will be replaced to look like the following:

```
%3 = alloca i32, align 4
...
store i32 %0, i32* %3, align 4
...
%5 = icmp sgt i32 %0, %1
```

## Memory Transfer Pass

In the `max` and `min` IR functions, there exists a pattern where a virtual register's value is transferred through memory to a basic block with a single predecessor basic block. The pattern looks like the following in IR:

```
  store i32 %1, i32* %4, align 4
  ...
  br i1 %5, label %6, label %8

6:
  %7 = load i32, i32* %4, align 4  
  store i32 %7, i32* %3, align 4
```

If the basic block who the data is transferred to had multiple predecessors then the value could be derived from either predecessor. Because the value is transferred to a basic block with a single predecessor, the transferred value can be deduced from the `store` in the parent basic block.

`MemoryTransferPass` implements this optimization by caching the `ptr` and most recently stored `value` at `ptr` into a map, and when a `br` opcode is found the branch successors with a single predecessor are searched. While searching the successor basic blocks, if a `load` occurs from one of the cached `ptr`s the `load` is elided with the `value` known to be stored at the `ptr`.

After the `MemoryTransferPass` is run the all previous instance of the target pattern will be replaced to look like the following:

```
  store i32 %0, i32* %3, align 4
  ...
  br i1 %5, label %6, label %7

6:
  store i32 %1, i32* %3, align 4
```

## Unused Store Pass

todo

## Primitive Branch Pass

todo
