; ModuleID = 'reduc2.c'
source_filename = "reduc2.c"
target datalayout = "e-i64:64-i128:128-v16:16-v32:32-n16:32:64"
target triple = "nvptx64-nvidia-cuda"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%struct.DynamicEnvironmentTy = type { i16 }
%struct.KernelEnvironmentTy = type { %struct.ConfigurationEnvironmentTy, ptr, ptr }
%struct.ConfigurationEnvironmentTy = type { i8, i8, i8, i32, i32, i32, i32, i32, i32 }

@__omp_rtl_debug_kind = weak_odr hidden constant i32 0
@__omp_rtl_assume_teams_oversubscription = weak_odr hidden constant i32 0
@__omp_rtl_assume_threads_oversubscription = weak_odr hidden constant i32 0
@__omp_rtl_assume_no_thread_state = weak_odr hidden constant i32 0
@__omp_rtl_assume_no_nested_parallelism = weak_odr hidden constant i32 0
@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, ptr @0 }, align 8
@__omp_offloading_50_b137bb_main_l6_dynamic_environment = weak_odr protected global %struct.DynamicEnvironmentTy zeroinitializer
@__omp_offloading_50_b137bb_main_l6_kernel_environment = weak_odr protected constant %struct.KernelEnvironmentTy { %struct.ConfigurationEnvironmentTy { i8 1, i8 1, i8 1, i32 1, i32 128, i32 0, i32 0, i32 0, i32 0 }, ptr @1, ptr @__omp_offloading_50_b137bb_main_l6_dynamic_environment }
@2 = private unnamed_addr constant %struct.ident_t { i32 0, i32 514, i32 0, i32 22, ptr @0 }, align 8
@__openmp_nvptx_data_transfer_temporary_storage = weak addrspace(3) global [32 x i32] undef
@3 = private unnamed_addr constant %struct.ident_t { i32 0, i32 66, i32 0, i32 22, ptr @0 }, align 8
@llvm.compiler.used = appending global [1 x ptr] [ptr addrspacecast (ptr addrspace(3) @__openmp_nvptx_data_transfer_temporary_storage to ptr)], section "llvm.metadata"
@llvm.compiler.used1 = appending global [1 x ptr] [ptr addrspacecast (ptr addrspace(3) @__openmp_nvptx_data_transfer_temporary_storage to ptr)], section "llvm.metadata"
@llvm.compiler.used2 = appending global [1 x ptr] [ptr addrspacecast (ptr addrspace(3) @__openmp_nvptx_data_transfer_temporary_storage to ptr)], section "llvm.metadata"
@llvm.compiler.used3 = appending global [1 x ptr] [ptr addrspacecast (ptr addrspace(3) @__openmp_nvptx_data_transfer_temporary_storage to ptr)], section "llvm.metadata"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define weak_odr protected void @__omp_offloading_50_b137bb_main_l6(ptr noalias noundef %dyn_ptr) #0 {
entry:
  %dyn_ptr.addr = alloca ptr, align 8
  %.zero.addr = alloca i32, align 4
  %.threadid_temp. = alloca i32, align 4
  store ptr %dyn_ptr, ptr %dyn_ptr.addr, align 8
  %0 = call i32 @__kmpc_target_init(ptr @__omp_offloading_50_b137bb_main_l6_kernel_environment, ptr %dyn_ptr)
  %exec_user_code = icmp eq i32 %0, -1
  br i1 %exec_user_code, label %user_code.entry, label %worker.exit

user_code.entry:                                  ; preds = %entry
  %1 = call i32 @__kmpc_global_thread_num(ptr @1)
  store i32 0, ptr %.zero.addr, align 4
  store i32 %1, ptr %.threadid_temp., align 4
  call void @__omp_offloading_50_b137bb_main_l6_omp_outlined(ptr %.threadid_temp., ptr %.zero.addr) #3
  call void @__kmpc_target_deinit()
  ret void

worker.exit:                                      ; preds = %entry
  ret void
}

declare i32 @__kmpc_target_init(ptr, ptr)

; Function Attrs: convergent noinline norecurse nounwind optnone
define internal void @__omp_offloading_50_b137bb_main_l6_omp_outlined(ptr noalias noundef %.global_tid., ptr noalias noundef %.bound_tid.) #1 {
entry:
  %.global_tid..addr = alloca ptr, align 8
  %.bound_tid..addr = alloca ptr, align 8
  %captured_vars_addrs = alloca [1 x ptr], align 8
  store ptr %.global_tid., ptr %.global_tid..addr, align 8
  store ptr %.bound_tid., ptr %.bound_tid..addr, align 8
  %sum = call align 16 ptr @__kmpc_alloc_shared(i64 4)
  store i32 0, ptr %sum, align 4
  %0 = getelementptr inbounds [1 x ptr], ptr %captured_vars_addrs, i64 0, i64 0
  store ptr %sum, ptr %0, align 8
  %1 = load ptr, ptr %.global_tid..addr, align 8
  %2 = load i32, ptr %1, align 4
  call void @__kmpc_parallel_51(ptr @1, i32 %2, i32 1, i32 -1, i32 -1, ptr @__omp_offloading_50_b137bb_main_l6_omp_outlined_omp_outlined, ptr @__omp_offloading_50_b137bb_main_l6_omp_outlined_omp_outlined_wrapper, ptr %captured_vars_addrs, i64 1)
  call void @__kmpc_free_shared(ptr %sum, i64 4)
  ret void
}

; Function Attrs: nosync nounwind allocsize(0)
declare noalias ptr @__kmpc_alloc_shared(i64) #2

; Function Attrs: convergent noinline norecurse nounwind optnone
define internal void @__omp_offloading_50_b137bb_main_l6_omp_outlined_omp_outlined(ptr noalias noundef %.global_tid., ptr noalias noundef %.bound_tid., ptr noundef nonnull align 4 dereferenceable(4) %sum) #1 {
entry:
  %.global_tid..addr = alloca ptr, align 8
  %.bound_tid..addr = alloca ptr, align 8
  %sum.addr = alloca ptr, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %sum1 = alloca i32, align 4
  %i = alloca i32, align 4
  %.omp.reduction.red_list = alloca [1 x ptr], align 8
  store ptr %.global_tid., ptr %.global_tid..addr, align 8
  store ptr %.bound_tid., ptr %.bound_tid..addr, align 8
  store ptr %sum, ptr %sum.addr, align 8
  %0 = load ptr, ptr %sum.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 127, ptr %.omp.ub, align 4
  store i32 1, ptr %.omp.stride, align 4
  store i32 0, ptr %.omp.is_last, align 4
  store i32 0, ptr %sum1, align 4
  %1 = load ptr, ptr %.global_tid..addr, align 8
  %2 = load i32, ptr %1, align 4
  call void @__kmpc_for_static_init_4(ptr @2, i32 %2, i32 33, ptr %.omp.is_last, ptr %.omp.lb, ptr %.omp.ub, ptr %.omp.stride, i32 1, i32 1)
  br label %omp.dispatch.cond

omp.dispatch.cond:                                ; preds = %omp.dispatch.inc, %entry
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sgt i32 %3, 127
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.dispatch.cond
  br label %cond.end

cond.false:                                       ; preds = %omp.dispatch.cond
  %4 = load i32, ptr %.omp.ub, align 4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ 127, %cond.true ], [ %4, %cond.false ]
  store i32 %cond, ptr %.omp.ub, align 4
  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp2 = icmp sle i32 %6, %7
  br i1 %cmp2, label %omp.dispatch.body, label %omp.dispatch.end

omp.dispatch.body:                                ; preds = %cond.end
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.dispatch.body
  %8 = load i32, ptr %.omp.iv, align 4
  %9 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %8, %9
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %11 = load i32, ptr %sum1, align 4
  %inc = add nsw i32 %11, 1
  store i32 %inc, ptr %sum1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %12, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.dispatch.inc

omp.dispatch.inc:                                 ; preds = %omp.inner.for.end
  %13 = load i32, ptr %.omp.lb, align 4
  %14 = load i32, ptr %.omp.stride, align 4
  %add5 = add nsw i32 %13, %14
  store i32 %add5, ptr %.omp.lb, align 4
  %15 = load i32, ptr %.omp.ub, align 4
  %16 = load i32, ptr %.omp.stride, align 4
  %add6 = add nsw i32 %15, %16
  store i32 %add6, ptr %.omp.ub, align 4
  br label %omp.dispatch.cond

omp.dispatch.end:                                 ; preds = %cond.end
  call void @__kmpc_for_static_fini(ptr @2, i32 %2)
  %17 = getelementptr inbounds [1 x ptr], ptr %.omp.reduction.red_list, i64 0, i64 0
  store ptr %sum1, ptr %17, align 8
  %18 = call i32 @__kmpc_nvptx_parallel_reduce_nowait_v2(ptr @1, i64 4, ptr %.omp.reduction.red_list, ptr @_omp_reduction_shuffle_and_reduce_func, ptr @_omp_reduction_inter_warp_copy_func)
  %19 = icmp eq i32 %18, 1
  br i1 %19, label %.omp.reduction.then, label %.omp.reduction.done

.omp.reduction.then:                              ; preds = %omp.dispatch.end
  %20 = load i32, ptr %0, align 4
  %21 = load i32, ptr %sum1, align 4
  %add7 = add nsw i32 %20, %21
  store i32 %add7, ptr %0, align 4
  br label %.omp.reduction.done

.omp.reduction.done:                              ; preds = %.omp.reduction.then, %omp.dispatch.end
  ret void
}

; Function Attrs: nounwind
declare void @__kmpc_for_static_init_4(ptr, i32, i32, ptr, ptr, ptr, ptr, i32, i32) #3

; Function Attrs: nounwind
declare void @__kmpc_for_static_fini(ptr, i32) #3

; Function Attrs: convergent noinline norecurse nounwind
define internal void @"__omp_offloading_50_b137bb_main_l6_omp_outlined_omp_outlined_omp$reduction$reduction_func"(ptr noundef %0, ptr noundef %1) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr, align 8
  %3 = load ptr, ptr %.addr1, align 8
  %4 = getelementptr inbounds [1 x ptr], ptr %3, i64 0, i64 0
  %5 = load ptr, ptr %4, align 8
  %6 = getelementptr inbounds [1 x ptr], ptr %2, i64 0, i64 0
  %7 = load ptr, ptr %6, align 8
  %8 = load i32, ptr %7, align 4
  %9 = load i32, ptr %5, align 4
  %add = add nsw i32 %8, %9
  store i32 %add, ptr %7, align 4
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define internal void @_omp_reduction_shuffle_and_reduce_func(ptr noundef %0, i16 noundef signext %1, i16 noundef signext %2, i16 noundef signext %3) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca i16, align 2
  %.addr2 = alloca i16, align 2
  %.addr3 = alloca i16, align 2
  %.omp.reduction.remote_reduce_list = alloca [1 x ptr], align 8
  %.omp.reduction.element = alloca i32, align 4
  store ptr %0, ptr %.addr, align 8
  store i16 %1, ptr %.addr1, align 2
  store i16 %2, ptr %.addr2, align 2
  store i16 %3, ptr %.addr3, align 2
  %4 = load ptr, ptr %.addr, align 8
  %5 = load i16, ptr %.addr1, align 2
  %6 = load i16, ptr %.addr2, align 2
  %7 = load i16, ptr %.addr3, align 2
  %8 = getelementptr inbounds [1 x ptr], ptr %4, i64 0, i64 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds [1 x ptr], ptr %.omp.reduction.remote_reduce_list, i64 0, i64 0
  %11 = getelementptr i32, ptr %9, i64 1
  %12 = load i32, ptr %9, align 4
  %13 = call i32 @__kmpc_get_warp_size()
  %14 = trunc i32 %13 to i16
  %15 = call i32 @__kmpc_shuffle_int32(i32 %12, i16 %6, i16 %14)
  store i32 %15, ptr %.omp.reduction.element, align 4
  %16 = getelementptr i32, ptr %9, i64 1
  %17 = getelementptr i32, ptr %.omp.reduction.element, i64 1
  store ptr %.omp.reduction.element, ptr %10, align 8
  %18 = icmp eq i16 %7, 0
  %19 = icmp eq i16 %7, 1
  %20 = icmp ult i16 %5, %6
  %21 = and i1 %19, %20
  %22 = icmp eq i16 %7, 2
  %23 = and i16 %5, 1
  %24 = icmp eq i16 %23, 0
  %25 = and i1 %22, %24
  %26 = icmp sgt i16 %6, 0
  %27 = and i1 %25, %26
  %28 = or i1 %18, %21
  %29 = or i1 %28, %27
  br i1 %29, label %then, label %else

then:                                             ; preds = %entry
  call void @"__omp_offloading_50_b137bb_main_l6_omp_outlined_omp_outlined_omp$reduction$reduction_func"(ptr %4, ptr %.omp.reduction.remote_reduce_list) #3
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %30 = icmp eq i16 %7, 1
  %31 = icmp uge i16 %5, %6
  %32 = and i1 %30, %31
  br i1 %32, label %then4, label %else5

then4:                                            ; preds = %ifcont
  %33 = getelementptr inbounds [1 x ptr], ptr %.omp.reduction.remote_reduce_list, i64 0, i64 0
  %34 = load ptr, ptr %33, align 8
  %35 = getelementptr inbounds [1 x ptr], ptr %4, i64 0, i64 0
  %36 = load ptr, ptr %35, align 8
  %37 = load i32, ptr %34, align 4
  store i32 %37, ptr %36, align 4
  br label %ifcont6

else5:                                            ; preds = %ifcont
  br label %ifcont6

ifcont6:                                          ; preds = %else5, %then4
  ret void
}

; Function Attrs: nounwind
declare i32 @__kmpc_get_warp_size() #3

declare i32 @__kmpc_shuffle_int32(i32, i16, i16)

; Function Attrs: convergent noinline norecurse nounwind
define internal void @_omp_reduction_inter_warp_copy_func(ptr noundef %0, i32 noundef %1) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca i32, align 4
  %omp_global_thread_num = call i32 @__kmpc_global_thread_num(ptr @1)
  store ptr %0, ptr %.addr, align 8
  store i32 %1, ptr %.addr1, align 4
  %2 = call i32 @__kmpc_get_hardware_thread_id_in_block()
  %3 = call i32 @__kmpc_get_hardware_thread_id_in_block()
  %nvptx_lane_id = and i32 %3, 31
  %4 = call i32 @__kmpc_get_hardware_thread_id_in_block()
  %nvptx_warp_id = ashr i32 %4, 5
  %5 = load ptr, ptr %.addr, align 8
  call void @__kmpc_barrier(ptr @3, i32 %omp_global_thread_num)
  %warp_master = icmp eq i32 %nvptx_lane_id, 0
  br i1 %warp_master, label %then, label %else

then:                                             ; preds = %entry
  %6 = getelementptr inbounds [1 x ptr], ptr %5, i64 0, i64 0
  %7 = load ptr, ptr %6, align 8
  %8 = getelementptr inbounds [32 x i32], ptr addrspace(3) @__openmp_nvptx_data_transfer_temporary_storage, i64 0, i32 %nvptx_warp_id
  %9 = load i32, ptr %7, align 4
  store volatile i32 %9, ptr addrspace(3) %8, align 4
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  call void @__kmpc_barrier(ptr @3, i32 %omp_global_thread_num)
  %10 = load i32, ptr %.addr1, align 4
  %is_active_thread = icmp ult i32 %2, %10
  br i1 %is_active_thread, label %then2, label %else3

then2:                                            ; preds = %ifcont
  %11 = getelementptr inbounds [32 x i32], ptr addrspace(3) @__openmp_nvptx_data_transfer_temporary_storage, i64 0, i32 %2
  %12 = getelementptr inbounds [1 x ptr], ptr %5, i64 0, i64 0
  %13 = load ptr, ptr %12, align 8
  %14 = load volatile i32, ptr addrspace(3) %11, align 4
  store i32 %14, ptr %13, align 4
  br label %ifcont4

else3:                                            ; preds = %ifcont
  br label %ifcont4

ifcont4:                                          ; preds = %else3, %then2
  ret void
}

; Function Attrs: nounwind
declare i32 @__kmpc_get_hardware_thread_id_in_block() #3

; Function Attrs: nounwind
declare i32 @__kmpc_global_thread_num(ptr) #3

; Function Attrs: convergent nounwind
declare void @__kmpc_barrier(ptr, i32) #5

declare i32 @__kmpc_nvptx_parallel_reduce_nowait_v2(ptr, i64, ptr, ptr, ptr)

; Function Attrs: convergent noinline norecurse nounwind
define internal void @__omp_offloading_50_b137bb_main_l6_omp_outlined_omp_outlined_wrapper(i16 noundef zeroext %0, i32 noundef %1) #4 {
entry:
  %.addr = alloca i16, align 2
  %.addr1 = alloca i32, align 4
  %.zero.addr = alloca i32, align 4
  %global_args = alloca ptr, align 8
  store i16 %0, ptr %.addr, align 2
  store i32 %1, ptr %.addr1, align 4
  store i32 0, ptr %.zero.addr, align 4
  call void @__kmpc_get_shared_variables(ptr %global_args)
  %2 = load ptr, ptr %global_args, align 8
  %3 = getelementptr inbounds ptr, ptr %2, i64 0
  %4 = load ptr, ptr %3, align 8
  call void @__omp_offloading_50_b137bb_main_l6_omp_outlined_omp_outlined(ptr %.addr1, ptr %.zero.addr, ptr %4) #3
  ret void
}

declare void @__kmpc_get_shared_variables(ptr)

; Function Attrs: alwaysinline
declare void @__kmpc_parallel_51(ptr, i32, i32, i32, i32, ptr, ptr, ptr, i64) #6

; Function Attrs: nosync nounwind
declare void @__kmpc_free_shared(ptr allocptr nocapture, i64) #7

declare void @__kmpc_target_deinit()

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "frame-pointer"="all" "kernel" "no-trapping-math"="true" "omp_target_thread_limit"="128" "stack-protector-buffer-size"="8" "target-cpu"="sm_70" "target-features"="+ptx72,+sm_70" }
attributes #1 = { convergent noinline norecurse nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="sm_70" "target-features"="+ptx72,+sm_70" }
attributes #2 = { nosync nounwind allocsize(0) }
attributes #3 = { nounwind }
attributes #4 = { convergent noinline norecurse nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="sm_70" "target-features"="+ptx72,+sm_70" }
attributes #5 = { convergent nounwind }
attributes #6 = { alwaysinline }
attributes #7 = { nosync nounwind }

!nvvm.annotations = !{!0, !1, !1}
!omp_offload.info = !{!2, !2, !2, !2, !2}
!llvm.module.flags = !{!3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = !{ptr @__omp_offloading_50_b137bb_main_l6, !"maxntidx", i32 128}
!1 = !{ptr @__omp_offloading_50_b137bb_main_l6, !"kernel", i32 1}
!2 = !{i32 0, i32 80, i32 11614139, !"main", i32 6, i32 0, i32 0}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"openmp", i32 51}
!5 = !{i32 7, !"openmp-device", i32 51}
!6 = !{i32 8, !"PIC Level", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 19.0.0git (https://github.com/efwright/llvm-project.git 55465ae477b156894e37c7f02167ae0989b33b49)"}
