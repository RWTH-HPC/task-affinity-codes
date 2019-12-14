class Tasks:
    ex_time = []
    ex_time_total = 0
    
    num_tasks = 0

    def sorted_add_to_list(value):
        ex_time_total = ex_time_total + value
        num_tasks = num_tasks + 1

        if num_tasks == 1:
            ex_time.append(value)
            return
        
        if value <= ex_time[0]: 
            ex_time.insert(0, value)
            return

        if value >= ex_time[-1]:
             ex_time.append(value)
             return

        for i in range(num_tasks)
            if value <= ex_time[i]:
                ex_time.insert(i, value)
                return
        
    def get_min()
        return ex_time[0]

    def get_max()
        return ex_time[-1]

    def get_median()
        return ex_time[round( num_tasks / 2 )]
    
    def get_mean()
        return ex_time_total / num_tasks

class Thread:
    tasks_generated = 0
    tasks_started_with_affinity = 0
    tasks_started_at_correct_thread = 0
    tasks_started_at_correct_domain = 0

    def __add__(self, a):
        new = Thread()
        new.tasks_generated = self.tasks_generated + a.tasks_generated
        new.tasks_started_with_affinity = self.tasks_started_with_affinity + a.tasks_started_with_affinity
        new.tasks_started_at_correct_thread = self.tasks_started_at_correct_thread + a.tasks_started_at_correct_thread
        new.tasks_started_at_correct_domain = self.tasks_started_at_correct_domain + a.tasks_started_at_correct_domain
        return new


class StratInfo:
    tasks_on_corr_domain = Tasks()
    tasks_on_in_corr_domain = Tasks()

    threads = []
    total_thread_data = Thread()
    num_threads = 0

    def add_thread(thread):
        num_threads = num_threads + 1
        threads.append(thread)
        total_thread_data = total_thread_data + thread


