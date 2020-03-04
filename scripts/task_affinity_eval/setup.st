#	Write "Task	"keyword to get task specific infos. Please use tab to not confuse the alghorithm.
#	Write "Thread	"keyword to get thread specific infos. Seriously, use tabs!
#	The Keyword has to be included in the line witch contains the Data
#	Every Line witch dont starts with one of the registered Keywords will be considered a comment
#	You can print out a box plot (key: "box_plot"), all total values ("total"), or all the average run stats with ("mean")
#	by adding the keyword "Timings", the plot will also print the corresponding timing
#Task	TASK_EXECUTION_TIME	box_plot	total
Thread	count_overall_tasks_generated	mean
Thread	count_task_with_affinity_started	mean
Thread	count_task_started_at_correct_data_domain	mean
Thread	task_execution_correct_domain	mean
Thread	count_task_started_at_correct_thread	mean
#Thread	overhead_numa_task_stealing	mean