#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifndef TASK_AFFINITY_SUPPORT
#define TASK_AFFINITY_SUPPORT

    

    const char *kmp_affinity_thread_selection_mode_c[] = {
        "thread_selection_mode_first",
        "thread_selection_mode_random",
        "thread_selection_mode_lowest_wl",
        "thread_selection_mode_round_robin",
        "thread_selection_mode_private",
    };

    const char *kmp_affinity_map_mode_c[] = {
        "map_type_thread",
        "map_type_domain",
    };

    const char *kmp_affinity_page_selection_strategy_c[] = {
        "page_mode_first_page_of_first_affinity_only",
        "page_mode_divide_in_n_pages",
        "page_mode_every_nth_page",
        "page_mode_first_and_last_page",
        "page_mode_continuous_binary_search",
        "page_mode_first_page",
    };

    const char *kmp_affinity_page_weighting_strategy_c[] = {
        "page_weight_mode_first_page_only",
        "page_weight_mode_majority",
        "page_weight_mode_by_affinity",
        "page_weight_mode_by_size"
    };


    int size_of_char_array(char *array[]) 
    {
        return (int)(sizeof(array) / sizeof(array[0]));
    }

    int init_task_affinity(int argc, char** argv)
    {
    #ifdef _OPENMP
        if (argc <= 1) {
            printf("_task_affinity_support: no Arguments passed, continue with default setting");
            return 1;
        }

        if (argc - 1 > 5) //checks if there are to many arguments
        {
            printf("_task_affinity_support: to many arguments, set all to default setting");
            return -1;
        }

        
        kmp_affinity_thread_selection_mode_t thread_selection_strategy = kmp_affinity_thread_selection_mode_random;
        kmp_affinity_map_mode_t affinity_map_mode = kmp_affinity_map_type_domain;
        kmp_affinity_page_selection_strategy_t page_selection_strategy = kmp_affinity_page_mode_divide_in_n_pages;
        kmp_affinity_page_weighting_strategy_t page_weighting_strategy = kmp_affinity_page_weight_mode_majority;
        int number_of_affinities = 1;

        int i, j;
        int check[] = {0, 0, 0, 0, 0}; //contains data wether or not a setting is set prior

        for (i = 1; i < argc; i++) 
        {
            //check if argument contains thread_selection_strategy
            for (j = 0; j < size_of_char_array(kmp_affinity_thread_selection_mode_c); j++) 
            {
                if (kmp_affinity_thread_selection_mode_c[j] == argv[i]) 
                {
                    if (check[0] >= 0)
                    {
                        printf("_task_affinity_support: could not set %d argument, '%s', was already declared as: '%s'",i , atoi(argv[i]), kmp_affinity_thread_selection_mode_c[thread_selection_strategy]);
                        continue;
                    }
                    thread_selection_strategy = j;
                    check[0]++;
                    continue;
                }
            }
            //check if argument contains map_mode
            for (j = 0; j < size_of_char_array(kmp_affinity_map_mode_c); j++) 
            {
                if (kmp_affinity_map_mode_c[j] == argv[i]) 
                {
                    if (check[1] >= 0)
                    {
                        printf("_task_affinity_support: could not set %d argument, '%s', was already declared as: '%s'", i , atoi(argv[i]), kmp_affinity_map_mode_c[affinity_map_mode]);
                        continue;
                    }
                    affinity_map_mode = j;
                    check[1]++;
                    continue;
                }
            }
            //check if argument contains page_selection_strategy
            for (j = 0; j < size_of_char_array(kmp_affinity_page_selection_strategy_c); j++) 
            {
                if (kmp_affinity_page_selection_strategy_c[j] == argv[i]) 
                {
                    if (check[2] >= 0)
                    {
                        printf("_task_affinity_support: could not set %d argument, '%s', was already declared as: '%s'",i ,  atoi(argv[i]), kmp_affinity_page_selection_strategy_c[page_selection_strategy]);
                        continue;
                    }
                    page_selection_strategy = j;
                    check[2]++;
                    continue;
                }
            }
            //check if argument contains page_weighting_strategy
            for (j = 0; j < size_of_char_array(kmp_affinity_page_weighting_strategy_c); j++) 
            {
                if (kmp_affinity_page_weighting_strategy_c[j] == argv[i]) 
                {
                    if (check[3] >= 0)
                    {
                        printf("_task_affinity_support: could not set %d argument, '%s', was already declared as: '%s'",i ,  atoi(argv[i]), kmp_affinity_thread_selection_mode_c[page_weighting_strategy]);
                        continue;
                    }
                    page_weighting_strategy = j;
                    check[3]++;
                    continue;
                }
            }
            int tmp = atoi(argv[i]);

            if (tmp != 0) 
            {
                if (check[4] >= 0)
                {
                    printf("_task_affinity_support: could not set %d argument, number_of_affinities was already declared as: '%d'",i ,  atoi(argv[i]), number_of_affinities);
                    continue;
                }
                check[4]++;
                number_of_affinities = tmp;
                continue;
            }

            printf("_task_affinity_support: '%s' is no valid affinity setting, this input will be ignored",  atoi(argv[i]));
        }

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
#endif