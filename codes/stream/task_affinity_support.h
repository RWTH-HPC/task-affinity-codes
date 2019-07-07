#ifndef TASK_AFFINITY_SUPPORT
#define TASK_AFFINITY_SUPPORT
    /* 
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
    };*/


    extern int size_of_char_array(char *array[]);

    extern int init_task_affinity(int argc, char** argv);
#endif