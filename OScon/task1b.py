import matplotlib.pyplot as plt

# Sample data: Replace these with actual execution times from your C program for each thread count
# Example: [number of threads, execution time in seconds]
data = [
    (1, 15.23),  # 1 thread
    (2, 8.12),   # 2 threads
    (4, 4.23),   # 4 threads
    (8, 2.56),   # 8 threads
    (16, 1.45),  # 16 threads
]

# Separate the data into two lists: one for thread counts and one for execution times
thread_counts = [item[0] for item in data]
execution_times = [item[1] for item in data]

# Plotting the graph
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, execution_times, marker='o', color='b', linestyle='-', linewidth=2, markersize=8)

# Adding labels and title
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (seconds)")
plt.title("Execution Time vs Number of Threads for Parallel Matrix Multiplication")
plt.grid(True)

# Display the values on each point
for i, time in enumerate(execution_times):
    plt.text(thread_counts[i], time + 0.1, f"{time:.2f}", ha='center')

# Show the plot
plt.show()
