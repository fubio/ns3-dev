import matplotlib.pyplot as plt
import numpy as np
import os
file_paths = [
    "allgather_results/Allgather.n8-DGX1-steps7.chunks6.msccl_results.csv",
    "allgather_results/Allgather.n8-DGX1-steps2.rounds2.chuncks1.msccl_results.csv",
    "allgather_results/Allgather.n8-DGX1-steps3.rounds3.chunks2.msccl_results.csv",
    "allgather_results/Allgather.n8-DGX1-steps4.rounds4.chunks3.msccl_results.csv",
    "allgather_results/Allgather.n8-DGX1-steps5.rounds5.chunks4.msccl_results.csv",
    "allgather_results/Allgather.n8-DGX1-steps6.rounds6.chunks5.msccl_results.csv"
] 

def plot_multiple_files(file_paths):
    plt.figure(figsize=(14, 8))  # Increase figure size

    # Iterate through each file
    for file_path in file_paths:
        # Read the data from the file
        with open(file_path, 'r') as file:
            lines = file.readlines()
        
        # Parse the first line as the data sizes (x-values)
        data_sizes = list(map(float, lines[0].strip().split(',')))
        
        # Parse the second line as the corresponding runtimes (y-values)
        runtimes = list(map(float, lines[1].strip().split(',')))
        
        # Plot the current file's data with markers
        plt.plot(
            data_sizes,
            runtimes,
            label=os.path.basename(file_path),  # Use file name as the label
            linewidth=2,                      # Thicker lines
            marker='o',                       # Add markers
            markersize=7                      # Increase marker size
        )

    # Set the axes to log scale
    plt.xscale('log')
    plt.yscale('log')

    # Set the x and y axis labels with larger font size
    plt.xlabel("Input Size (KBytes)", fontsize=16, fontweight='bold')
    plt.ylabel("Average Latency (ms)", fontsize=16, fontweight='bold')

    # Set custom ticks for the y-axis (log scale)
    plt.yticks([0.01, 0.1, 1, 10, 100], ["0.01", "0.1", "1", "10", "100"], fontsize=14)

    # Set custom ticks for the x-axis (log scale)
    powers_of_two = [0.75, 12, 192, 3072, 49152, 786432]
    plt.xticks(powers_of_two, [str(int(x)) for x in powers_of_two], fontsize=14)

    # Add a grid with both major and minor ticks
    plt.grid(True, which="both", linestyle="--", linewidth=0.7, alpha=0.6)

    # Move legend below the plot
    plt.legend(
        loc="upper center",
        bbox_to_anchor=(0.5, -0.15),
        fontsize=12,
        ncol=3,  # Arrange in multiple columns
        frameon=False
    )

    # Add a bold title
    plt.title("Log-Log Plot of Data Size vs Runtime", fontsize=18, fontweight='bold')

    # Adjust layout to make room for the legend
    plt.tight_layout()
    
    # Save plot as a file (optional)
    plt.savefig("improved_plot.png", bbox_inches="tight")

    # Show the plot
    plt.show()

# Plot the data from all files
plot_multiple_files(file_paths)
