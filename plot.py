import os
import pandas as pd
import matplotlib.pyplot as plt

# Folder where the CSV files are located (change if needed)
folder = "."

# Find all latency CSV files in the current folder
csv_files = [f for f in os.listdir(folder) if f.startswith("latencies_") and f.endswith(".csv")]

# Sort for consistent plot order
csv_files.sort()

# Plot each CSV
for file in csv_files:
    filepath = os.path.join(folder, file)
    df = pd.read_csv(filepath, names=["task_id", "latency_ms"])

    # Create a more readable title from filename
    title = file.replace("latencies_", "").replace("__", " ").replace("_", " ").replace(".csv", "")

    # Plot setup
    plt.figure(figsize=(10, 5), dpi=120)
    plt.plot(df["task_id"], df["latency_ms"], marker='o')
    plt.title(f"Latency per Task - {title}")
    plt.xlabel("Task ID")
    plt.ylabel("Latency (ms)")
    plt.grid(True)

    plt.autoscale(enable=True, axis='both', tight=True)
    plt.tight_layout()

    image_file = file.replace(".csv", ".png")
#plt.savefig(image_file)

#print(f"Saved: {image_file}")
    plt.show()

