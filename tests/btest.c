// Program to test out new ioctl interface to /dev/cpu/0/msr_safe
#include <linux/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include "../msr.h"

#define RDMSR(m) {{ .d64 = 0 }, .mask=0, .msr=m, .isread=1, .errno=0}
struct msr_cpu_ops bundle[] = 
{
  { .cpu = 0, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 1, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 2, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 3, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 4, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 5, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 6, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 7, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 8, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 9, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 10, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 11, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  },
  { .cpu = 22, .n_ops = 33, 
    {
      RDMSR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
      //RDMSR(0x3B),	// IA32_TSC_ADJUST, thread
      RDMSR(0xE7),	// IA32_MPERF, varies (unique to thread)
      RDMSR(0xE8),	// IA32_APERF, varies (unique to thread)
      RDMSR(0x199),	// IA32_PERF_CTL, varies (unique to thread)
      RDMSR(0x19A),	// IA32_CLOCK_MODULATION, varies (unique to thread)
      RDMSR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
      RDMSR(0x19C),	// IA32_THERM_STATUS, all except thread
      RDMSR(0x19B),	// MSR_THERM2_CTL, unique and shared
      RDMSR(0x1A2),	// MSR_TEMPERATURE_TARGET, varies (pretty much all of them)
      RDMSR(0x309),	// IA32_FIXED_CTR0, unique, core and thread
      RDMSR(0x30A),	// IA32_FIXED_CTR1, unique, core and thread
      RDMSR(0x30B),	// IA32_FIXED_CTR2, unique, core and thread
      RDMSR(0x38D),	// IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread
      RDMSR(0xC1),	// IA32_PMC0, thread, core and unique
      RDMSR(0xC2),	// IA32_PMC1
      RDMSR(0xC3),	// IA32_PMC2
      RDMSR(0xC4),	// IA32_PMC3
      //RDMSR(0xC5),	// IA32_PMC4
      //RDMSR(0xC6),	// IA32_PMC5
      //RDMSR(0xC7),	// IA32_PMC6
      //RDMSR(0xC8),	// IA32_PMC7
      RDMSR(0x345),	// IA32_PERF_CAPABILITIES, unique, shared, core, and thread
      RDMSR(0x38E),	// IA32_PERF_GLOBAL_STATUS, thread, core, and unique
      RDMSR(0x38F),	// IA32_PERF_GLOBAL_CTRL, thread, core, and unique
      RDMSR(0x390),	// IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique
      //RDMSR(0x391),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      //RDMSR(0x392),	// MSR_UNCORE_PERF_GLOBAL_CTRL, package
      RDMSR(0x3F1),	// MSR_PEBS_ENABLE, thread, shared, unique, core
      RDMSR(0x606),	// MSR_RAPL_POWER_UNIT, package
      RDMSR(0x610),	// MSR_PKG_POWER_LIMIT, package
      RDMSR(0x611),	// MSR_PKG_ENERGY_STATUS, package
      //RDMSR(0x613),	// MSR_PKG_PERF_STATUS, package
      RDMSR(0x614),	// MSR_PKG_POWER_INFO, package
      RDMSR(0x618),	// MSR_DRAM_POWER_LIMIT, package
      RDMSR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
      RDMSR(0x61B),	// MSR_DRAM_PERF_STATUS, package
      RDMSR(0x61C),	// MSR_DRAM_POWER_INFO, package
      RDMSR(0x638),	// MSR_PP0_POWER_LIMIT, package
      RDMSR(0x639),	// MSR_PP0_ENERGY_STATUS, package
      //RDMSR(0x63A),	// MSR_PP0_POLICY, package
      //RDMSR(0x63B),	// MSR_PP0_PERF_STATUS, package
      //RDMSR(0x640),	// MSR_PP1_POWER_LIMIT, package
      //RDMSR(0x641),	// MSR_PP1_ENERGY_STATUS, package
      //RDMSR(0x642),	// MSR_PP1_POLICY, package
      //RDMSR(0x64C),	// MSR_TURBO_ACTIVATION_RATIO, package
      //RDMSR(0x66E),	// MSR_PKG_POWER_INFO, package
      //RDMSR(0x690),	// MSR_CORE_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B0),	// MSR_GRAPHICS_PERF_LIMIT_REASONS, package
      //RDMSR(0x6B1),	// MSR_RING_PERF_LIMIT_REASONS, package
      RDMSR(0xE7)	// IA32_MPERF, varies (unique to thread)
    }
  }
};

int main()
{
	int fd;
	struct msr_bundle_desc bd;
	int i, j;

	if ((fd = open("/dev/cpu/0/msr_safe", O_RDWR)) < 0) {
		perror("Failed to open /dev/cpu/0/msr_safe");
		_exit(1);
	}

	bd.n_msr_bundles = sizeof(bundle) / sizeof(bundle[0]);
	bd.bundle = bundle;

	if (ioctl(fd, X86_IOC_MSR_BATCH, &bd) < 0) {
		perror("Ioctl failed");
		for (i = 0; i < bd.n_msr_bundles; ++i) {
			for (j = 0; j < bundle[i].n_ops; ++j) {
				if (bundle[i].ops[j].errno) {
					fprintf(stderr, 
					    "CPU %d, %s %x, Err %d\n", 
						bundle[i].cpu,
						bundle[i].ops[j].isread ? 
							"RDMSR" : "WRMSR",
						bundle[i].ops[j].msr, 
						bundle[i].ops[j].errno);
				}
			}
		}
		_exit(1);
	}
	
	for (i = 0; i < bd.n_msr_bundles; ++i) {
		for (j = 0; j < bundle[i].n_ops; ++j) {
			if (bundle[i].ops[j].errno) {
				fprintf(stderr, "Huh? %s %x, Err %d\n",
				  bundle[i].ops[j].isread ? "RDMSR" : "WRMSR",
				     bundle[i].ops[j].msr, 
					bundle[i].ops[j].errno);
			}
			else {
				printf("CPU %d: %s %x Data %lld\n", 
				  bundle[i].cpu,
				  bundle[i].ops[j].isread ? "RDMSR" : "WRMSR",
					bundle[i].ops[j].msr, 
					bundle[i].ops[j].d.d64);
			}
		}
	}
	
	close(fd);
	return 0;
}
