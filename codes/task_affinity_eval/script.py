import csv
import os
import platform
import pathlib

task_search_items = []
thread_search_items = []

class Object:

    def __init__(self, length):
        self.data = []
        for i in range(length):
            self.data.append(0)

    def __add__(self, other):
        for i in range(len(self.data)):
            self.data[i] += other.data[i]
        return self

    def __eq__(self, other):
        for i in range(len(self.data)):
            if not self.data[i] == other.data[i]:
                return False
        return True
    
    def __gt__(self, other):
        for i in range(len(self.data)):
            if self.data[i] <= other.data[i]:
                return False
        return True

    def __lt__(self, other):
        for i in range(len(self.data)):
            if self.data[i] >= other.data[i]:
                return False
        return True

    def __ge__(self, other):
        for i in range(len(self.data)):
            if self.data[i] < other.data[i]:
                return False
        return True

    def __le__(self, other):
        for i in range(len(self.data)):
            if self.data[i] > other.data[i]:
                return False
        return True

    def set_data(self, data, search_criteria):
        for i in range(len(search_criteria)):
            if search_criteria[i] in data[2]:
                tmp = data[2].partition("TASK_EXECUTION_TIME of task ")[2][0:7]
                self.data[i] = tmp
                #print(search_criteria[i] + "\t" + data[3])
                return True
        return False

    def merge(self, other):
        tmp = []
        for i in range(len(self.data)):
            if not other.data[i] == 0:
                if not self.data[i] == 0:
                    return False
                tmp.append(other.data[i])
            else:
                tmp.append(self.data[i])
        self.data = tmp
        return True

class Thread(Object):
    thread_id = ""

    map_data = {}
    # key: search string
    # value: array
    def __init__(self):
        super().__init__(len(thread_search_items))

    def set_thread_data(self, data):
        if super().set_data(data, thread_search_items):
            #print("Thread:")
            self.thread_id = data[1]
            return True
        return False

class Task(Object):
    task_id = ""
    domain = -2
    
    def __init__(self):
        for i in range(len(task_search_items)):
            self.data.append(0)
        #super().__init__(len(task_search_items))

    def set_task_data(self, data):
        if self.set_data(data, task_search_items):
            #print("Task:")
            tmp_task_id = data[2].partition("TASK_EXECUTION_TIME of task ")[2][0:7]
            self.task_id = tmp_task_id
            if "data domain = " in data[2]:
                self.domain = data[2].partition("data domain = ")[2][0]
            return 1
        return 0

class StratInfo:
    task_data = []          #stores data from tasks witch are executed on correct domain
    thread_data = []        #stores data from threads
    strat_exec_time = []    #stores runtime from each run of one strategy

    strat_name = ""

    def create_strat_info(self, file_path, file_name):
        file = open(file_path, "r")
        line = file.readline()
        self.strat_name = file_name.split("_")[1]

        #task = Task()
        #self.task_data.append(task)
        #thread = Thread()
        #self.thread_data.append(thread)

        while line:
            data = line.split("\t")

            if len(data) > 2:

                tmp_task = Task()
                tmp_thread = Thread()

                if len(self.task_data) > 1:
                    print(str(self.task_data[0].data[0]) + "   ts  " + str(self.task_data[0].task_id))

                if tmp_task.set_task_data(data):
                    if len(self.task_data) > 1:
                        print(str(self.task_data[0].data[0]) + "   aa  " + str(self.task_data[0].task_id))
                    if len(self.task_data) == 0:
                        #self.task_data[0].task_id = tmp_task.task_id
                        #self.task_data[0].merge(tmp_task)
                        self.task_data.append(tmp_task)
                    elif self.task_data[-1].task_id == tmp_task.task_id:
                        self.task_data[-1].merge(tmp_task)
                    else:
                        #print(self.task_data[-1].data[0])
                        self.task_data.append(tmp_task)
                        #for i in range(len(self.task_data)):
                            #print(str(i) + "\t" + str(self.task_data[i].data[0]))
                    

                elif tmp_thread.set_thread_data(data):
                    if len(self.thread_data) == 0:
                        self.thread_data.append(tmp_thread)
                    elif len(self.thread_data) == 1 or self.thread_data[-1].thread_id == tmp_thread.thread_id:
                        self.thread_data[-1].merge(tmp_thread)
                    else:
                        self.thread_data.append(tmp_thread)
                elif "Elapsed time for program" in data[0]:
                    self.strat_exec_time.append(data[1])

                #print(self.task_data[0].task_id)
                #print(self.task_data[-1].task_id)
            line = file.readline()
            del data
        
        #print(self.task_data[100].data[0])

    def get_task_data_array(self, data_id):
        tmp = []
        
        print(len(self.task_data))

        for i in range(len(self.task_data)): 
            if data_id > len(self.task_data[i].data):
                return []
            #print(self.task_data[i].data[data_id])
            tmp.append(self.task_data[i].task_id)

        return tmp

    def get_thread_data_array(self, id):
        tmp = []


        for i in range(len(self.thread_data)): 
            if id > len(self.thread_data[i].data):
                return []
            tmp.append(self.thread_data[i].data[id])

        return tmp

class TestInfo:
    test_name = ""
    strats = []

    def __init__(self, test_name):
        self.test_name = test_name
    
    def create_test_info(self, folder_path):
        for file_name in os.listdir(folder_path):
            tmp_strat = StratInfo()
            file_path = os.path.join(folder_path, file_name)
            tmp_strat.create_strat_info(file_path, file_name)
            self.strats.append(tmp_strat)

#F_SIZE = 16

#plt.rc('font', size=F_SIZE)             # controls default text sizes
#plt.rc('axes', titlesize=F_SIZE)        # fontsize of the axes title
#plt.rc('axes', labelsize=F_SIZE)        # fontsize of the x and y labels
#plt.rc('xtick', labelsize=F_SIZE)       # fontsize of the tick labels
#plt.rc('ytick', labelsize=F_SIZE)       # fontsize of the tick labels
#plt.rc('legend', fontsize=F_SIZE)       # legend fontsize
#plt.rc('figure', titlesize=F_SIZE)      # fontsize of the figure title


def read_setup_file(source_folder):
    setup_file = open(os.path.join(source_folder, "setup.txt"), "r")
    line = setup_file.readline()

    while line:
        data = line.split("\t")
        data[1] = data[1].replace("\n","")

        if "Thread" in data[0]:
            thread_search_items.append(data[1])
            #print(data[1])
        elif "Task" in data[0]: 
            task_search_items.append(data[1])
            #print(data[1])

        line = setup_file.readline()
    

if __name__ == "__main__":
    tests = []

    #source_folder = os.path.dirname(os.path.abspath(__file__))
    source_folder = "C:\\Users\\rober\\Desktop\\repos\\hpc\\ba-raimbault-task-affinity\\codes\\task_affinity_eval"
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
        tmp_test = TestInfo(folder_name)
        folder_path = os.path.join(source_benchmark_folder, folder_name)
        tmp_test.create_test_info(folder_path)
        tests.append(tmp_test)


    
    for test in tests:
        print(test.test_name)
        for strat in test.strats:
            #print(strat.get_task_data_array(0))
            strat.get_task_data_array(0)
