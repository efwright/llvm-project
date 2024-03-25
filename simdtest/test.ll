; ModuleID = '/var/tmp/test-bf76f7.bc'
source_filename = "test.c"
target datalayout = "e-m:e-Fn32-i64:64-n32:64-S128-v256:256:256-v512:512:512"
target triple = "powerpc64le-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%struct.__tgt_offload_entry = type { ptr, ptr, i64, i32, i32 }
%struct.__tgt_kernel_arguments = type { i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, [3 x i32], [3 x i32], i32 }

@.str = private unnamed_addr constant [10 x i8] c"(%i, %i)\0A\00", align 1
@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, ptr @0 }, align 8
@.__omp_offloading_50_10f3bff_foo_l6.region_id = weak constant i8 0
@.omp_offloading.entry_name = internal unnamed_addr constant [35 x i8] c"__omp_offloading_50_10f3bff_foo_l6\00"
@.omp_offloading.entry.__omp_offloading_50_10f3bff_foo_l6 = weak constant %struct.__tgt_offload_entry { ptr @.__omp_offloading_50_10f3bff_foo_l6.region_id, ptr @.omp_offloading.entry_name, i64 0, i32 0, i32 0 }, section "omp_offloading_entries", align 1
@llvm.embedded.object = private constant [6760 x i8] c"\10\FF\10\AD\01\00\00\00h\1A\00\00\00\00\00\00 \00\00\00\00\00\00\00(\00\00\00\00\00\00\00\02\00\01\00\00\00\00\00H\00\00\00\00\00\00\00\02\00\00\00\00\00\00\00\90\00\00\00\00\00\00\00\D8\19\00\00\00\00\00\00i\00\00\00\00\00\00\00\89\00\00\00\00\00\00\00n\00\00\00\00\00\00\00u\00\00\00\00\00\00\00\00arch\00triple\00nvptx64-nvidia-cuda\00sm_70\00\00BC\C0\DE5\14\00\00\05\00\00\00b\0C0$MY\BEf\DD\FB\B4O\1B\C8$D\012\05\00!\0C\00\00\09\05\00\00\0B\02!\00\02\00\00\00\17\00\00\00\07\81#\91A\C8\04I\06\1029\92\01\84\0C%\05\08\19\1E\04\8Bb\80\18E\02B\92\0BB\C4\102\148\08\18K\0A2b\88Hp\C4!#D\12\87\8C\10A\92\02d\C8\08\B1\14 CF\88 \C9\012b\84X\0E\90\11#D\90\A1\82\A2\02\19\C3\07\CB\15\09b\8C\0C\89 \00\00?\00\00\00\22f\04\10\B2B\82\89\11RB\82\89\91q\C2PH\0A\09&F\C6\05Bb&\08\C2\80\E6\08\C0`\8E\00\C9\0B\E1\1Ci\8A(a\F2\91\81h\A6\7FBE\10\04\010G\00\DD&M\11%L~\875\00\83\08^S!\91\D30D3\B5\18\0A\8A\0B\03i\8A(a\F29\A7)\10\83\8A\80\09q\1A\AF\A9\90\C8i\18\A2\99Z\0C%\82 \04A\10\04\C1i\D2\14Q\C2\E4\93D\D4\10\8B\D7TH\E44\0C\D1L-\86\86\03\00RA\18\01(\82\02\94!\00\802(\00P\06%\00\0A\11\00@P\82\A0\10\0A\10\00\C5P\00 \00\82\22\04\C0\1C\01(\14E\01\82 \08\00\00\A0J\A0\14\01\A0\CA\A0\00\D4@@\02\00s\04\C1\14\C0-\D2\14Q\C2\E4\03\8D\D3\A0\000\02\00\DC\F2DH3\15?\10\19\12\12\82\E0\1Ai\8A(a\F2\81\C6i|\1A\05\01\00\00\00Q\18\00\00\1C\01\00\00\1BF#\F8\FF\FF\FF\FF\01X\03\C0\14\00\07\80\03@\02\CA\800\07y\08\87v(\876\80\87wH\07w\A0\87r\90\07 \1C\D8\81\1D\80a\1D\CAA\1E\DC\A1\1C\D8\01 \DC\E1\1D\DA\80\1E\E4!\1C\E0\01\1E\D2\C1\1D\CE\A1\0D\DA!\1C\E8\01\1D\00z\90\87z(\07\80x\87v\80\87_\A0\87p\90\87s(\07z\F8\05w\A8\87v\F8\05z(\87ph\87y\00\E2\00 \DE\A1\1D\E0\E1\17\E8!\1C\E4\E1\1C\CA\81\1E~\81\1E\D0A\1E\CA!\1C\C8\E1\17\D8!\1D\DA!\1D\E8\01\90\03\80\98\07z\08\87qX\876\80\07yx\07z(\87q\A0\87w\90\876\10\87z0\07s(\07yh\83yH\07}(\07\00\0F\00\82\1E\C2A\1E\CE\A1\1C\E8\A1\0D\C6\01\1E\EA\01\98\87v\F8\85;\80\03\80\A0\87p\90\87s(\07zh\03s(\87p\A0\87z\90\87r\98\07`\0D\E0\81\1E\F0\E1\0E\F0\80\0D\D6`\1E\DA\E1\17\EE\00\0E\80\0D\86\10\00\09@\0A\1BLb\F8\FF\FF\FF\FF\01X\03\C0\01\E0\00\90\802 \CCA\1E\C2\A1\1D\CA\A1\0D\E0\E1\1D\D2\C1\1D\E8\A1\1C\E4\01\08\07v`\07\80p\87wh\03z\90\87p\80\07xH\07w8\876h\87p\A0\07t\00\E8A\1E\EA\A1\1C\00b\1E\E8!\1C\C6a\1D\DA\00\1E\E4\E1\1D\E8\A1\1C\C6\81\1E\DEA\1E\DA@\1C\EA\C1\1C\CC\A1\1C\E4\A1\0D\E6!\1D\F4\A1\1C\00<\00\08z\08\07y8\87r\A0\876\18\07x\A8\07`\1E\DA\E1\17\EE\00\0E\00\82\1E\C2A\1E\CE\A1\1C\E8\A1\0D\CC\A1\1C\C2\81\1E\EAA\1E\CAa\1E\805\80\07z\C0\87;\C0\036X\83yh\87_\B8\038\006\18\84\00$\00)l \89\E2\FF\FF\FF\FF\07\C0\01\E0\00\90\802 \CCA\1E\C2\A1\1D\CA\A1\0D\E0\E1\1D\D2\C1\1D\E8\A1\1C\E4\01\08\07v`\07\80p\87wh\03z\90\87p\80\07xH\07w8\876h\87p\A0\07t\00\E8A\1E\EA\A1\1C\00b\1E\E8!\1C\C6a\1D\DA\00\1E\E4\E1\1D\E8\A1\1C\C6\81\1E\DEA\1E\DA@\1C\EA\C1\1C\CC\A1\1C\E4\A1\0D\E6!\1D\F4\A1\1C\00<\00\08z\08\07y8\87r\A0\876\18\07x\A8\07`\1E\DA\E1\17\EE\00\0E\00\82\1E\C2A\1E\CE\A1\1C\E8\A1\0D\CC\A1\1C\C2\81\1E\EAA\1E\CAa\1E\805\80\07z\C0\87;\C0\036X\83yh\87_\B8\038\006\90\84\F1\FF\FF\FF\FF\03\B0\06\80\03H@\19\10\E6 \0F\E1\D0\0E\E5\D0\06\F0\F0\0E\E9\E0\0E\F4P\0E\F2\00\84\03;\B0\03@\B8\C3;\B4\01=\C8C8\C0\03<\A4\83;\9CC\1B\B4C8\D0\03:\00\F4 \0F\F5P\0E\001\0F\F4\10\0E\E3\B0\0Em\00\0F\F2\F0\0E\F4P\0E\E3@\0F\EF \0Fm \0E\F5`\0E\E6P\0E\F2\D0\06\F3\90\0E\FAP\0E\00\1E\00\04=\84\83<\9CC9\D0C\1B\8C\03<\D4\030\0F\ED\F0\0Bw\00\07\00A\0F\E1 \0F\E7P\0E\F4\D0\06\E6P\0E\E1@\0F\F5 \0F\E50\0F\C0\1A\C0\03=\E0\C3\1D\E0\01\1B\AC\C1<\B4\C3/\DC\01\1C\00\1B\98#\00H\018\83  \824 6\F8\03\F2\FF\FF\FF\FF\03\B0\06\849\C8C8\B4C9\B4\01<\BCC:\B8\03=\94\83<\00\E1\C0\0E\EC\00\10\EE\F0\0Em@\0F\F2\10\0E\F0\00\0F\E9\E0\0E\E7\D0\06\ED\10\0E\F4\80\0E\00=\C8C=\94\03@\CC\03=\84\C38\ACC\1B\C0\83<\BC\03=\94\C38\D0\C3;\C8C\1B\88C=\98\839\94\83<\B4\C1<\A4\83>\94\03\80\07\00A\0F\E1 \0F\E7P\0E\F4\D0\06\E3\00\0F\F5\00\CCC;\FC\C2\1D\C0\01@\D0C8\C8\C39\94\03=\B4\819\94C8\D0C=\C8C9\CC\03\B0\06\F0@\0F\F8p\07x\C0\06k0\0F\ED\F0\0Bw\00\07\C0\06\22\11\00R\D8`(\03\90\00\A4\B0\81X\FE\FF\FF\FF\7F\00\A4\0D\04\F3\FF\FF\FF\FF\03 lH\9A\FF\FF\FF\FF\1F\80?\00\A4`\0E\FE\FF\FF\FF\7F\D8@8\00\90l0\9E\FF\FF\FF\FF\1F\80?\00\A4\0D\06\14\00\B1\00,\1B\88\E8\FF\FF\FF\FF\07`\0D\00I\18\00\00\0A\00\00\00\13\84@\980\0C\021!(&\0C\C6AL\08\90\09\84q$\CA\84`\99\100\13\84\C6\99 <\D0\84 \02\00\00\00\13&w\B0\07x\A0\07|\B0\03:h\03w\B0\87t \87t\08\876\18\87z \87p\D8\90\06\E5\D0\06\E9`\07t\A0\07v@\07m\90\0Eq \07x\A0\07q \07x\D0\06\F6\10\07v\A0\07q`\07m`\0Fs \07z0\07r\D0\06\EE\10\07v\A0\07s \07z`\07t\A0\F4\80\10!!d\C8H\91\11@#\84a\8D\89\90&_\D8A\01T0d\11\00\04\00\00\00\00\10P\00\00\EC\A0\A83\08\86,\02\80\00\00\00\00\00\02\0A\00\80\1D\D4\1E\A4A0d\11\00\04\00\00\00\00\10P\00\00\EC\A0\C8\01\0C\82!\8B\00 \00\00\00\00\80\80\02\00`\07E\12f\10\0CY\04\00\01\00\00\00\00\04\14\00\00;(\0A\18\06-\09\00\00\08\00\00\00\02\0A\00\80\1D\14\05\10C\97\10\00\00\04\00\00\00\01\05\00\C0\0EJ-\EC\C0\10\BC\08\00\04\00\00\00\00\08(\00\00vP\E0Q\07\C8@\06\11\00\08\00\00\00\00\10P\00\00\EC\A0j\84H\862H\02\00\00\02\00\00\80\80\02\00`H\95#b\B0\00@\14\00\80\00\00\00\00\00@\00P\C0\90jO$\06\08\00\00\00\00\00\00\00\00\00\00\80\02\86T\AD\F2\06\0D\00\0C\02\00\00\00\00\00\00\00\04\00\05\0C\A9\F8\05\0F\1C\00\18\06\00\00\00\00\00\00\00\08\00\0A\18R\D1\CD\D3\00\C0@\00\00\00\00\00\00\00@\00P\C0\90jt\A4\07\08\00\00\00\00\00\00\00\00\00\00\80\02\86T\B5#A@\00\14\00\00\00\00\00\00\00\00\00\140\A4:\1F(\02\80\C1\00\00\00\00\00\00\00\80\00\A0\80!\D5\FD8\12\10\00\00\00\00\00\00\00\00\00\00\00\05\0C\A9J\08\9B\80\008\00\00\00\00\00\00\00\00\00(`H\D5CR\05\04\00\02\00\00\00\00\00\00\00\00@\01C\AA7\A2, \00\00\00\00\00\00\00\00\00\00\00\0A\18R\8D\D2t\01\01\90\00\00\00\00\00\00\00\00\00P\C0\90\CA\96$\0C\08\00\05\00\00\00\00\00\00\00\00\80\02$6\08\14V(\00\00\C8\02\01\1A\00\00\002\1E\98\18\19\11L\90\8C\09&G\C6\04C\0AF\00hPkWw\AF\DD\EF\BB{Ww\AF\DD\EF\BB{\07\DB\C1v;\0A\8A\A0X\88\C0\02\D3\B0\AD c\04\80\84\11\80\12(\02\0AJ\80\0EL\BC\0F\0C\B50\C40Bp\10\8D\83\12MP\A54\0BTJ\A7\14\14\14h@\F9\BF\E7C\14C!\94C\99\15\04\00\B1\18\00\00\C1\00\00\003\08\80\1C\C4\E1\1Cf\14\01=\88C8\84\C3\8CB\80\07yx\07s\98q\0C\E6\00\0F\ED\10\0E\F4\80\0E3\0CB\1E\C2\C1\1D\CE\A1\1Cf0\05=\88C8\84\83\1B\CC\03=\C8C=\8C\03=\CCx\8Ctp\07{\08\07yH\87pp\07zp\03vx\87p \87\19\CC\11\0E\EC\90\0E\E10\0Fn0\0F\E3\F0\0E\F0P\0E3\10\C4\1D\DE!\1C\D8!\1D\C2a\1Ef0\89;\BC\83;\D0C9\B4\03<\BC\83<\84\03;\CC\F0\14v`\07{h\077h\87rh\077\80\87p\90\87p`\07v(\07v\F8\05vx\87w\80\87_\08\87q\18\87r\98\87y\98\81,\EE\F0\0E\EE\E0\0E\F5\C0\0E\EC0\03b\C8\A1\1C\E4\A1\1C\CC\A1\1C\E4\A1\1C\DCa\1C\CA!\1C\C4\81\1D\CAa\06\D6\90C9\C8C9\98C9\C8C9\B8\C38\94C8\88\03;\94\C3/\BC\83<\FC\82;\D4\03;\B0\C3\0C\C7i\87pX\87rp\83th\07x`\87t\18\87t\A0\87\19\CES\0F\EE\00\0F\F2P\0E\E4\90\0E\E3@\0F\E1 \0E\ECP\0E3 (\1D\DC\C1\1E\C2A\1E\D2!\1C\DC\81\1E\DC\E0\1C\E4\E1\1D\EA\01\1Ef\18Q8\B0C:\9C\83;\CCP$v`\07{h\077`\87wx\07x\98QL\F4\90\0F\F0P\0E3\1Ej\1E\CAa\1C\E8!\1D\DE\C1\1D~\01\1E\E4\A1\1C\CC!\1D\F0a\06T\85\838\CC\C3;\B0C=\D0C9\FC\C2<\E4C;\88\C3;\B0\C3\8C\C5\0A\87y\98\87w\18\87t\08\07z(\07r\98\81\\\E3\10\0E\EC\C0\0E\E5P\0E\F30#\C1\D2A\1E\E4\E1\17\D8\E1\1D\DE\01\1EfH\19;\B0\83=\B4\83\1B\84\C38\8CC9\CC\C3<\B8\C19\C8\C3;\D4\03<\CCH\B4q\08\07v`\07q\08\87qX\87\19\DB\C6\0E\EC`\0F\ED\E0\06\F0 \0F\E50\0F\E5 \0F\F6P\0En\10\0E\E30\0E\E50\0F\F3\E0\06\E9\E0\0E\E4P\0E\F80#\E2\ECa\1C\C2\81\1D\D8\E1\17\EC!\1D\E6!\1D\C4!\1D\D8!\1D\E8!\1Ff \9D;\BCC=\B8\039\94\839\CCX\BCpp\07wx\07z\08\07zH\87wp\87\19\CB\E7\0E\EF0\0F\E1\E0\0E\E9@\0F\E9\A0\0F\E50\C3\01\03s\A8\07w\18\87_\98\87pp\87t\A0\87t\D0\87r\98\81\84A9\E0\C38\B0C=\90C9\CC@\C4\A0\1D\CA\A1\1D\E0A\1E\DE\C1\1Cf$c0\0E\E1\C0\0E\EC0\0F\E9@\0F\E50C!\83u\18\07sH\87_\A0\87|\80\87r\98\B1\94\01<\8C\C3<\94\C38\D0C:\BC\83;\CC\C3\8C\C5\0CH!\15Ba\1E\E6!\1D\CE\C1\1DR\81\14fLg0\0E\EF \0F\EF\E0\06\EFP\0F\F40\0F\E9@\0E\E5\E0\06\E6 \0F\E1\D0\0E\E5\00\00\00y \00\00m\00\00\00r\1EH C\88\0C\19\09r2H #\81\8C\91\91\D1D\A0\10(d<12B\8E\90!\A3\B80\B5\01\0E\82\18\83b4I\F3\0E\00\00\00maxclusterrankminctasmmaxntidxkernelfoowchar_sizeopenmpopenmp-devicePIC Levelframe-pointerclang version 19.0.0git (https://github.com/efwright/llvm-project.git 173ea981b52157e84bb8ff7f7b3d694b810e7a0f)\00\00\00#\08\802\82\10\84\C1\08B\A0\8D \04\D8\08BP\06#\08\81\19\8C \04g0\82\10\A0\C1\08B\90\06#\08\81\1A\8C \04k0\C3\C0\04\CD\0C\03#43\0C\CC\E0\CC00D3\C3\F1@Q!=\CF\0CCcL3\0C\D4Q\CD0PH5\C3`%\CE\0C\03\A583\04\8B\8C\04&(!7;\BB6\9707\B77\BA0\BA\B47\B7\B9Q\0A\EB\C2\B2,\A1\B76\B8\AF733\B6\B702\9747\B3\B7Q\0AM\D34-#66\BB6\97\B67\B2:\B62\173\B6\B0\B3\B9Q\8A\8D\EB\BC/\1566\BB6\974\B227\BAQ\020\00\00\00\A9\18\00\00-\00\00\00\0B\0Ar(\87w\80\07zXp\98C=\B8\C38\B0C9\D0\C3\82\E6\1C\C6\A1\0D\E8A\1E\C2\C1\1D\E6!\1D\E8!\1D\DE\C1\1D\164\E3`\0E\E7P\0F\E1 \0F\E4@\0F\E1 \0F\E7P\0E\F4\B0\80\81\07y(\87p`\07vx\87q\08\07z(\07rXp\9C\C38\B4\01;\A4\83=\94\C3\02k\1C\D8!\1C\DC\E1\1C\DC \1C\E4a\1C\DC \1C\E8\81\1E\C2a\1C\D0\A1\1C\C8a\1C\C2\81\1D\D8a\C1\01\0F\F4 \0F\E1P\0F\F4\80\0E\0B\88u\18\07sH\87\05\CF8\BC\83;\D8C9\C8\C39\94\83;\8CC9\8C\03=\C8\03;\00\00\00\00\D1\10\00\00\06\00\00\00\07\CC<\A4\83;\9C\03;\94\03=\A0\83<\94C8\90\C3\01\00\00\00a \00\000\00\00\00\13\04C,\10\00\00\00\03\00\00\00\14\D4\00\01#\00D\8D\00\00\00\003\11@\10\06\A40\13\A1\05a@\0A\C3\06\84`\10\C0\88\81\01\80 \180g\A0\06\C6pC`\80\C1,C \04#\06\05\00\82`0\85\81\1B\8C\184\00\08\82A\15\06o\10P\1B\92\06\07a\8C\18\10\00\08\82\81\05\06\BB\E1@\00\00\14\00\00\00\C6r\0CX\F3?St\01\0F%\11\D1/8\03\E1\13\CD\14a6\E3\10\17!\FC\94DD\BF\E0\0C\84\C5L\02\F0LTD\0C\7F\05D\D2\0F\0CC$\19\0C3`\CD\FFL\91\0F\0CCt\02\8B\E5D\0A\11\F9\C4\85L\07P\10\CD\14a\00\00\00\00\00\00a \00\00h\00\00\00\13\04E,\10\00\00\00\02\00\00\00D\15HA\14\02\00\00#\06\05\00\82`pm\C3\88A\01\80 \18\\\DC0bP\00 \08\06WG\8C\18\14\00\08\82\C1\E5\11#\06\05\00\82`p}\C6\88A\01\80 \18\\``\8C\18\14\00\08\82\C1\15\06\C7\B0\01a0\040l@\14\0B\01\CC\12\04\C3\06\04\01\06\03\E0\BBa\00\030\186 \82\82\00F\0C\0C\00\04\C1\A0Y\83\81\A0!\18\B3\04\82\F3\86D\0C\C4`\D8\80\08\08\02p\DE\A0\8C\816l@\04\08\01\8C\18$\00\08\82\81\A4\06v\F0\06\C3\22\06\B3\04\C3,\011b`\00 \08\06X\19\14\CC\88\81\01\80 \18`e`0#\06\06\00\82`\80\95\C1\D1\8C\18\18\00\08\82\01V\06\083b`\00 \08\06X\19$\CD\88\81\01\80 \18`e\A04#\06\06\00\82`\80\95\C1\E2l8\10\1D\00\00\00\C6\C2\F8\C6\E2\04\C0\F2O\C8\E0[\8E\E1\0B\0E\D5L\B6S\18\C4\F3#'\808\CC\F3/\8E\F3\1C\C1\E40\8F\BF8\CE\E3\13\172\F9\D2\B3 \93\E9D\06\F1\FC\80a\F8\02\F0LTD\0C\F5\01\14D3E\D8\0DX\0E\F3\F8\8B\E3<\BECM\0B\D2\10\83O\\\C8d-\8B\1F8T3\FC\132\F8\17\109\CC\E3/\8E\F3\F8\03\22M@#\10\00\00!1\00\00\09\00\00\00\0B\8A\00\18\04mA!\00\C1`, \00!\B0\83\05\04 \04u\B0\A0\00\06!\A0\83\05\050\08A\1C\00\00\00\00\00a \00\00N\00\00\00\13\04\85\F3\86\C0\B2(\00\88\F3\86\01{(\00\C8L\C4\17\84\01)\CCD\04A\18\8C\82\07\87\82\E3\88\08\C6\B0\01!\04\030K\10\8C\18\1C\00\08\82AT\06\83r\8C\18\10\0B\08\82\01d\06D\04\C3\7F\83\01\06`0l@\04\C3\00\F8o8\C2\00\1B6 \82a\00F\0C\0E\00\04\C1\E0Y\838@\9EY\02a\96`\98% 6\1C\08\00\00\00*\00\00\00\F6R-\0E0\18\C4\F3\03\86\E1\0B\C03Q\111\D4fC\08\C0\B2\\\C0\E30\8F\BF8\CE\E3G\84\818\CD\0DT\0E\F3\F8\8B\E3<\FE\13\11\7F\814\C0\82d\84\CD$\88\BF8\CES\01\D1\11|\0E\F3\F8\8B\E3<\BECM\0B\D2\10\83O\\\C8\E4\13\172\B1\13\15\98Kd\10\CF\0F\18\86/\00\CFDE\C4P\1BLa\10\CF\8FXL\B28\C0`\10\CF\8F\1C\80\E30\8F\BF8\CE\E3\13\CD\14a'\A09\CC\E3/\8E\F3\F8\11a N\E3K\083\F8@1\11\91\B1\18\0E\F3\F8He2\C3\C4<\00\00\00!1\00\00\03\00\00\00\0B\86\00\98\83\05C\00\BC\01\00\00\00\00\00\00a \00\00G\00\00\00\13\04D,\10\00\00\00\01\00\00\00\14\14\05\003\11@\10\06\A40\13\01\04a@\0A3\11A\10\06\A30\13\11\04a0\0A3\11A\10\06\A30l@\14\08\01\0C\1B\10\C4A\00D\00\C4wC\D0u\14\00\84\82`\0C\1B\10G0\00\C3\06\84\A1\0C\C0\B0\01Q\\\03@G0\E8\08\C6p\83\10\A0\C1,C \04\94\04\83\96`\A0\10\84\00-\C1\B8\C0\8B\1B\02 '\18'\043K0\CC\12\0C\03\15\81\10\D8\82@\15@\86\0D\88@\18\80\0D\07\02\00\00\00\1F\00\00\00\064\00\C3`-\C9\FF\0BN3\11\D7t\02\8A\E04\83_\00\8BD\18\0B\D2!\D2\044\02a?\84D\05\B5\BD4\1D\22M@#\10>0\0C\91\09\0D\03R\1D@A4S\84\19\8C\F3\FF\82\D3L\C45\F9\C00D\17\90\08N3\F8SD\116S\F8\D2D<\D63HT`1\86/M@4\99\CE 0\CF\0D \82\D3\0C>\D1\0C&S\F8\D2\E4<6D\08N3\00\00\00\00\00\00a \00\003\00\00\00\13\04\C1L\04\10\84\01)\CCD\04A\18\8C\C2L\04\10\84\01)\0C\1B\10\83A\00\C3\06\84P\0C\C0\B0\01\11\10\04@\01@<8\04\DBFA0\A8\08\C6U\81\DC\10\00%\00\196 \02a\006\1C\08\13\00\00\00v3\00\C3`1\CE\FF\0BN3\11\D7\E4\03\C3\10\99\CD\C0P\8B\BD0\A5\E3</\10\F9\C00D\E6\92\FC\BF\E04\13qM\D6r\94\8E\81\08\C0b0L\E9\18\88\00,>0\0C\D1\01\14D3E\98\B1\1C\A5\E3</\10\01\00!1\00\00\0B\00\00\00\0B:\E3h\9CGY\18$\19\88\02\08\04(\926-\C3\AE\CA\9A(l\C1E\14\C6\B10\8D\F3@Q\82(\83\00\04\954Qa\00\00\00\00\00q \00\00\06\00\00\002\0E\10\22\84\0A\A0\07\C8 }\80\0D\BC\08\E8\C0\98\80\11\D5\09\00\00\00\00\00\00e\0C\00\00\A9\00\00\00\12\03\948\05\00\00\00\03\00\00\00h\02\00\002\00\00\00L\00\00\00\01\00\00\00X\00\00\00\00\00\00\00X\00\00\00\18\00\00\00\98\02\00\00\00\00\00\00\9A\02\00\00\13\00\00\00\AD\02\00\00\06\00\00\00\14\00\00\00\00\00\00\00\98\02\00\00\00\00\00\00\00\00\00\00\18\00\00\00\00\00\00\00\19\01\00\00\22\00\00\00\19\01\00\00\22\00\00\00\FF\FF\FF\FF\12$\00\00;\01\00\00\12\00\00\00;\01\00\00\12\00\00\00\FF\FF\FF\FF\08$\00\00M\01\00\00/\00\00\00M\01\00\00/\00\00\00\FF\FF\FF\FF\00 \00\00|\01\00\008\00\00\00|\01\00\008\00\00\00\FF\FF\FF\FF\00 \00\00\B4\01\00\00\0F\00\00\00\B4\01\00\00\0F\00\00\00\FF\FF\FF\FF\00 \00\00\C3\01\00\00\12\00\00\00\C3\01\00\00\12\00\00\00\FF\FF\FF\FF\08$\00\00\D5\01\00\00\12\00\00\00\D5\01\00\00\12\00\00\00\FF\FF\FF\FF\08$\00\00\E7\01\00\00\10\00\00\00\E7\01\00\00\10\00\00\00\FF\FF\FF\FF\00 \00\00\F7\01\00\00\0E\00\00\00\F7\01\00\00\0E\00\00\00\FF\FF\FF\FF\08$\00\00\05\02\00\00\18\00\00\00\05\02\00\00\18\00\00\00\FF\FF\FF\FF\08$\00\00\1D\02\00\00\12\00\00\00\1D\02\00\00\12\00\00\00\FF\FF\FF\FF\08$\00\00/\02\00\00\14\00\00\00/\02\00\00\14\00\00\00\FF\FF\FF\FF\08$\00\00C\02\00\00\13\00\00\00C\02\00\00\13\00\00\00\FF\FF\FF\FF\08$\00\00V\02\00\00\12\00\00\00V\02\00\00\12\00\00\00\FF\FF\FF\FF\08$\00\00\00\00\00\00\14\00\00\00\00\00\00\00\14\00\00\00\FF\FF\FF\FF\11\04\00\00\14\00\00\00'\00\00\00\14\00\00\00'\00\00\00\FF\FF\FF\FF\11\04\00\00;\00\00\00)\00\00\00;\00\00\00)\00\00\00\FF\FF\FF\FF\11\04\00\00d\00\00\00 \00\00\00d\00\00\00 \00\00\00\FF\FF\FF\FF\11\04\00\00\84\00\00\00&\00\00\00\84\00\00\00&\00\00\00\FF\FF\FF\FF\11\04\00\00\B3\02\00\00\0B\00\00\00\14\00\00\00\00\00\00\00\FF\FF\FF\FF\80\18\00\00\BE\02\00\00\0B\00\00\00\14\00\00\00\00\00\00\00\FF\FF\FF\FF\80\18\00\00\AA\00\00\006\00\00\00\AA\00\00\006\00\00\00\FF\FF\FF\FF\12\04\00\00\E0\00\00\005\00\00\00\E0\00\00\005\00\00\00\FF\FF\FF\FF\12\04\00\00\15\01\00\00\04\00\00\00\15\01\00\00\04\00\00\00\FF\FF\FF\FF\00\18\00\00\00\00\00\00]\0C\00\00\B6\00\00\00\12\03\94\A9\05\00\00\00__omp_rtl_debug_kind__omp_rtl_assume_teams_oversubscription__omp_rtl_assume_threads_oversubscription__omp_rtl_assume_no_thread_state__omp_rtl_assume_no_nested_parallelism__omp_offloading_50_10f3bff_foo_l6_dynamic_environment__omp_offloading_50_10f3bff_foo_l6_kernel_environment.str__omp_offloading_50_10f3bff_foo_l6__kmpc_target_init__omp_offloading_50_10f3bff_foo_l6_omp_outlined__omp_offloading_50_10f3bff_foo_l6_omp_outlined..omp_par__captured_stmt__llvm_omp_vprintfomp_get_thread_num__captured_stmt1__kmpc_simd_4u__kmpc_global_thread_num__kmpc_parallel_51__kmpc_target_deinit__kmpc_alloc_shared__kmpc_free_shared19.0.0git 173ea981b52157e84bb8ff7f7b3d694b810e7a0fnvptx64-nvidia-cudatest.c__unnamed_1__unnamed_2\00\00\00\00\00\00\00", section ".llvm.offloading", align 8, !exclude !0
@llvm.compiler.used = appending global [1 x ptr] [ptr @llvm.embedded.object], section "llvm.metadata"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %kernel_args = alloca %struct.__tgt_kernel_arguments, align 8
  %0 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 0
  store i32 2, ptr %0, align 4
  %1 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 1
  store i32 0, ptr %1, align 4
  %2 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 2
  store ptr null, ptr %2, align 8
  %3 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 3
  store ptr null, ptr %3, align 8
  %4 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 4
  store ptr null, ptr %4, align 8
  %5 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 5
  store ptr null, ptr %5, align 8
  %6 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 6
  store ptr null, ptr %6, align 8
  %7 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 7
  store ptr null, ptr %7, align 8
  %8 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 8
  store i64 0, ptr %8, align 8
  %9 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 9
  store i64 0, ptr %9, align 8
  %10 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 10
  store [3 x i32] [i32 1, i32 0, i32 0], ptr %10, align 4
  %11 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 11
  store [3 x i32] [i32 2, i32 0, i32 0], ptr %11, align 4
  %12 = getelementptr inbounds %struct.__tgt_kernel_arguments, ptr %kernel_args, i32 0, i32 12
  store i32 0, ptr %12, align 4
  %13 = call i32 @__tgt_target_kernel(ptr @1, i64 -1, i32 1, i32 2, ptr @.__omp_offloading_50_10f3bff_foo_l6.region_id, ptr %kernel_args)
  %14 = icmp ne i32 %13, 0
  br i1 %14, label %omp_offload.failed, label %omp_offload.cont

omp_offload.failed:                               ; preds = %entry
  call void @__omp_offloading_50_10f3bff_foo_l6() #3
  br label %omp_offload.cont

omp_offload.cont:                                 ; preds = %omp_offload.failed, %entry
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define internal void @__omp_offloading_50_10f3bff_foo_l6() #1 {
entry:
  %0 = call i32 @__kmpc_global_thread_num(ptr @1)
  call void @__kmpc_push_num_threads(ptr @1, i32 %0, i32 2)
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr @1, i32 0, ptr @__omp_offloading_50_10f3bff_foo_l6.omp_outlined)
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define internal void @__omp_offloading_50_10f3bff_foo_l6.omp_outlined(ptr noalias noundef %.global_tid., ptr noalias noundef %.bound_tid.) #1 {
entry:
  %.global_tid..addr = alloca ptr, align 8
  %.bound_tid..addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %.global_tid., ptr %.global_tid..addr, align 8
  store ptr %.bound_tid., ptr %.bound_tid..addr, align 8
  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %0 = load i32, ptr %.omp.iv, align 4, !llvm.access.group !10
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %1 = load i32, ptr %.omp.iv, align 4, !llvm.access.group !10
  %mul = mul nsw i32 %1, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4, !llvm.access.group !10
  %call = call signext i32 @omp_get_thread_num(), !llvm.access.group !10
  %2 = load i32, ptr %i, align 4, !llvm.access.group !10
  %call1 = call signext i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef signext %call, i32 noundef signext %2), !llvm.access.group !10
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %3 = load i32, ptr %.omp.iv, align 4, !llvm.access.group !10
  %add2 = add nsw i32 %3, 1
  store i32 %add2, ptr %.omp.iv, align 4, !llvm.access.group !10
  br label %omp.inner.for.cond, !llvm.loop !11

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  store i32 10, ptr %i, align 4
  ret void
}

declare signext i32 @printf(ptr noundef, ...) #2

declare signext i32 @omp_get_thread_num() #2

; Function Attrs: nounwind
declare signext i32 @__kmpc_global_thread_num(ptr) #3

; Function Attrs: nounwind
declare void @__kmpc_push_num_threads(ptr, i32 signext, i32 signext) #3

; Function Attrs: nounwind
declare !callback !14 void @__kmpc_fork_call(ptr, i32 signext, ptr, ...) #3

; Function Attrs: nounwind
declare signext i32 @__tgt_target_kernel(ptr, i64, i32 signext, i32 signext, ptr, ptr) #3

; Function Attrs: noinline nounwind optnone uwtable
define dso_local signext i32 @main() #0 {
entry:
  call void @foo()
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ppc64le" "target-features"="+altivec,+bpermd,+crbits,+crypto,+direct-move,+extdiv,+htm,+isa-v206-instructions,+isa-v207-instructions,+power8-vector,+vsx,-aix-small-local-exec-tls,-isa-v30-instructions,-power9-vector,-privileged,-quadword-atomics,-rop-protect,-spe" }
attributes #1 = { noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ppc64le" "target-features"="+altivec,+bpermd,+crbits,+crypto,+direct-move,+extdiv,+htm,+isa-v206-instructions,+isa-v207-instructions,+power8-vector,+vsx,-aix-small-local-exec-tls,-isa-v30-instructions,-power9-vector,-privileged,-quadword-atomics,-rop-protect,-spe" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ppc64le" "target-features"="+altivec,+bpermd,+crbits,+crypto,+direct-move,+extdiv,+htm,+isa-v206-instructions,+isa-v207-instructions,+power8-vector,+vsx,-aix-small-local-exec-tls,-isa-v30-instructions,-power9-vector,-privileged,-quadword-atomics,-rop-protect,-spe" }
attributes #3 = { nounwind }

!omp_offload.info = !{!1}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}
!llvm.embedded.objects = !{!9}

!0 = !{}
!1 = !{i32 0, i32 80, i32 17775615, !"foo", i32 6, i32 0, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"openmp", i32 51}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 19.0.0git (https://github.com/efwright/llvm-project.git 173ea981b52157e84bb8ff7f7b3d694b810e7a0f)"}
!9 = !{ptr @llvm.embedded.object, !".llvm.offloading"}
!10 = distinct !{}
!11 = distinct !{!11, !12, !13}
!12 = !{!"llvm.loop.parallel_accesses", !10}
!13 = !{!"llvm.loop.vectorize.enable", i1 true}
!14 = !{!15}
!15 = !{i64 2, i64 -1, i64 -1, i1 true}
