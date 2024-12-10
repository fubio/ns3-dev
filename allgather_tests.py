import subprocess
import os
import csv

# Define the parameters to vary
chunks_list = [6, 1, 2, 3, 4, 5]  # Example chunk sizes
data_sizes = []  # Example data sizes
start = 0.75
end = 6144
powers_of_two = []
current = start
while current <= end:
    data_sizes.append(current)
    current *= 2

file_paths = [
    "../msccl-tools/allgather/Allgather.n8-DGX1-steps7.chunks6.msccl.json",
    "../msccl-tools/allgather/Allgather.n8-DGX1-steps2.rounds2.chuncks1.msccl.json",
    "../msccl-tools/allgather/Allgather.n8-DGX1-steps3.rounds3.chunks2.msccl.json",
    "../msccl-tools/allgather/Allgather.n8-DGX1-steps4.rounds4.chunks3.msccl.json",
    "../msccl-tools/allgather/Allgather.n8-DGX1-steps5.rounds5.chunks4.msccl.json",
    "../msccl-tools/allgather/Allgather.n8-DGX1-steps6.rounds6.chunks5.msccl.json"
]  # Example file paths

# Iterate over each file and save its data in a separate CSV
file_num = -1
for file_path in file_paths:
    file_num += 1
    file_results = []
    
    # Construct a filename for each file's CSV
    file_name = os.path.basename(file_path).replace(".json", "_results.csv")
    
    # Open the CSV for writing
    with open(file_name, mode="w", newline="") as file:
        writer = csv.writer(file)
        
        # Write header: the data sizes (converted to a string for easy use as headers)
        writer.writerow([str(int(ds)) for ds in data_sizes])
        

        runtimes = []  # Store the runtime for each data size
        
        for data_size in data_sizes:
            # Construct the command to run ns3 simulation
            command = [
                "./ns3",
                "run",
                f"scratch/sccl-sim-basic.cc --chuncks={chunks_list[file_num]} --dataSize={int(data_size*1000)} --file={file_path}"
            ]
            
            # Execute the command
            print(f"Running command: {' '.join(command)}")
            result = subprocess.run(command, capture_output=True, text=True)
            
            if result.returncode == 0:
                # Capture the last line of the output
                output_lines = result.stdout.strip().split("\n")
                last_line = output_lines[-1] if output_lines else "No output"
                
                # Add to results (convert output to milliseconds)
                try:
                    runtime = float(last_line) * 1000  # Convert to milliseconds
                    runtimes.append(runtime)
                except ValueError:
                    print(f"Failed to convert last line to float: {last_line}")
            else:
                print(f"Error for chunks={chunks_list[file_num]}, dataSize={data_size}, file={file_path}:")
                print(f"stderr: {result.stderr}")
                
        # Write the data for the current chunk as a row in the CSV
        writer.writerow(runtimes)
print(f"Results saved to {file_name}")
