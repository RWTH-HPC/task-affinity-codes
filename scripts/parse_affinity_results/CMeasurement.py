class CStatWIthMean():
    def __init__(self) -> None:
        self.thread_id  = 0
        self.n_events   = 0
        self.time_ms    = 0.0
    
    def get_mean_time(self):
        return self.time_ms / self.n_events

class CMeasurement():
    
    STAT_IDENTIFIERS = [
        "gl_numa_map_create",
        "count_overall_tasks_generated",
        "task_execution",
        "task_execution_correct_domain",
        "time_identify_physical_location",
        "time_strategy1",
        "time_strategy2",

        "count_overall_tasks_generated",
        "count_task_with_affinity_generated",
        "count_task_with_affinity_started",
        "count_task_started_at_correct_thread",
        "count_task_started_at_correct_threads_domain",
        "count_task_started_at_correct_data_domain",

        "count_map_found",
        "count_map_not_found"
    ]

    def get_stats_for_identifier(self, all_lines, identifier: str):
        ret_val = []
        for line in all_lines:
            if f"{identifier}\t" in line:
                spl             = line.split("\t")
                obj             = CStatWIthMean()
                obj.thread_id   = int(spl[1].split("#")[1].strip())
                obj.n_events    = int(spl[3])
                obj.time_ms     = float(spl[4])
                ret_val.append(obj)
        exec(f"self.stats_{identifier} = ret_val")
        return ret_val

    def get_stats_sum_for_identifier(self, identifier: str):
        cur_list = eval(f"self.stats_{identifier}")
        sum_events  = 0
        sum_time_ms = 0.0
        for x in cur_list:
            sum_events  += x.n_events
            sum_time_ms += x.time_ms
        
        return (sum_events, sum_time_ms)

    def __init__(self, file_path, file_name):
        file_name = file_name.split(".")[0]
        tmp_split = file_name.split("_")

        # needs to parsed depending on the program output
        self.time_work = 0.0

        # parse meta information from filename
        if "baseline_" in file_name:
            self.mode                   = "baseline"
            self.page_selection_strat   = ""
            self.n_affinities           = 0
            self.weighting_strat        = ""
            self.version                = "baseline"
            self.n_threads              = int(tmp_split[1])
            self.repetition             = int(tmp_split[2])
        else:
            self.mode                   = tmp_split[1]
            self.page_selection_strat   = tmp_split[2]
            self.n_affinities           = int(tmp_split[3])
            self.weighting_strat        = tmp_split[4]
            self.version                = f"{self.mode}-{self.page_selection_strat}-{self.n_affinities}-{self.weighting_strat}"
            self.n_threads              = int(tmp_split[5])
            self.repetition             = int(tmp_split[6])

        # statistics parse from file content
        with open(file_path) as f: all_lines = [x.strip() for x in list(f)]

        for stat_id in CMeasurement.STAT_IDENTIFIERS:
            self.get_stats_for_identifier(all_lines, stat_id)