/*
 * RISC-V virtual CPU header.
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

#ifndef CPU_RISCV_H
#define CPU_RISCV_H

#define TARGET_LONG_BITS 64
#define TARGET_VIRT_ADDR_SPACE_BITS 64
#define TARGET_PAGE_BITS 12
#define ELF_MACHINE EM_RISCV

#define MMU_USER_IDX 0

#define CPUArchState struct CPURISCVState

#include "qemu-common.h"
#include "exec/cpu-defs.h"
#include "qom/cpu.h"

#define TYPE_RISCV_CPU "riscv-cpu"

#define RISCV_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(RISCVCPUClass, (klass), TYPE_RISCV_CPU)
#define RISCV_CPU(obj) \
    OBJECT_CHECK(RISCVCPU, (obj), TYPE_RISCV_CPU)
#define RISCV_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(RISCVCPUClass, (obj), TYPE_RISCV_CPU)

typedef struct RISCVCPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    DeviceRealize parent_realize;
    void (*parent_reset)(CPUState *cpu);
} RISCVCPUClass;

typedef struct CPURISCVState {
    target_ulong pc;
    target_ulong sp;

    CPU_COMMON

} CPURISCVState;

typedef struct RISCVCPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPURISCVState env;
} RISCVCPU;

static inline RISCVCPU *riscv_env_get_cpu(CPURISCVState *env)
{
    return RISCV_CPU(container_of(env, RISCVCPU, env));
}

#define ENV_GET_CPU(e) CPU(riscv_env_get_cpu(e))
#define ENV_OFFSET offsetof(RISCVCPU, env)

void cpu_riscv_list(FILE *f, fprintf_function cpu_fprintf);
RISCVCPU *cpu_riscv_init(const char *cpu_model);
int cpu_riscv_signal_handler(int host_signum, void *pinfo, void *puc);

#define cpu_list cpu_riscv_list
#define cpu_init(cpu_model) CPU(cpu_riscv_init(cpu_model))
#define cpu_signal_handler cpu_riscv_signal_handler

#include "exec/cpu-all.h"

static inline void cpu_get_tb_cpu_state(CPURISCVState *env,
        target_ulong *pc, target_ulong *cs_base, uint32_t *flags)
{
    *pc = env->pc;
    *cs_base = 0;
    *flags = 0;
}

static inline int cpu_mmu_index(CPURISCVState *env)
{
    return 0;
}

static inline target_ulong cpu_get_pc(CPURISCVState *env)
{
    return env->pc;
}

#include "exec/exec-all.h"

static inline void cpu_pc_from_tb(CPURISCVState *env, TranslationBlock *tb)
{
    env->pc = tb->pc;
}

#endif /* CPU_RISCV_H */
