# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <math.h>
# include <float.h>
# include <limits.h>
# include <sys/time.h>
# include "no_huge_page_alloc.h"
#include "task_affinity_support.h"

#ifdef TASK_AFFINITY
#include <omp.h>
#endif

int init_task_affinity()
{
#ifdef TASK_AFFINITY
    kmp_affinity_thread_selection_mode_t thread_selection_strategy = get_env_int_value(kmp_affinity_thread_selection_mode_random, "THREAD_SELECTION_STRATEGY");
    kmp_affinity_map_mode_t affinity_map_mode = get_env_int_value(kmp_affinity_map_type_domain, "AFFINITY_MAP_MODE");
    kpm_affintiy_page_selection_strategy_t page_selection_strategy = get_env_int_value(kmp_affinity_page_mode_divide_in_n_pages, "PAGE_SELECTION_MODE");
    kmp_affinity_page_weighting_strategy_t page_weighting_strategy = get_env_int_value(kmp_affinity_page_weight_mode_majority, "PAGE_WEIGHTING_STRATEGY");
    int number_of_affinities = get_env_int_value(1, "NUMBER_OF_AFFINITIES");

    kmp_affinity_settings_t affinity_settings = 
    {
      .thread_selection_strategy = thread_selection_strategy,
      .affinity_map_mode = affinity_map_mode,
      .page_selection_strategy = page_selection_strategy,
      .page_weighting_strategy = page_weighting_strategy,
      .number_of_affinities = number_of_affinities
    };
    
    kmpc_task_affinity_init(affinity_settings);

    return 1;
#else
    return 0;
#endif
}

int get_env_int_value(int default_value, char *env)
{
    env = getenv(env);

    if (env == NULL)
    {
        return default_value;
    }

    int value = atoi(env);

    if (value < 0) 
    {
        printf("%s was set to default\n", env);
        return default_value;
    }

    return value;
}