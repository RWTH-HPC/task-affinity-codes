import argparse
import os, sys
import numpy as np
import statistics as st
import matplotlib.pyplot as plt
import csv

from CMeasurement import *
# TODO: load that dynamically depending on parameter (execute/eval)
from CMetricsSTREAM import *

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Parses and summarizes execution time measurement results and statistics")
    parser.add_argument("-s", "--source",       required=True,  type=str, metavar="<folder path>", help=f"source folder containing outputs")
    parser.add_argument("-d", "--destination",  required=False, type=str, metavar="<folder path>", default=None, help=f"destination folder where resulting data and plots will be stored")
    args = parser.parse_args()

    if not os.path.exists(args.source):
        print(f"Source folder path \"{args.source}\" does not exist")
        sys.exit(1)

    # ========== DEBUG ==========
    # args.source = "C:\\J.Klinkenberg.Local\\repos\\hpc-research\\openmp\\task-affinity\\task-affinity-codes\\codes\\stream\\2023-05-12_093613_results"
    # ========== DEBUG ==========

     # save results in source folder
    if args.destination is None:
        args.destination = os.path.join(args.source, "result_evaluation")

    if not os.path.exists(args.destination):
        os.makedirs(args.destination)

    list_files: list[CMeasurement] = []
    for file in os.listdir(args.source):
        if file.endswith("_output.txt"):
            cur_file_path = os.path.join(args.source, file)
            print(cur_file_path)

            # read file meta data and measurements
            meas = CMeasurement(cur_file_path, file)
            metr_loader = CMetricsSTREAM()
            metr_loader.load_metrics(cur_file_path, meas)
            # append list of measurements
            list_files.append(meas)

    # ========================================
    # === Group: For each number of threads
    # ========================================
    unique_n_threads = sorted(list(set([x.n_threads for x in list_files])))

    for nthr in unique_n_threads:
        print(f"Generating views for {nthr} Threads ...")
        # filter by number threads
        sub1 = [x for x in list_files if x.n_threads == nthr]
        # get all versions
        unique_versions = sorted(list(set([x.version for x in sub1])))

        target_file_thr = os.path.join(args.destination, f"data_{nthr}_threads.csv")
        with open(target_file_thr, mode="w", newline='') as f_thr:
            writer_thr = csv.writer(f_thr, delimiter=';')

            writer_thr.writerow([f"===== Exection time ====="])
            writer_thr.writerow(['Versions', 'Execution Time [s]'])            
            for cur_ver in unique_versions:
                sub2 = [x for x in sub1 if x.version == cur_ver]
                exec_time = st.mean([x.time_work for x in sub2])
                writer_thr.writerow([cur_ver, exec_time])
            writer_thr.writerow([])

            for stat_id in CMeasurement.STAT_IDENTIFIERS:
                writer_thr.writerow([f"===== {stat_id} ====="])
                writer_thr.writerow(['Versions', 'Nr Events', 'Execution Time [ms]'])
                for cur_ver in unique_versions:
                    sub2            = [x for x in sub1 if x.version == cur_ver]
                    cur_data        = [x.get_stats_sum_for_identifier(stat_id) for x in sub2]
                    mean_events     = st.mean([x[0] for x in cur_data])
                    mean_time_ms    = st.mean([x[1] for x in cur_data])
                    writer_thr.writerow([cur_ver, mean_events, mean_time_ms])
                writer_thr.writerow([])