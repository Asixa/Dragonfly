; ModuleID = 'test.cpp'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-mingw32"

%struct.A = type { i32, i32 }

@a = global %struct.A zeroinitializer, align 4
@llvm.global_ctors = appending global [1 x { i32, void ()* }] [{ i32, void ()* } { i32 65535, void ()* @_GLOBAL__I_a }]

define internal void @__cxx_global_var_init() {
entry:
  call void @_ZN1AC2Ei(%struct.A* @a, i32 2333)
  ret void
}

; Function Attrs: nounwind
define linkonce_odr void @_ZN1AC2Ei(%struct.A* %this, i32 %x) unnamed_addr #0 align 2 {
entry:
  %this.addr = alloca %struct.A*, align 4
  %x.addr = alloca i32, align 4
  store %struct.A* %this, %struct.A** %this.addr, align 4
  store i32 %x, i32* %x.addr, align 4
  %this1 = load %struct.A** %this.addr
  %0 = load i32* %x.addr, align 4
  %size = getelementptr inbounds %struct.A* %this1, i32 0, i32 0
  store i32 %0, i32* %size, align 4
  %1 = load i32* %x.addr, align 4
  %align = getelementptr inbounds %struct.A* %this1, i32 0, i32 1
  store i32 %1, i32* %align, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @main() #0 {
entry:
  ret i32 0
}

define internal void @_GLOBAL__I_a() {
entry:
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 (198054)"}
