import matplotlib.pyplot as plt
import numpy as np

# Sample data (replace these values with actual measurements)
# This data represents the number of producers, consumers, and the measured execution time.
data = [
    (1, 1, 5.1),
    (2, 1, 4.3),
    (1, 2, 4.5),
    (2, 2, 3.8),
    (3, 1, 3.9),
    (1, 3, 4.0),
    (3, 3, 3.5),
    (4, 2, 3.2),
    (2, 4, 3.3),
    (4, 4, 3.1),
]

# Extracting the number of producers, consumers, and execution time
producers = [item[0] for item in data]
consumers = [item[1] for item in data]
execution_time = [item[2] for item in data]

# Create a 2D scatter plot
plt.figure(figsize=(10, 6))
scatter = plt.scatter(producers, consumers, c=execution_time, cmap='viridis', s=100)
plt.colorbar(scatter, label='Execution Time (seconds)')
plt.xlabel('Number of Producers')
plt.ylabel('Number of Consumers')
plt.title('Execution Time with Varying Producers and Consumers')
plt.grid(True)
plt.show()
