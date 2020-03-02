# LLVM Explore

**Fisher Price Baby's First: LLVM Middle-end Optimization Pipeline**

The boilerplate template for the passes and cmake file were taken from [llvm-tutor](https://github.com/banach-space/llvm-tutor).
In addition to diving into the [LLVM Docs](http://llvm.org/doxygen/), I referenced heavily two talks from the 2019 LLVM Developer's Meeting: [Getting Started with LLVM](https://www.youtube.com/watch?v=3QQuhL-dSys) and [Writing and LLVM Pass 101](https://www.youtube.com/watch?v=ar7cJl2aBuU).

## Overview

This is a mini-pipeline to optimize three simple C functions:

-	`int max(int value, int max);`
-	`int min(int value, int min);`
-	`int clamp(int value, int m, int M);`

The un-optimized LLVM IR for these functions were riddled with unnecessary memory accesses.
The goal of the pipeline is to eliminate all the memory accesses and collapse primitive conditional branches into `select` IR instructions. The final output of the pipeline should be very close to the emitted IR when the functions are compiled with an optimization flag greater than 0.

The pipeline is composed of four passes where each optimization pass sets up the IR for the next optimization pass.
The four passes are the following and are meant to execute in the following order:

-	`RedundantLoadPass`
-	`MemoryTransferPass`
-	`UnusedStorePass`
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
```

Memory is allocated at the top of the basic block, and a virtual register is stored at the memory only to be loaded back into a new virtual register which is then used.
Where this pattern occurs within a basic block the `load` is redundant.
The users of the `load`'s def can be swapped to use the def of the value stored.
We can't make any assumption yet about removing the `store` and `alloca` themselves, because another basic block may `load` the value.

`RedundantLoadPass` implements this optimization by caching the `ptr` and most recently stored `value` at `ptr` into a map, and when a `load` opcode is found the map is searched. If the `load`'s `ptr` is cached, then the `load` can be elided with the `value` known to be stored at the `ptr`.

## Memory Transfer Pass

todo

## Unused Store Pass

todo

## Primitive Branch Pass

todo
