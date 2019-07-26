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
    printf("Selected Strategy was: %s\n", kmp_affinity_thread_selection_mode_c[thread_selection_strategy]);
    kmp_affinity_map_mode_t affinity_map_mode = get_env_int_value(kmp_affinity_map_type_domain, "AFFINITY_MAP_MODE");
    printf("Selected Strategy was: %s\n}", kmp_affinity_map_mode_c[affinity_map_mode]);
    kmp_affinity_page_selection_strategy_t page_selection_strategy = get_env_int_value(kmp_affinity_page_mode_divide_in_n_pages, "PAGE_SELECTION_MODE");
    printf("Selected Strategy was: %s\n", kmp_affinity_page_selection_strategy_c[page_selection_strategy]);
    kmp_affinity_page_weighting_strategy_t page_weighting_strategy = get_env_int_value(kmp_affinity_page_weight_mode_majority, "PAGE_WEIGHTING_STRATEGY");
    printf("Selected Strategy was: %s\n", kmp_affinity_page_weighting_strategy_c[page_weighting_strategy]);
    int number_of_affinities = get_env_int_value(1, "NUMBER_OF_AFFINITIES");
    printf("Chosen Number of affinities: %d\n", number_of_affinities);

    kmp_affinity_settings_t affinity_settings = 
    {
      .thread_selection_strategy = thread_selection_strategy,
      .affinity_map_mode = affinity_map_mode,
      .page_selection_strategy = page_selection_strategy,
      .page_weighting_strategy = page_weighting_strategy,
      .number_of_affinities = number_of_affinities
    };
    
    /* 
    int thread_selection_strategy = get_env_int_value(3, "THREAD_SELECTION_STRATEGY");
    int affinity_map_mode = get_env_int_value(0, "AFFINITY_MAP_MODE");
    int page_selection_strategy = get_env_int_value(1, "PAGE_SELECTION_MODE");
    int page_weighting_strategy = get_env_int_value(1, "PAGE_WEIGHTING_STRATEGY");
    int number_of_affinities = get_env_int_value(1, "NUMBER_OF_AFFINITIES");
    kmpc_task_affinity_init(thread_selection_strategy, affinity_map_mode, page_selection_strategy*100+page_weighting_strategy , number_of_affinities);
    */
   
    kmpc_task_affinity_init(affinity_settings);

    return 1;
#else
    return 0;
#endif
}

int get_env_int_value(int default_value, char *env)
{
    
    printf("%s is set as: ", env);
    env = getenv(env);
    printf("%s \n", env);

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