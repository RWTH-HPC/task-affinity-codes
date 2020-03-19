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

average_plot_items = []
absolute_plot_items = []
box_plot_items = []
timings_items = []

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
        if devidor == 0:
            return Item('', 0, 0, 0)
        value = self.value/devidor
        absolute_time = self.absolute_time/devidor
        mean_time = self.mean_time/devidor
        return Item('', value, absolute_time, mean_time)

class Info_Object:
    def __init__(self, index):
        self.data = {}
        self.index = index

    def add_data(self, ID, data_array):
        item = Item(ID, data_array[0], 0, 0)
        
        if len(data_array) == 3:
            item.absolute_time = data_array[1]
            item.absolute_time = data_array[2]

        if ID not in self.data.keys():
            self.data[ID] = [item]
        else:
            self.data[ID].append(item)

    def get_data(self):
        
        data_pack = [[],[],[],[]] #for id, value, absolute_time, means

        for item_id in self.data.keys():
            for item in self.data[item_id]:
                data_pack[0].append(item.index)
                data_pack[1].append(item.value)
                data_pack[2].append(item.absolute_time)
                data_pack[3].append(item.mean_time)
                #print(self.index + "\t" + str(item.index) + "\t" + str(item.value))

        return data_pack
        
class Run:

    def __init__(self):
        self.thread_info = {}
        self.task_info = [{}, {}]   #correct domain, incorrect domain
        self.number_of_threads = 0
        self.number_of_affinities = 0

        for key in thread_search_items:
            self.thread_info[key] = Info_Object(key)
        
        for key in task_search_items:
            self.task_info[0][key] = Info_Object(key)
            self.task_info[1][key] = Info_Object(key)

        self.elapsed_time = 0

    def extract_data(self, file):
        line = file.readline()
        while line:
            data = line.split("\t")

            if "Elapsed time for program" in line:
                self.elapsed_time = data[1]
                #print(self.elapsed_time)
            if "Chosen Number of affinities: " in line:
                self.number_of_affinities = line.split(": ")[1]
                #print("Chosen Number of affinities: " + str(self.number_of_affinities))
            
            if "Number of Threads counted = " in line:
                self.number_of_threads = line.split("= ")[1]


            if len(data) > 3:
                thread_id = data[1]
                task_id = data[2].partition(" of task ")[2][0:7]
                
                for key in thread_search_items:
                    if key == data[2]:
                        self.thread_info[key].add_data(thread_id, [data[3], data[4], data[6]])

                for key in task_search_items:
                    if key in data[2]:
                        if 'in_corr_domain' in line:
                            task_info_list_indicator = 1
                        else:
                            task_info_list_indicator = 0
                        self.task_info[task_info_list_indicator][key].add_data(task_id, [data[3]])

            line = file.readline()

    def get_data_by_key(self, key):
        if key in thread_search_items:
            return self.thread_info[key].get_data()
        
        if key in task_search_items:
            return [self.task_info[0][key].get_data(), self.task_info[1][key].get_data()]

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
            data.append(elapsed)

        return data

    def get_total_thread_stats_by_key(self, key):
        stat_array = [[],[],[],[]]
        for run in self.runs:
            for thread_list in run.get_data_by_key(key):
                for thread in thread_list:
                    stat_array[0].append((thread[0]))
                    for i in range(1, 3):
                        stat_array[i].append(float(thread[i]))

        return stat_array

    def get_mean_thread_stats_by_key(self, key):
        output = []
        counter = 0
        thread_data = [[],[],[],[]]
        for run in self.runs:
            new_thread_data = run.get_data_by_key(key)
            if counter == 0:
                thread_data = run.get_data_by_key(key)
            else:
                counter = counter + 1
                
                divider = 1
                if counter == len(self.runs):
                    divider = len(self.runs)

                for i in range(len(thread_data[0])):
                    for j in range(1, 3):
                        thread_data[j][i] = (float(thread_data[j][i]) + float(new_thread_data[j]))/divider

        return thread_data

    def get_total_task_stats_by_key(self, key):
        stat_array = [[],[]]
        for run in self.runs:
            for task_list in run.get_data_by_key(key)[0][1]:
                stat_array[0].append(float(task_list))
            for task_list in run.get_data_by_key(key)[1][1]:
                stat_array[1].append(float(task_list))
        return stat_array

class Test:

    def __init__(self, name):
        self.name = name
        self.strategies = {}
        self.plot_number = 112

    def sort_strategy(self):
        tmp_strat = [None] * (len(self.strategies))
        

        for strat_key in self.strategies.keys():
            lenght = len(strat_key.split("-"))

            if lenght < 3:
                return

            if strat_key.split("-")[lenght-2] != "combined":
                return


            #print(lenght)
            i = int(strat_key.split("-")[lenght-1][:-2])

            if len(self.strategies) < i:
                return

            tmp_strat[i] = self.strategies[strat_key]
            #print(strat_key.split("-")[lenght-1][:-2])

        tmp_dict = {}

        for strat in tmp_strat:
            tmp_dict[strat.name] = strat

        self.strategies = tmp_dict

        

    def read_data(self, folder_path):
        for file_name in os.listdir(folder_path): #filename structure = strategy_runnumber_extrainformation.txt 
            if len(file_name.split("_")) > 1:
                file = open(os.path.join(folder_path, file_name), "r")

                #print(file_name)
                strat_name = file_name.split("_")[1]

                if strat_name not in self.strategies.keys():
                    self.strategies[strat_name] = Strategy(strat_name)

                self.strategies[strat_name].add_run(file)
        self.sort_strategy()

    def print_timings(self, plot_folder):
        data = []
        label = []

        for strat_key in self.strategies.keys():
            strat = self.strategies[strat_key]
            data.append(strat.get_timings())
            label.append(strat.name) 
        
        title = self.name + '_elapsed_time_plot'

        print (self.get_name())

        self.print_box_plot(title, data, label, "seconds", plot_folder)

    def get_name(self):
        tmp_start_key = list(self.strategies.keys())[0]
        tmp_strat = self.strategies[tmp_start_key]
        tmp_run = tmp_strat.runs[0]
        tmp_thread_num = tmp_run.number_of_threads
        tmp_affiniti_num = tmp_run.number_of_affinities

        #return self.name + ", Affinities: "  + tmp_affiniti_num[:-1] + ", Threads: " + tmp_thread_num[:-1]
        return self.name + ", Affinities: "  + str(tmp_affiniti_num)[:-1] + ", Threads: " + str(tmp_thread_num)

    def print_box_plot(self, title, data_array, label_array, ylabel, plot_folder):
        mpl.use('agg')

        fig = plt.figure(self.plot_number, figsize=(len(label_array),10))
        self.plot_number = self.plot_number + 1

        ax = fig.add_subplot()
        ax.yaxis.grid(True, linestyle='-',which='major')
        plt.xticks(rotation=90)
        ax.set_title(self.get_name())
        ax.set_ylabel(ylabel)
        ax.boxplot(data_array, labels=label_array)

        fig.savefig(os.path.join(plot_folder,title + '.png'), bbox_inches='tight')
        fig.clear()
        
    def print_bar_plot(self, title, data_array, strat_array, labels, ylabel, plot_folder):

        fig = plt.figure(self.plot_number, figsize=(len(strat_array),10))
        self.plot_number = self.plot_number + 1

        index = np.arange(len(labels))

        bar_width = 1/(len(strat_array)*2)
        opacity = 0.8
        count = 0

        ax = fig.add_subplot()
        ax.set_title(self.get_name())
        ax.yaxis.grid(True, linestyle='-',which='major')
        ax.set_ylabel(ylabel)

        for i in range(len(data_array)):
            ax.bar(index + bar_width*count*(1+bar_width), data_array[i], bar_width, label=strat_array[i])
            count = count + 1

        plt.xticks(index + bar_width, labels, rotation=90)
        plt.legend()
        fig.savefig(os.path.join(plot_folder,title + '.png'), bbox_inches='tight')
        fig.clear()

    def print_stats(self, plot_folder):
        total_data = {}
        total_label = []
        total_key = []
        mean_data = {}
        mean_label = []
        mean_key = []
        box_data = []
        box_label = []
        

        for strat_key in self.strategies:
            strat = self.strategies[strat_key]
            total_data[strat_key] = []
            mean_data[strat_key] = []

        for key in thread_search_items:
            index = 1
            total_label = []
            mean_label = []
            for strat_key in self.strategies:
                strat = self.strategies[strat_key]
                tmp_mn = strat.get_mean_thread_stats_by_key(key)

                if key in absolute_plot_items:
                    total_data[strat_key].append(sum(tmp_mn[index]))
                    total_label.append(strat_key)
                    if key not in total_key:
                        total_key.append(key)
                if key in average_plot_items:
                    mean_data[strat_key].append(sum(tmp_mn[index]) /  len(tmp_mn[index]))
                    mean_label.append(strat_key)
                    if key not in mean_key:
                        mean_key.append(key)
                if key in box_plot_items:
                    box_data.append(tmp_mn[index])
                    box_label.append(strat_key)
            if len(box_data) > 0:
                self.print_box_plot(self.get_name() + " Box Plot " + key, box_data, box_label, "count", plot_folder)
                box_data = []
                box_label = []


        
        tmp_total_data = []
        tmp_mean_data = []
        for strat_key in self.strategies:
            tmp = []
            strat = self.strategies[strat_key]
            for item in total_data[strat_key]:
                tmp.append(item)
            if len(tmp) > 0:
                tmp_total_data.append(tmp)
            tmp = []
            for item in mean_data[strat_key]:
                tmp.append(item)
            if len(tmp) > 0:
                tmp_mean_data.append(tmp)

        
        if len(tmp_total_data) > 0:
            self.print_bar_plot(self.name + "_bar_plot_absolute_", tmp_total_data, total_label, total_key, "count", plot_folder)
        if len(tmp_mean_data) > 0:
            self.print_bar_plot(self.name + "_bar_plot_mean_", tmp_mean_data, mean_label, mean_key, "count", plot_folder)



        total_data = {}
        total_label = []
        total_key = []
        mean_data = {}
        mean_label = []
        mean_key = []
        box_data = []
        box_label = []
        

        for strat_key in self.strategies:
            strat = self.strategies[strat_key]
            total_data[strat_key] = []
            mean_data[strat_key] = []

        
        for key in task_search_items:
            index = 1
            total_label = []
            mean_label = []
            for strat_key in self.strategies:
                strat = self.strategies[strat_key]
                tmp_mn = strat.get_total_task_stats_by_key(key)

                if key in absolute_plot_items:
                    total_data[strat_key].append(sum(tmp_mn[0]))
                    total_data[strat_key].append(sum(tmp_mn[1]))
                    total_label.append(strat_key + "_corr")
                    total_label.append(strat_key + "_incorr")
                    total_key.append(key + "_corr")
                    total_key.append(key + "_incorr")
                if key in average_plot_items:
                    mean_data[strat_key].append(sum(tmp_mn[0]) /  len(tmp_mn[0]))
                    mean_data[strat_key].append(sum(tmp_mn[1]) /  len(tmp_mn[0]))
                    mean_label.append(strat_key)
                    mean_key.append(key + "_corr")
                    mean_key.append(key + "_incorr")
                if key in box_plot_items:
                    box_data.append(tmp_mn[0])
                    box_data.append(tmp_mn[1])
                    box_label.append(strat_key + "_corr")
                    box_label.append(strat_key + "_incorr")

            if len(box_data) > 0:
                self.print_box_plot(self.name + "_box_plot_" + key, box_data, box_label, "count", plot_folder)
                box_data = []
                box_label = []


        tmp_total_data = []
        tmp_mean_data = []
        for strat_key in self.strategies:
            tmp = []
            strat = self.strategies[strat_key]
            for item in total_data[strat_key]:
                tmp.append(item)
            if len(tmp) > 0:
                tmp_total_data.append(tmp)
            tmp = []
            for item in mean_data[strat_key]:
                tmp.append(item)
            if len(tmp) > 0:
                tmp_mean_data.append(tmp)

        # if len(tmp_total_data) > 0:
        #     self.print_bar_plot(self.name + "_bar_plot_absolute_", tmp_total_data, total_label, total_key, "count", plot_folder)
        # if len(tmp_mean_data) > 0:
        #     self.print_bar_plot(self.name + "_bar_plot_mean_", tmp_mean_data, mean_label, mean_key, "count", plot_folder)

    def print_detailed_infos(self, data_folder):
        with open(os.path.join(data_folder, self.name + "_data.csv"), mode='w', newline='') as f:
            writer = csv.writer(f, delimiter=',')
            writer.writerow(['Thread Data:'])
            writer.writerow(["Strategy","Thread ID", "counter",  "value", "exec_time", "mean_time"])
            for strat_key in test.strategies.keys():
                strat = test.strategies[strat_key]
                for run in strat.runs:
                    run_info = test.name + "\t" + strat.name + "\t"
                    for thread_key in run.thread_info.keys():
                        thread = run.thread_info[thread_key].data
                        for thread_search_item in thread_search_items:
                            items = thread[thread_search_item]
                            for item in items:
                                writer.writerow([strat.name, thread_key, thread_search_item, item.value, item.absolute_time, item.mean_time])
                                # item = item
                                # print(run_info + item.index + "\t" + thread_search_item + "\t" + str(item.value))
            writer.writerow([])
            writer.writerow(['Task Data:'])
            writer.writerow(["Strategy","Task ID", "counter",  "value", "exec_time", "mean_time"])
            for strat_key in test.strategies.keys():
                strat = test.strategies[strat_key]
                for run in strat.runs:
                    run_info = test.name + "\t" + strat.name + "\t"
                    for task_key in run.task_info.keys():
                        task = run.task_info[task_key].data
                        for task_search_item in task_search_items:
                            items = task[task_search_item]
                            for item in items:
                                writer.writerow([strat.name, task_key, task_search_item + " corr_domain", item.value, item.absolute_time, item.mean_time])
                    for task_key in run.task_incorr_info.keys():
                        task = run.task_incorr_info[task_key].data
                        for task_search_item in task_search_items:
                            items = task[task_search_item]
                            for item in items:
                                writer.writerow([strat.name, task_key, task_search_item + " incorr_domain", item.value, item.absolute_time, item.mean_time])
            writer.writerow([])
                
def read_setup_file(folder):
    setup_file = open(os.path.join(source_folder, "setup.st"), "r")
    line = setup_file.readline()

    while line:
        data = line.split("\t")
        data[1] = data[1].replace("\n","")

        if not "#" in data[0][0]:
            found = False
            if "Thread" in data[0]:
                thread_search_items.append(data[1])
                found = True
                #print(data[1])
            elif "Task" in data[0]: 
                task_search_items.append(data[1])
                found = True
                #print(data[1])

            if found:
                if len(data) == 2:
                    if data[1] not in absolute_plot_items:
                        absolute_plot_items.append(data[1])
                for i in range (2, len(data)):
                    data[i] = data[i].replace('\n', '')
                    print(data[i] + "\t" + data[1])
                    if "box_plot" in data[i]:
                        if data[i] not in box_plot_items:
                            box_plot_items.append(data[1])
                    elif "mean" in data[i]:
                        if data[i] not in average_plot_items:
                            average_plot_items.append(data[1])
                    elif "total":
                        if data[i] not in absolute_plot_items:
                            absolute_plot_items.append(data[1])
                    elif "Timings":
                        if data[i] not in timings_items:
                            timings_items.append(data[1])

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
        test.print_timings(target_folder_plot)
        test.print_stats(target_folder_plot)
        #test.print_detailed_infos(target_folder_data)