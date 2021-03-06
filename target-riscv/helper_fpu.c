/*
 * RISC-V floating-point helpers
 *
 * Copyright (c) 2016 Alex Suykov <alex.suykov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "exception.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"

#define ENV CPURISCVState *env      /* CPU state environment */
#define FPS &env->fpstatus          /* FP status (softfloat internal state) */
#define RM uint32_t rm

#ifdef TARGET_RISCV32
#define gpv uint32_t                /* RV32 GPR */
#else
#define gpv uint64_t                /* RV64 GPR */
#endif
#define fpv uint64_t                /* FPR value type common for both */

/* All FP ops that round values have rounding mode encoded in insn.
   For softfloat, it's a global setting instead of per-op parameter,
   so each RISC-V op starts with a call to set_rounding_mode.

   The value of 111 means "use default", which is env->frm.
   Per spec, writing illegal values to frm is allowed, and exception
   should only be raised on the first FP op that tries to use that
   value. */

static void set_rounding_mode(ENV, uint32_t rm)
{
    unsigned effective = (rm == /* 111 */ 7) ? env->frm : rm;
    unsigned char mode;

    switch(effective) {
        case /* 000 */ 0: mode = float_round_nearest_even; break;
        case /* 001 */ 1: mode = float_round_to_zero; break;
        case /* 010 */ 2: mode = float_round_down; break;
        case /* 011 */ 3: mode = float_round_up; break;
        case /* 100 */ 4: mode = float_round_ties_away; break;
        default: raise_exception(env, EXCP_ILLEGAL);
    }

    env->fpstatus.float_rounding_mode = mode;
}

/* 32-bit fused multiply-add */

fpv HELPER(fmadd_s)(ENV, fpv a, fpv b, fpv c, RM)
{
    int flags = 0;
    set_rounding_mode(env, rm);
    return float32_muladd(a, b, c, flags, FPS);
}

fpv HELPER(fmsub_s)(ENV, fpv a, fpv b, fpv c, RM)
{
    int flags = float_muladd_negate_c;
    set_rounding_mode(env, rm);
    return float32_muladd(a, b, c, flags, FPS);
}

fpv HELPER(fnmadd_s)(ENV, fpv a, fpv b, fpv c, RM)
{
    /* negate-result means -(a*b) in softfloat, not -(a*b + c) */
    int flags = float_muladd_negate_result | float_muladd_negate_c;
    set_rounding_mode(env, rm);
    return float32_muladd(a, b, c, flags, FPS);
}

fpv HELPER(fnmsub_s)(ENV, fpv a, fpv b, fpv c, RM)
{
    int flags = float_muladd_negate_result;
    set_rounding_mode(env, rm);
    return float32_muladd(a, b, c, flags, FPS);
}

/* 64-bit fused multiply-add */

fpv HELPER(fmadd_d)(ENV, fpv a, fpv b, fpv c, RM)
{
    int flags = 0;
    set_rounding_mode(env, rm);
    return float64_muladd(a, b, c, flags, FPS);
}

fpv HELPER(fmsub_d)(ENV, fpv a, fpv b, fpv c, RM)
{
    int flags = float_muladd_negate_c;
    set_rounding_mode(env, rm);
    return float64_muladd(a, b, c, flags, FPS);
}

fpv HELPER(fnmadd_d)(ENV, fpv a, fpv b, fpv c, RM)
{
    /* negate-result means -(a*b) in softfloat, not -(a*b + c) */
    int flags = float_muladd_negate_result | float_muladd_negate_c;
    set_rounding_mode(env, rm);
    return float64_muladd(a, b, c, flags, FPS);
}

fpv HELPER(fnmsub_d)(ENV, fpv a, fpv b, fpv c, RM)
{
    int flags = float_muladd_negate_result;
    set_rounding_mode(env, rm);
    return float64_muladd(a, b, c, flags, FPS);
}

/* 32-bit basic FP ops */

fpv HELPER(fadd_s)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float32_add(a, b, FPS);
}

fpv HELPER(fsub_s)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float32_sub(a, b, FPS);
}

fpv HELPER(fmul_s)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float32_mul(a, b, FPS);
}

fpv HELPER(fdiv_s)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float32_div(a, b, FPS);
}

fpv HELPER(fsqrt_s)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float32_sqrt(a, FPS);
}

/* 64-bit basic FP ops */

fpv HELPER(fadd_d)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float64_add(a, b, FPS);
}

fpv HELPER(fsub_d)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float64_sub(a, b, FPS);
}

fpv HELPER(fmul_d)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float64_mul(a, b, FPS);
}

fpv HELPER(fdiv_d)(ENV, fpv a, fpv b, RM)
{
    set_rounding_mode(env, rm);
    return float64_div(a, b, FPS);
}

fpv HELPER(fsqrt_d)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float64_sqrt(a, FPS);
}

/* 32-bit FP min/max and comparison */

fpv HELPER(fmin_s)(ENV, fpv a, fpv b)
{
    return float32_minnum(a, b, FPS);
}

fpv HELPER(fmax_s)(ENV, fpv a, fpv b)
{
    return float32_maxnum(a, b, FPS);
}

gpv HELPER(flt_s)(ENV, fpv a, fpv b)
{
    return float32_lt(a, b, FPS);
}

gpv HELPER(fle_s)(ENV, fpv a, fpv b)
{
    return float32_le(a, b, FPS);
}

gpv HELPER(feq_s)(ENV, fpv a, fpv b)
{
    return float32_eq(a, b, FPS);
}

/* 64-bit FP min/max and comparison */

fpv HELPER(fmin_d)(ENV, fpv a, fpv b)
{
    return float64_minnum(a, b, FPS);
}

fpv HELPER(fmax_d)(ENV, fpv a, fpv b)
{
    return float64_maxnum(a, b, FPS);
}

gpv HELPER(flt_d)(ENV, fpv a, fpv b)
{
    return float64_lt(a, b, FPS);
}

gpv HELPER(fle_d)(ENV, fpv a, fpv b)
{
    return float64_le(a, b, FPS);
}

gpv HELPER(feq_d)(ENV, fpv a, fpv b)
{
    return float64_eq(a, b, FPS);
}

/* Single-Double conversion */

fpv HELPER(fcvt_s_d)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float64_to_float32(a, FPS);
}

fpv HELPER(fcvt_d_s)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm); /* pointelss */
    return float32_to_float64(a, FPS);
}

/* 32-bit FP-integer conversion */

gpv HELPER(fcvt_w_s)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float32_to_int32(a, FPS);
}

gpv HELPER(fcvt_wu_s)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    /* sign-extend for RV64 */
    return (int32_t)float32_to_uint32(a, FPS);
}

fpv HELPER(fcvt_s_w)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return int32_to_float32(a, FPS);
}

fpv HELPER(fcvt_s_wu)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return uint32_to_float32(a, FPS);
}

#ifndef TARGET_RISCV32

gpv HELPER(fcvt_l_s)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float32_to_int64(a, FPS);
}

gpv HELPER(fcvt_lu_s)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float32_to_uint64(a, FPS);
}

fpv HELPER(fcvt_s_l)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return int64_to_float32(a, FPS);
}

fpv HELPER(fcvt_s_lu)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return uint64_to_float32(a, FPS);
}

#endif

/* 64-bit FP-integer conversion */

gpv HELPER(fcvt_w_d)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float64_to_int32(a, FPS);
}

gpv HELPER(fcvt_wu_d)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    /* sign-extend for RV64 */
    return (int32_t)float64_to_uint32(a, FPS);
}

fpv HELPER(fcvt_d_w)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return int32_to_float64(a, FPS);
}

fpv HELPER(fcvt_d_wu)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return uint32_to_float64(a, FPS);
}

#ifndef TARGET_RISCV32

gpv HELPER(fcvt_l_d)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float64_to_int64(a, FPS);
}

gpv HELPER(fcvt_lu_d)(ENV, fpv a, RM)
{
    set_rounding_mode(env, rm);
    return float64_to_uint64(a, FPS);
}

fpv HELPER(fcvt_d_l)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return int64_to_float64(a, FPS);
}

fpv HELPER(fcvt_d_lu)(ENV, gpv a, RM)
{
    set_rounding_mode(env, rm);
    return uint64_to_float64(a, FPS);
}

#endif

/* Like float32_is_signaling_nan() with status->snan_bit_is_one clear.
   Bringing in ENV to pass a value that is a hard-wired constant for
   the arch sounds like a huge overkill. */

static bool float32_is_signaling_nan_(float32 a)
{
    return (((a >> 22) & 0x1FF) == 0x1FE) && (a & 0x003FFFFF);
}

static bool float32_is_quiet_nan_(float32 a)
{
    return ((uint32_t)(a << 1) >= 0xFF800000);
}

/* FCLASS helpers. The "functions" it calls are all short inline bit tests. */

gpv HELPER(fclass_s)(fpv a)
{
    if(float32_is_signaling_nan_(a))
        return (1<<8);
    if(float32_is_quiet_nan_(a))
        return (1<<9);

    if(float32_is_neg(a)) {
        if(float32_is_infinity(a))
            return (1<<0);      /* -inf */
        else if(float32_is_zero(a))
            return (1<<3);      /* -0 */
        else if(float32_is_zero_or_denormal(a))
            return (1<<2);      /* negative subnormal */
        else
            return (1<<1);      /* negative normal */
    } else {
        if(float32_is_infinity(a))
            return (1<<7);      /* +inf */
        else if(float32_is_zero(a))
            return (1<<4);      /* +0 */
        else if(float32_is_zero_or_denormal(a))
            return (1<<5);      /* positive subnormal */
        else
            return (1<<6);      /* positive normal */
    }
}

static bool float64_is_signaling_nan_(float64 a)
{
    return (((a >> 51) & 0xFFF) == 0xFFE) && (a & 0x0007FFFFFFFFFFFFLL);
}

static bool float64_is_quiet_nan_(float64 a)
{
    return ((a << 1) >= 0xFFF0000000000000ULL);
}

gpv HELPER(fclass_d)(fpv a)
{
    if(float64_is_signaling_nan_(a))
        return (1<<8);
    if(float64_is_quiet_nan_(a))
        return (1<<9);

    if(float64_is_neg(a)) {
        if(float64_is_infinity(a))
            return (1<<0);      /* -inf */
        else if(float64_is_zero(a))
            return (1<<3);      /* -0 */
        else if(float64_is_zero_or_denormal(a))
            return (1<<2);      /* negative subnormal */
        else
            return (1<<1);      /* negative normal */
    } else {
        if(float64_is_infinity(a))
            return (1<<7);      /* +inf */
        else if(float64_is_zero(a))
            return (1<<4);      /* +0 */
        else if(float64_is_zero_or_denormal(a))
            return (1<<5);      /* positive subnormal */
        else
            return (1<<6);      /* positive normal */
    }
}
