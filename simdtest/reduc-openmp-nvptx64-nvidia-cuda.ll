; ModuleID = 'reduc.c'
source_filename = "reduc.c"
target datalayout = "e-i64:64-i128:128-v16:16-v32:32-n16:32:64"
target triple = "nvptx64-nvidia-cuda"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%struct.DynamicEnvironmentTy = type { i16 }
%struct.KernelEnvironmentTy = type { %struct.ConfigurationEnvironmentTy, ptr, ptr }
%struct.ConfigurationEnvironmentTy = type { i8, i8, i8, i32, i32, i32, i32, i32, i32 }
%struct.anon = type { ptr }
%struct.anon.0 = type { i32 }

@__omp_rtl_debug_kind = weak_odr hidden constant i32 0
@__omp_rtl_assume_teams_oversubscription = weak_odr hidden constant i32 0
@__omp_rtl_assume_threads_oversubscription = weak_odr hidden constant i32 0
@__omp_rtl_assume_no_thread_state = weak_odr hidden constant i32 0
@__omp_rtl_assume_no_nested_parallelism = weak_odr hidden constant i32 0
@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, ptr @0 }, align 8
@__omp_offloading_50_239ef55_main_l6_dynamic_environment = weak_odr protected global %struct.DynamicEnvironmentTy zeroinitializer
@__omp_offloading_50_239ef55_main_l6_kernel_environment = weak_odr protected constant %struct.KernelEnvironmentTy { %struct.ConfigurationEnvironmentTy { i8 0, i8 1, i8 2, i32 1, i32 32, i32 1, i32 1, i32 0, i32 0 }, ptr @1, ptr @__omp_offloading_50_239ef55_main_l6_dynamic_environment }

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define weak_odr protected void @__omp_offloading_50_239ef55_main_l6(ptr noalias noundef %dyn_ptr) #0 {
entry:
  %dyn_ptr.addr = alloca ptr, align 8
  %captured_vars_addrs = alloca [0 x ptr], align 8
  store ptr %dyn_ptr, ptr %dyn_ptr.addr, align 8
  %0 = call i32 @__kmpc_target_init(ptr @__omp_offloading_50_239ef55_main_l6_kernel_environment, ptr %dyn_ptr)
  %exec_user_code = icmp eq i32 %0, -1
  br i1 %exec_user_code, label %user_code.entry, label %worker.exit

user_code.entry:                                  ; preds = %entry
  %1 = call i32 @__kmpc_global_thread_num(ptr @1)
  call void @__kmpc_parallel_51(ptr @1, i32 %1, i32 1, i32 32, i32 -1, ptr @__omp_offloading_50_239ef55_main_l6_omp_outlined, ptr null, ptr %captured_vars_addrs, i64 0)
  call void @__kmpc_target_deinit()
  ret void

worker.exit:                                      ; preds = %entry
  ret void
}

declare i32 @__kmpc_target_init(ptr, ptr)

; Function Attrs: convergent noinline norecurse nounwind optnone
define internal void @__omp_offloading_50_239ef55_main_l6_omp_outlined(ptr noalias noundef %.global_tid., ptr noalias noundef %.bound_tid.) #1 {
entry:
  %structArg = alloca { ptr, ptr, ptr }, align 8
  %.global_tid..addr = alloca ptr, align 8
  %.bound_tid..addr = alloca ptr, align 8
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  %agg.captured = alloca %struct.anon, align 8
  %.count.addr = alloca i32, align 4
  %sum1 = alloca i32, align 4
  %agg.captured2 = alloca %struct.anon.0, align 4
  store ptr %.global_tid., ptr %.global_tid..addr, align 8
  store ptr %.bound_tid., ptr %.bound_tid..addr, align 8
  store i32 0, ptr %sum, align 4
  br label %omp.loop.distance

omp.loop.distance:                                ; preds = %entry
  store i32 0, ptr %i, align 4
  %0 = getelementptr inbounds %struct.anon, ptr %agg.captured, i32 0, i32 0
  store ptr %i, ptr %0, align 8
  call void @__captured_stmt(ptr %.count.addr, ptr %agg.captured)
  %.count = load i32, ptr %.count.addr, align 4
  store i32 0, ptr %sum1, align 4
  br label %omp_loop

omp_loop:                                         ; preds = %omp.loop.distance
  %gep_agg.captured2 = getelementptr { ptr, ptr, ptr }, ptr %structArg, i32 0, i32 0
  store ptr %agg.captured2, ptr %gep_agg.captured2, align 8
  %gep_i = getelementptr { ptr, ptr, ptr }, ptr %structArg, i32 0, i32 1
  store ptr %i, ptr %gep_i, align 8
  %gep_sum1 = getelementptr { ptr, ptr, ptr }, ptr %structArg, i32 0, i32 2
  store ptr %sum1, ptr %gep_sum1, align 8
  call void @__kmpc_simd_4u(ptr @1, ptr @__omp_offloading_50_239ef55_main_l6_omp_outlined..omp_par, i32 %.count, ptr %structArg, i32 3)
  br label %omp.loop.outlined.exit

omp.loop.outlined.exit:                           ; preds = %omp_loop
  br label %omp.loop.exit.split

omp.loop.exit.split:                              ; preds = %omp.loop.outlined.exit
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone
define internal void @__omp_offloading_50_239ef55_main_l6_omp_outlined..omp_par(i32 %omp.iv, ptr %0) #2 {
omp.loop.entry:
  %gep_agg.captured2 = getelementptr { ptr, ptr, ptr }, ptr %0, i32 0, i32 0
  %loadgep_agg.captured2 = load ptr, ptr %gep_agg.captured2, align 8
  %gep_i = getelementptr { ptr, ptr, ptr }, ptr %0, i32 0, i32 1
  %loadgep_i = load ptr, ptr %gep_i, align 8
  %gep_sum1 = getelementptr { ptr, ptr, ptr }, ptr %0, i32 0, i32 2
  %loadgep_sum1 = load ptr, ptr %gep_sum1, align 8
  %i.loopvar = alloca i32, align 4
  %1 = getelementptr inbounds %struct.anon.0, ptr %loadgep_agg.captured2, i32 0, i32 0
  %2 = load i32, ptr %loadgep_i, align 4
  store i32 %2, ptr %1, align 4
  br label %omp.loop.region

omp.loop.region:                                  ; preds = %omp.loop.entry
  call void @__captured_stmt1(ptr %i.loopvar, i32 %omp.iv, ptr %loadgep_agg.captured2)
  %3 = load i32, ptr %loadgep_sum1, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %loadgep_sum1, align 4
  br label %omp.loop.region.simd.after

omp.loop.region.simd.after:                       ; preds = %omp.loop.region
  br label %omp.loop.pre_finalize

omp.loop.pre_finalize:                            ; preds = %omp.loop.region.simd.after
  br label %omp.loop.outlined.exit.exitStub

omp.loop.outlined.exit.exitStub:                  ; preds = %omp.loop.pre_finalize
  ret void
}

; Function Attrs: convergent noinline nounwind optnone
define internal void @__captured_stmt(ptr noundef nonnull align 4 dereferenceable(4) %Distance, ptr noalias noundef %__context) #3 {
entry:
  %Distance.addr = alloca ptr, align 8
  %__context.addr = alloca ptr, align 8
  %.start = alloca i32, align 4
  %.stop = alloca i32, align 4
  %.step = alloca i32, align 4
  store ptr %Distance, ptr %Distance.addr, align 8
  store ptr %__context, ptr %__context.addr, align 8
  %0 = load ptr, ptr %__context.addr, align 8
  %1 = getelementptr inbounds %struct.anon, ptr %0, i32 0, i32 0
  %2 = load ptr, ptr %1, align 8
  %3 = load i32, ptr %2, align 4
  store i32 %3, ptr %.start, align 4
  store i32 32, ptr %.stop, align 4
  store i32 1, ptr %.step, align 4
  %4 = load i32, ptr %.start, align 4
  %5 = load i32, ptr %.stop, align 4
  %cmp = icmp slt i32 %4, %5
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %6 = load i32, ptr %.stop, align 4
  %7 = load i32, ptr %.start, align 4
  %sub = sub nsw i32 %6, %7
  %8 = load i32, ptr %.step, align 4
  %sub1 = sub i32 %8, 1
  %add = add i32 %sub, %sub1
  %9 = load i32, ptr %.step, align 4
  %div = udiv i32 %add, %9
  br label %cond.end

cond.false:                                       ; preds = %entry
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %div, %cond.true ], [ 0, %cond.false ]
  %10 = load ptr, ptr %Distance.addr, align 8
  store i32 %cond, ptr %10, align 4
  ret void
}

; Function Attrs: convergent noinline nounwind optnone
define internal void @__captured_stmt1(ptr noundef nonnull align 4 dereferenceable(4) %LoopVar, i32 noundef %Logical, ptr noalias noundef %__context) #3 {
entry:
  %LoopVar.addr = alloca ptr, align 8
  %Logical.addr = alloca i32, align 4
  %__context.addr = alloca ptr, align 8
  store ptr %LoopVar, ptr %LoopVar.addr, align 8
  store i32 %Logical, ptr %Logical.addr, align 4
  store ptr %__context, ptr %__context.addr, align 8
  %0 = load ptr, ptr %__context.addr, align 8
  %1 = getelementptr inbounds %struct.anon.0, ptr %0, i32 0, i32 0
  %2 = load i32, ptr %1, align 4
  %3 = load i32, ptr %Logical.addr, align 4
  %mul = mul i32 1, %3
  %add = add i32 %2, %mul
  %4 = load ptr, ptr %LoopVar.addr, align 8
  store i32 %add, ptr %4, align 4
  ret void
}

declare void @__kmpc_simd_4u(ptr, ptr, i32, ptr, i32)

; Function Attrs: nounwind
declare i32 @__kmpc_global_thread_num(ptr) #4

; Function Attrs: alwaysinline
declare void @__kmpc_parallel_51(ptr, i32, i32, i32, i32, ptr, ptr, ptr, i64) #5

declare void @__kmpc_target_deinit()

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "frame-pointer"="all" "kernel" "no-trapping-math"="true" "omp_target_num_teams"="1" "omp_target_thread_limit"="32" "stack-protector-buffer-size"="8" "target-cpu"="sm_70" "target-features"="+ptx78,+sm_70" }
attributes #1 = { convergent noinline norecurse nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="sm_70" "target-features"="+ptx78,+sm_70" }
attributes #2 = { noinline norecurse nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="sm_70" "target-features"="+ptx78,+sm_70" }
attributes #3 = { convergent noinline nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="sm_70" "target-features"="+ptx78,+sm_70" }
attributes #4 = { nounwind }
attributes #5 = { alwaysinline }

!nvvm.annotations = !{!0, !1, !2, !3, !3}
!omp_offload.info = !{!4, !4, !4, !4, !4}
!llvm.module.flags = !{!5, !6, !7, !8, !9}
!llvm.ident = !{!10}

!0 = !{ptr @__omp_offloading_50_239ef55_main_l6, !"maxclusterrank", i32 1}
!1 = !{ptr @__omp_offloading_50_239ef55_main_l6, !"minctasm", i32 1}
!2 = !{ptr @__omp_offloading_50_239ef55_main_l6, !"maxntidx", i32 32}
!3 = !{ptr @__omp_offloading_50_239ef55_main_l6, !"kernel", i32 1}
!4 = !{i32 0, i32 80, i32 37351253, !"main", i32 6, i32 0, i32 0}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"openmp", i32 51}
!7 = !{i32 7, !"openmp-device", i32 51}
!8 = !{i32 8, !"PIC Level", i32 2}
!9 = !{i32 7, !"frame-pointer", i32 2}
!10 = !{!"clang version 19.0.0git (https://github.com/efwright/llvm-project.git 55465ae477b156894e37c7f02167ae0989b33b49)"}
