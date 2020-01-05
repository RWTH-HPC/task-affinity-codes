import os
import csv
import numpy as np
import matplotlib as mpl 
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
        self.value = float(str(value).replace(',', '.', 1))
        self.absolute_time = float(str(absolute_time).replace(',', '.', 1))
        self.mean_time = float(str(mean_time).replace(',', '.', 1))

    def __add__(self, other):
        value = self.value + other.value
        absolute_time = self.absolute_time + other.absolute_time
        mean_time = self.mean_time + other.mean_time
        return Item('', value, absolute_time, mean_time)

    def __div__(self, devidor):
        value = self.value/devidor
        absolute_time = self.absolute_time/devidor
        mean_time = self.mean_time/devidor
        return Item('', value, absolute_time, mean_time)

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
        self.task_incorr_info = {}
        self.elapsed_time = 0

    def extract_data(self, file):
        line = file.readline()
        while line:
            data = line.split("\t")

            if "Elapsed time for program" in line:
                self.elapsed_time = data[1]
                #print(self.elapsed_time)

            if len(data) > 1:
                thread_id = data[1]
                task_id = data[2].partition(" of task ")[2][0:7]
                
                
                if task_id in self.task_info.keys():
                    new_task = self.task_info[task_id]
                else:
                    new_task = Task(task_id)   

                if task_id in self.task_incorr_info.keys():
                    new_task_incorr = self.task_incorr_info[task_id]
                else:
                    new_task_incorr = Task(task_id)   

                if thread_id in self.thread_info.keys():
                    new_thread = self.thread_info[thread_id]
                else:
                    new_thread = Thread(thread_id)

                if len(data) > 3:
                    value = data[3]

                    if not task_id == "":
                        if 'in_corr_domain' in line:
                            if new_task_incorr.add_data(task_id, data[2], value):
                                self.task_incorr_info[task_id] = new_task_incorr
                        else:
                            if new_task.add_data(task_id, data[2], value):
                                self.task_info[task_id] = new_task

                    if len(data) > 6:
                        mean = data[4]
                        ms = data[6]

                        if new_thread.add_data(thread_id, data[2], value, mean, ms):
                            self.thread_info[thread_id] = new_thread

            line = file.readline()

    def get_total_thread_stats_by_key(self, key):
        result = Item('', 0, 0, 0)

        if key in thread_search_items:
            for thread_id in self.thread_info.keys():
                #print(thread_id + '\t' + key + '\t' + str(self.thread_info[thread_id].data[key][0].value))
                #print(self.thread_info[thread_id].data[key].index)
                result = result + self.thread_info[thread_id].data[key][0]

        return result

    def get_total_task_stats_by_key(self, key, corr_domain):
        result = Item('', 0, 0, 0)

        if key in task_search_items:
            if corr_domain:
                for task_id in self.task_info.keys():
                    result = result + self.task_info[task_id].data[key][0]
            else:
                for task_id in self.task_incorr_info.keys():
                    result = result + self.task_incorr_info[task_id].data[key][0]
                    #result = result + self.task_incorr_info[task_id].data[key][0]

        return result

class Strategy:

    def __init__(self, name):
        self.name = name
        self.runs = []

    def add_run(self, file):
        new_run = Run()
        new_run.extract_data(file)
        self.runs.append(new_run)


    def get_timings(self):
        data = []
        for run in self.runs:
            elapsed = float(run.elapsed_time.replace(',', '.', 1))
            #print(elapsed)
            data.append(elapsed)

        return data

    def get_mean_thread_stats(self):
        output = {}
        for thread_items in thread_search_items:
            item = Item('',0,0,0)
            count = 0
            for run in self.runs:
                item = item + run.get_total_thread_stats_by_key(thread_items)
                count = count + 1
            if count > 0:
                item = item.__div__(count)
            output[thread_items] = item
        return output
            
    def get_mean_task_stats_corr_domain(self):
        output = {}
        for task_items in task_search_items:
            item = Item('',0,0,0)
            count = 0
            for run in self.runs:
                #tmp = run.get_total_stats_by_key(task_items).__div__(len(run.task_info.keys()))
                item = item + run.get_total_task_stats_by_key(task_items, True).__div__(len(run.task_info.keys()))
                count = count + 1
            if count > 0:
                item = item.__div__(count)
            output[task_items] = item
        return output

    def get_mean_task_stats_incorr_domain(self):
        output = {}
        for task_items in task_search_items:
            item = Item('',0,0,0)
            count = 0
            for run in self.runs:
                #tmp = run.get_total_stats_by_key(task_items).__div__(len(run.task_info.keys()))
                item = item + run.get_total_task_stats_by_key(task_items, False).__div__(len(run.task_incorr_info.keys()))
                count = count + 1
            if count > 0:
                item = item.__div__(count)
            output[task_items] = item
        return output

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


    def print_timings(self, plot_folder):
        data = []
        label = []

        for strat_key in self.strategies.keys():
            strat = self.strategies[strat_key]
            data.append(strat.get_timings())
            label.append(strat.name) 
        
        title = self.name + '_elapsed_time_plot'

        mpl.use('agg')
        fig = plt.figure(1, figsize=(10,10))
        ax = fig.add_subplot(111)
        ax.yaxis.grid(True, linestyle='-',which='major')
        ax.set_title(title)
        ax.set_ylabel('seconds')
        ax.boxplot(data, labels=label)
        fig.savefig(os.path.join(plot_folder,title + '.png'), bbox_inches='tight')

    def print_thread_stats(self, plot_folder):
        if len(thread_search_items) < 1:
            return False

        fig1 = plt.figure(2, figsize=(10,10))
        ax1 = fig1.add_subplot()
        ax1.set_title('thread stats')
        index = np.arange(len(thread_search_items))
        bar_width = 0.2
        opacity = 0.8
        count = 0

        title = self.name + '_thread_stats'

        for strat_key in self.strategies.keys():
            strat = self.strategies[strat_key]
            stats = strat.get_mean_thread_stats()
            data = []
            for item in thread_search_items:
                #print (strat.name + "\t" + item + "\t" + str(stats[item].value))
                data.append(stats[item].value)

            # stats = strat.get_mean_task_stats()
            # for item in task_search_items:
            #     data.append(stats[item].value)

            #print(strat.name + "\t" + data)
            ax1.bar(index + bar_width*count*1.2, data, bar_width, label=strat_key)
            count = count + 1

        ax1.yaxis.grid(True, linestyle='-',which='major')
        plt.xticks(index + bar_width, thread_search_items, rotation=90)
        plt.legend()
        fig1.savefig(os.path.join(plot_folder,title + '.png'), bbox_inches='tight')

    def print_task_stats(self, plot_folder):
        if len(task_search_items) < 1:
            return False

        fig2 = plt.figure(3, figsize=(10,10))
        ax2 = fig2.add_subplot()
        ax2.set_title('task execution time')
        ax2.set_ylabel('seconds')
        index = np.arange(len(task_search_items)*2)
        bar_width = 0.2
        opacity = 0.8
        count = 0

        title = self.name + '_task_stats'

        for strat_key in self.strategies.keys():
            strat = self.strategies[strat_key]
            stats_corr = strat.get_mean_task_stats_corr_domain()
            stats_incorr = strat.get_mean_task_stats_incorr_domain()

            data = []
            for item in task_search_items:
                #print (strat.name + "\t" + item + "\t" + str(stats[item].value))
                data.append(stats_corr[item].value)

            for item in task_search_items:
                data.append(stats_incorr[item].value)
            # stats = strat.get_mean_task_stats()
            # for item in task_search_items:
            #     data.append(stats[item].value)

            #print(strat.name + "\t" + data)
            ax2.bar(index + bar_width*count*1.2, data, bar_width, label=strat_key)
            count = count + 1

        ax2.yaxis.grid(True, linestyle='-',which='major')
        plt.xticks(index + bar_width, ('corr domain', 'in corr domain'), rotation=90)
        plt.legend()
        fig2.savefig(os.path.join(plot_folder,title + '.png'), bbox_inches='tight')

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
        

def create_csv(folder):
    with open(tmp_target_file_path, mode='w', newline='') as f:
        writer = csv.writer(f, delimiter=',')

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
        
    #for test_key in Tests.keys():
    #    create_csv(target_folder_data, Tests[test_key])
        

    for test_key in Tests.keys():
        test = Tests[test_key]
        test.print_timings(target_folder_plot)
        test.print_thread_stats(target_folder_plot)
        test.print_task_stats(target_folder_plot)
        # for strat_key in test.strategies.keys():
        #     strat = test.strategies[strat_key]
        #     print(strat.name)
        #     #strat.print_timings()
        #     for run in strat.runs:
        #         run_info = test.name + "\t" + strat.name + "\t"
        #         for thread_key in run.thread_info.keys():
        #             thread = run.thread_info[thread_key].data
        #             for thread_search_item in thread_search_items:
        #                 items = thread[thread_search_item]
        #                 for item in items:
        #                     item = item
        #                     print(run_info + item.index + "\t" + thread_search_item + "\t" + str(item.value))
    #             for task_key in run.task_info.keys():
    #                 task = run.task_info[task_key].data
    #                 for task_search_item in task_search_items:
    #                     items = task[task_search_item]
    #                     for item in items:
    #                         item = item
    #                         #print(test.name + "\t" + strat.name + "\t" + item.index + "\t" + task_search_item + "\t" + str(item.value))
    #             #for task in run.task_info.keys():
    #                 #print(run.task_info[task].ID + "\t" + str(run.task_info[task].data["TASK_EXECUTION_TIME"][0].value))

