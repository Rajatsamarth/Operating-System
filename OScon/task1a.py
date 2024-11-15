import matplotlib.pyplot as plt

# Sample data: Replace these with actual measured times from your C program output
methods = ['Gemm_IP', 'Gemm_OP', 'Gemm_ML', 'Gemm_Tiling']
execution_times = [1.23, 0.98, 1.45, 0.75]  # Replace with actual measured times

# Plotting the bar chart
plt.figure(figsize=(10, 6))
plt.bar(methods, execution_times, color=['skyblue', 'orange', 'green', 'red'])

# Adding titles and labels
plt.title('Execution Times of GEMM Variants')
plt.xlabel('GEMM Variant')
plt.ylabel('Execution Time (seconds)')
plt.ylim(0, max(execution_times) * 1.2)  # Adjust y-axis to add some space above bars

# Display the values on top of each bar
for i, time in enumerate(execution_times):
    plt.text(i, time + 0.05, f"{time:.2f}", ha='center', va='bottom')

# Show the plot
plt.show()
