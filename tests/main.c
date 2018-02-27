/*
 *  Copyright (c) 2016-2018 Positive Technologies, https://www.ptsecurity.com,
 *  Fast Positive Hash.
 *
 *  Portions Copyright (c) 2010-2018 Leonid Yuriev <leo@yuriev.ru>,
 *  The 1Hippeus project (t1h).
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgement in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 */

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned default_option_flags =
    bench_0 | bench_1 | bench_2 | bench_tiny | bench_medium;

const unsigned available_eas_flags =
#ifdef T1HA0_AESNI_AVAILABLE
    bench_aes | bench_avx | user_wanna_aes |
#ifndef __e2k__
    bench_avx2 |
#endif /* !__e2k__ */
#endif /* T1HA0_AESNI_AVAILABLE */
    0u;

const unsigned default_disabled_option_flags =
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    bench_le
#else
    bench_be
#endif /* BIG_ENDIAN */
    | ((UINTPTR_MAX > 0xffffFFFFul || ULONG_MAX > 0xffffFFFFul) ? bench_32 : 0);

unsigned option_flags, disabled_option_flags;

void usage(void) {
  printf(
      "By default                  - run reasonable tests and benchmarks\n"
      "                              for current platform\n"
      "Generic options:\n"
      "  --test-only, --no-bench   - just run tests, but don't benchmarking\n"
      "  --test-verbose            - be verbose while testing\n"
      "  --bench-verbose           - be verbose while benchmarking\n"
      "  --verbose                 - turn both --test-verbose\n"
      "                              and --bench-verbose\n"
      "Keys size choices:\n"
      "  --tiny, --no-tiny         - include/exclude 5 bytes, i.e tiny keys\n"
      "  --small, --no-small       - include/exclude 31 bytes, i.e small keys\n"
      "  --medium, --no-medium     - include/exclude 1K, i.e medium keys\n"
      "  --large, --no-large       - include/exclude 16K, i.e large keys\n"
      "  --huge, --no-huge         - include/exclude 256K, i.e huge keys\n"
      "  --all-sizes               - run benchmark for all sizes of keys\n"
      "  --all-funcs               - run benchmark for all functions\n"
      "\n"
      "Functions choices:\n"
      "  --0, --no-0               - include/exclude t1ha0\n"
      "  --1, --no-1               - include/exclude t1ha1\n"
      "  --2, --no-2               - include/exclude t1ha2\n"
      "  --32, --no-32             - include/exclude 32-bit targets,\n"
      "                              i.e t1ha0_32le(), t1ha0_32be()...\n"
      "  --64, --no-64             - include/exclude 64-bit targets,\n"
      "                              i.e. t1ha1_64le(), t1ha1_64be()...\n"
      "  --le, --no-le             - include/exclude little-endian targets,\n"
      "                              i.e. t1ha0_32le(), t1ha2...\n"
      "  --be, --no-le             - include/exclude big-endian targets,\n"
      "                              i.e. t1ha0_32be(), t1ha1_64be()...\n"
#ifdef T1HA0_AESNI_AVAILABLE
      "  --aes, --no-aes           - include/exclude AES-NI accelerated,\n"
      "                              i.e. t1ha0_ia32aes_avx(), etc...\n"
#endif /* T1HA0_AESNI_AVAILABLE */
      );
}

static bool option(const char *arg, const char *opt, unsigned flag) {
  if (strncmp(arg, "--", 2) == 0 && strcmp(arg + 2, opt) == 0) {
    option_flags |= flag;
    return true;
  }
  if (strncmp(arg, "--no-", 5) == 0 && strcmp(arg + 5, opt) == 0) {
    disabled_option_flags |= flag;
    return true;
  }
  return false;
}

int main(int argc, const char *argv[]) {
  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      if (strcmp("--test-only", argv[i]) == 0 ||
          strcmp("--no-bench", argv[i]) == 0) {
        option_flags &= test_verbose;
        continue;
      }
      if (strcmp("--test-verbose", argv[i]) == 0) {
        option_flags |= test_verbose;
        continue;
      }
      if (strcmp("--bench-verbose", argv[i]) == 0) {
        option_flags |= bench_verbose;
        continue;
      }
      if (strcmp("--verbose", argv[i]) == 0 || strcmp("-v", argv[i]) == 0) {
        option_flags |= bench_verbose | test_verbose;
        continue;
      }
      if (strcmp("--bench-all", argv[i]) == 0) {
        option_flags |= ~(test_verbose | bench_verbose);
        disabled_option_flags = 0;
        continue;
      }
      if (strcmp("--all-funcs", argv[i]) == 0) {
        option_flags |= bench_funcs_flags;
        disabled_option_flags &= ~bench_funcs_flags;
        continue;
      }
      if (strcmp("--all-sizes", argv[i]) == 0) {
        option_flags |= bench_size_flags;
        disabled_option_flags &= ~bench_size_flags;
        continue;
      }
      if (strcmp("--aes", argv[i]) == 0) {
        if (available_eas_flags) {
          option_flags |= available_eas_flags;
          continue;
        }
        fprintf(stderr, "%s: AES-NI not available for '%s', bailout\n", argv[0],
                argv[i]);
        return EXIT_FAILURE;
      }
      if (strcmp("--no-aes", argv[i]) == 0) {
        if (available_eas_flags) {
          disabled_option_flags |= available_eas_flags;
        } else {
          fprintf(stderr, "%s: AES-NI not available for '%s', ignore\n",
                  argv[0], argv[i]);
        }
        continue;
      }
      if (option(argv[i], "0", bench_0))
        continue;
      if (option(argv[i], "1", bench_1))
        continue;
      if (option(argv[i], "2", bench_2))
        continue;
      if (option(argv[i], "le", bench_le))
        continue;
      if (option(argv[i], "be", bench_be))
        continue;
      if (option(argv[i], "32", bench_32))
        continue;
      if (option(argv[i], "64", bench_64))
        continue;
      if (option(argv[i], "tiny", bench_tiny))
        continue;
      if (option(argv[i], "small", bench_small))
        continue;
      if (option(argv[i], "medium", bench_medium))
        continue;
      if (option(argv[i], "large", bench_large))
        continue;
      if (option(argv[i], "huge", bench_huge))
        continue;

      if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
        usage();
        return EXIT_SUCCESS;
      } else {
        fprintf(stderr, "%s: unknown option '%s'\n\n", argv[0], argv[i]);
        usage();
        return EXIT_FAILURE;
      }
    }
    if ((option_flags & bench_funcs_flags) == 0)
      option_flags |= default_option_flags & bench_funcs_flags;
    if ((option_flags & bench_size_flags) == 0)
      option_flags |= default_option_flags & bench_size_flags;
  } else {
    option_flags = default_option_flags;
    disabled_option_flags = default_disabled_option_flags;
  }

  bool failed = false;
  /* Nowadays t1ha2 not frozen */
  failed |= verify("t1ha2_atonce", t1ha2_atonce, refval_2atonce, true);
  failed |=
      verify("t1ha2_atonce128", thunk_t1ha2_atonce128, refval_2atonce128, true);
  failed |= verify("t1ha2_stream", thunk_t1ha2_stream, refval_2stream, true);
  failed |=
      verify("t1ha2_stream128", thunk_t1ha2_stream128, refval_2stream128, true);

  /* Stable t1ha1 and t1ha0 */
  failed |= verify("t1ha1_64le", t1ha1_le, refval_64le, false);
  failed |= verify("t1ha1_64be", t1ha1_be, refval_64be, false);
  failed |= verify("t1ha0_32le", t1ha0_32le, refval_32le, false);
  failed |= verify("t1ha0_32be", t1ha0_32be, refval_32be, false);

#ifdef __e2k__
  failed |= verify("t1ha0_ia32aes_noavx", t1ha0_ia32aes_noavx, refval_ia32aes_a,
                   false);
  failed |=
      verify("t1ha0_ia32aes_avx", t1ha0_ia32aes_avx, refval_ia32aes_a, false);
#elif defined(T1HA0_AESNI_AVAILABLE)
  ia32_fetch_cpu_features();
  if (ia32_cpu_features.basic.ecx & UINT32_C(0x02000000)) {
    failed |= verify("t1ha0_ia32aes_noavx", t1ha0_ia32aes_noavx,
                     refval_ia32aes_a, false);
    if ((ia32_cpu_features.basic.ecx & UINT32_C(0x1A000000)) ==
        UINT32_C(0x1A000000)) {
      failed |= verify("t1ha0_ia32aes_avx", t1ha0_ia32aes_avx, refval_ia32aes_a,
                       false);
      if (ia32_cpu_features.extended_7.ebx & 32)
        failed |= verify("t1ha0_ia32aes_avx2", t1ha0_ia32aes_avx2,
                         refval_ia32aes_b, false);
    }
  } else {
    if (option_flags & user_wanna_aes)
      printf(" - AES-NI not available on the current CPU\n");
    option_flags &= ~bench_aes;
  }
  if ((ia32_cpu_features.basic.ecx & UINT32_C(0x1A000000)) !=
      UINT32_C(0x1A000000))
    option_flags &= ~bench_avx;
  if ((ia32_cpu_features.extended_7.ebx & 32) == 0)
    option_flags &= ~bench_avx2;
#endif /* T1HA0_AESNI_AVAILABLE */

  if (failed)
    return EXIT_FAILURE;

  printf("\nPreparing to benchmarking...\n");
  fflush(NULL);
  if (!mera_init()) {
    printf(" - sorry, usable clock-source unavailable\n");
    return EXIT_SUCCESS;
  }

  if (mera.cpunum >= 0)
    printf(" - running on CPU#%d\n", mera.cpunum);
  printf(" - use %s as clock source for benchmarking\n", mera.source);
  printf(" - assume it %s and %s\n",
         (mera.flags & timestamp_clock_cheap) ? "cheap" : "costly",
         (mera.flags & timestamp_clock_stable)
             ? "stable"
             : "floating (RESULTS MAY VARY AND BE USELESS)");

  printf(" - measure granularity and overhead: ");
  fflush(NULL);
  double mats /* MeasurAble TimeSlice */ = bench_mats();
  printf("%g %s, %g iteration/%s\n", mats, mera.units, 1 / mats, mera.units);

  if (is_option_set(bench_verbose)) {
    printf(" - convergence: ");
    if (mera_bci.retry_count)
      printf("retries %u, ", mera_bci.retry_count);
    printf("restarts %u, accounted-loops %u, worthless-loops %u, spent <%us\n",
           mera_bci.restart_count, mera_bci.overhead_accounted_loops,
           mera_bci.overhead_worthless_loops, mera_bci.spent_seconds);
    printf(" - mats/overhead: best %" PRIu64 ", gate %" PRIu64
           ", inner-loops-max %u, best-count %u\n",
           mera_bci.overhead_best, mera_bci.overhead_gate,
           mera_bci.overhead_loops_max, mera_bci.overhead_best_count);
  }
  fflush(NULL);

#if !defined(__OPTIMIZE__) && (defined(_MSC_VER) && defined(_DEBUG))
  bench_size(1, "Non-optimized/Debug");
  printf("\nNon-optimized/Debug build, skip benchmark\n");
#else
  if (is_option_set(bench_tiny))
    bench_size(5, "tiny");
  if (is_option_set(bench_small))
    bench_size(31, "small");
  if (is_option_set(bench_medium))
    bench_size(1024, "medium");
  if (is_option_set(bench_large))
    bench_size(1024 * 16, "large");
  if (is_option_set(bench_huge))
    bench_size(1024 * 256, "huge");
#endif /* __OPTIMIZE__ */

  return EXIT_SUCCESS;
}
