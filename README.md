Converts [ClangIR](https://github.com/llvm/clangir) SSA Variables to Regex format. I got tired of doing this myself when adding CIR tests. Getting the global definition and replacing with wildcards isn't entirely accurate for scopes, but meh -- this works for small cases.

## Sample input (`in.txt`)
```
cir.scope {
      %4 = cir.alloca !s32i, !cir.ptr<!s32i>, ["i", init] {alignment = 4 : i64} loc(#loc20)
      %5 = cir.const #cir.int<0> : !s32i loc(#loc10)
      cir.store %5, %4 : !s32i, !cir.ptr<!s32i> loc(#loc20)
      cir.for : cond {
        %6 = cir.load %4 : !cir.ptr<!s32i>, !s32i loc(#loc11)
        %7 = cir.load %0 : !cir.ptr<!s32i>, !s32i loc(#loc12)
        %8 = cir.cmp(lt, %6, %7) : !s32i, !cir.bool loc(#loc21)
        cir.condition(%8) loc(#loc21)
      } body {
        %6 = cir.load %4 : !cir.ptr<!s32i>, !s32i loc(#loc8)
        cir.store %6, %1 : !s32i, !cir.ptr<!s32i> loc(#loc22)
        cir.yield loc(#loc8)
      } step {
        %6 = cir.load %4 : !cir.ptr<!s32i>, !s32i loc(#loc14)
        %7 = cir.unary(inc, %6) : !s32i, !s32i loc(#loc15)
        cir.store %7, %4 : !s32i, !cir.ptr<!s32i> loc(#loc23)
        cir.yield loc(#loc19)
      } loc(#loc19)
    } loc(#loc19)
```
## Sample output (out.txt)
```
cir.scope {
      %[[V4:.*]] = cir.alloca !s32i, !cir.ptr<!s32i>, ["i", init] {alignment = 4 : i64}
      %[[V5:.*]] = cir.const #cir.int<0> : !s32i
      cir.store %[[V5]], %[[V4]] : !s32i, !cir.ptr<!s32i>
      cir.for : cond {
        %[[V6:.*]] = cir.load %[[V4]] : !cir.ptr<!s32i>, !s32i
        %[[V7:.*]] = cir.load {{.*}} : !cir.ptr<!s32i>, !s32i
        %[[V8:.*]] = cir.cmp(lt, %[[V6]], %[[V7]]) : !s32i, !cir.bool
        cir.condition(%[[V8]])
      } body {
        %[[V6:.*]] = cir.load %[[V4]] : !cir.ptr<!s32i>, !s32i
        cir.store %[[V6]], {{.*}} : !s32i, !cir.ptr<!s32i>
        cir.yield
      } step {
        %[[V6:.*]] = cir.load %[[V4]] : !cir.ptr<!s32i>, !s32i
        %[[V7:.*]] = cir.unary(inc, %[[V6]]) : !s32i, !s32i
        cir.store %[[V7]], %[[V4]] : !s32i, !cir.ptr<!s32i>
        cir.yield
      }
    }
```
