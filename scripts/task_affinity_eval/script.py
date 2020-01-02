import os
import matplotlib.pyplot as plt

F_SIZE = 16

plt.rc('font', size=F_SIZE)             # controls default text sizes
plt.rc('axes', titlesize=F_SIZE)        # fontsize of the axes title
plt.rc('axes', labelsize=F_SIZE)        # fontsize of the x and y labels
plt.rc('xtick', labelsize=F_SIZE)       # fontsize of the tick labels
plt.rc('ytick', labelsize=F_SIZE)       # fontsize of the tick labels
plt.rc('legend', fontsize=F_SIZE)       # legend fontsize
plt.rc('figure', titlesize=F_SIZE)      # fontsize of the figure title

task_search_items = []
thread_search_items = []

class Item:
    def __init__(self, index, value, absolute_time, mean_time):
        self.index = index
        self.value = value
        self.absolute_time = absolute_time
        self.mean_time = mean_time

class Object:
    def __init__(self, ID, item_list):
        self.data = {}
        self.ID = ""

        for key in item_list:
            self.data[key] = []

    def add_data(self, ID, key_line, value, mean, ms):
        for key in self.data.keys():
            if key in key_line:
                self.data[key].append(Item(ID, value, mean, ms))
                #print(str(ID) + "\t" + str(value))
                return True

        return False

class Thread(Object):
    def __init__(self, thread_id):
        super().__init__(thread_id, thread_search_items)

class Task(Object):
    def __init__(self, task_id):
        super().__init__(task_id, task_search_items)
    
    def add_data(self, ID, key_line, value):
        return super().add_data(ID, key_line, value, 0, 0)

class Run:

    def __init__(self):
        self.thread_info = {}
        self.task_info = {}
        self.elapsed_time = 0

    def extract_data(self, file):
        line = file.readline()
        while line:
            data = line.split("\t")

            if "Elapsed time for program" in line:
                self.elapsed_time = data[1]
                print(self.elapsed_time)

            if len(data) > 1:
                thread_id = data[1]
                task_id = data[2].partition(" of task ")[2][0:7]
                
                if task_id in self.task_info.keys():
                    new_task = self.task_info[task_id]
                else:
                    new_task = Task(task_id)   
                
                if thread_id in self.thread_info.keys():
                    new_thread = self.thread_info[thread_id]
                else:
                    new_thread = Thread(thread_id)

                if len(data) > 3:
                    value = data[3]

                    if not task_id == "":
                        if new_task.add_data(task_id, data[2], value):
                            self.task_info[task_id] = new_task

                    if len(data) > 6:
                        mean = data[4]
                        ms = data[6]

                        if new_thread.add_data(thread_id, data[2], value, mean, ms):
                            self.thread_info[thread_id] = new_thread

            line = file.readline()

class Strategy:

    def __init__(self, name):
        self.name = name
        self.runs = []

    def add_run(self, file):
        new_run = Run()
        new_run.extract_data(file)
        self.runs.append(new_run)

class Test:

    def __init__(self, name):
        self.name = name
        self.strategies = {}

    def read_data(self, folder_path):
        for file_name in os.listdir(folder_path): #filename structure = strategy_runnumber_extrainformation.txt 
            file = open(os.path.join(folder_path, file_name), "r")

            strat_name = file_name.split("_")[1]

            if strat_name not in self.strategies.keys():
                self.strategies[strat_name] = Strategy(strat_name)

            self.strategies[strat_name].add_run(file)


def read_setup_file(folder):
    setup_file = open(os.path.join(source_folder, "setup.txt"), "r")
    line = setup_file.readline()

    while line:
        data = line.split("\t")
        data[1] = data[1].replace("\n","")

        if not "#" in data[0][0]:
            if "Thread" in data[0]:
                thread_search_items.append(data[1])
                #print(data[1])
            elif "Task" in data[0]: 
                task_search_items.append(data[1])
                #print(data[1])

        line = setup_file.readline()

if __name__ == "__main__":
    Tests = {}

    source_folder = os.getcwd()

    source_benchmark_folder  = os.path.join(source_folder, "result_benchmarks")
    target_folder_data  = os.path.join(source_folder, "result_data")
    target_folder_plot  = os.path.join(source_folder, "result_plots")

    if not os.path.exists(source_benchmark_folder):
        os.makedirs(source_benchmark_folder)
    if not os.path.exists(target_folder_data):
        os.makedirs(target_folder_data)
    if not os.path.exists(target_folder_plot):
        os.makedirs(target_folder_plot)

    read_setup_file(source_folder)

    for folder_name in os.listdir(source_benchmark_folder):
        Tests[folder_name] = Test(folder_name)
        Tests[folder_name].read_data(os.path.join(source_benchmark_folder, folder_name))
        

    for test_key in Tests.keys():
        test = Tests[test_key]
        for strat_key in test.strategies.keys():
            strat = test.strategies[strat_key]
            for run in strat.runs:
                run_info = test.name + "\t" + strat.name + "\t"
                for thread_key in run.thread_info.keys():
                    thread = run.thread_info[thread_key].data
                    for thread_search_item in thread_search_items:
                        items = thread[thread_search_item]
                        for item in items:
                            item = item
                            #print(run_info + item.index + "\t" + thread_search_item + "\t" + str(item.value))
                for task_key in run.task_info.keys():
                    task = run.task_info[task_key].data
                    for task_search_item in task_search_items:
                        items = task[task_search_item]
                        for item in items:
                            item = item
                            #print(test.name + "\t" + strat.name + "\t" + item.index + "\t" + task_search_item + "\t" + str(item.value))
                #for task in run.task_info.keys():
                    #print(run.task_info[task].ID + "\t" + str(run.task_info[task].data["TASK_EXECUTION_TIME"][0].value))

