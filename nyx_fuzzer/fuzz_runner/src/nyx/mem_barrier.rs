use std::sync::atomic::compiler_fence;
use std::sync::atomic::Ordering;

// we expect this to be a nop.
// but in some extreme cases, this
/*
use std::sync::atomic::compiler_fence;
use std::sync::atomic::Ordering;

fn barrier() {
compiler_fence(Ordering::SeqCst);
}

pub fn read2(data: &mut u32) -> u32{
    let a = *data;
    barrier();
    let b = *data;
    return a.wrapping_add(b);
}
*/

//compiles to
/*
        mov     eax, dword ptr [rdi]
        add     eax, dword ptr [rdi]
        ret
*/
//while the second access gets optimized out without the barrier.
//To ensure that reads/writes to the shared memory buffer actually are executed, we use mem_barrier to lightweight synchronize the values.

pub fn mem_barrier() {
    compiler_fence(Ordering::SeqCst);
}
