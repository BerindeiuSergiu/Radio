#!/usr/bin/env python3
"""
Parse simulation CSV results and create graphs
"""
import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def read_csv_file(filename):
    """Read CSV file with simulation data"""
    try:
        df = pd.read_csv(filename)
        return df
    except Exception as e:
        print(f"Error reading {filename}: {e}")
        return None

def find_csv_files(results_dir='results'):
    """Find all .csv files in results directory"""
    csv_files = []
    if os.path.exists(results_dir):
        for file in os.listdir(results_dir):
            if file.endswith('.csv'):
                csv_files.append(os.path.join(results_dir, file))
    return csv_files

def plot_scheduler_data(df):
    """Plot scheduler weight and delay data"""
    if df is None or df.empty:
        print("No data to plot")
        return
    
    fig, ax = plt.subplots(figsize=(12, 6))
    
    # Plot weight vector over time
    ax.plot(df['time'], df['weight'], marker='o', linestyle='-', linewidth=1, markersize=3, label='High Priority Weight')
    
    ax.set_xlabel('Simulation Time (s)')
    ax.set_ylabel('Weight Value')
    ax.set_title('Scheduler Weight Vector Over Time')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('results/scheduler_plot.png', dpi=150)
    print("Graph saved as results/scheduler_plot.png")
    plt.show()

def main():
    # Find CSV files
    csv_files = find_csv_files()
    
    if not csv_files:
        print("No CSV files found in results/ directory")
        return
    
    print(f"Found {len(csv_files)} CSV file(s):")
    for f in csv_files:
        print(f"  - {f}")
    
    # Read and plot data
    for csv_file in csv_files:
        print(f"\nProcessing {csv_file}...")
        df = read_csv_file(csv_file)
        
        if df is not None:
            print(f"Data shape: {df.shape}")
            print(f"\nFirst few rows:")
            print(df.head())
            print(f"\nStatistics:")
            print(df.describe())
            
            # Plot if it has the right columns
            if 'time' in df.columns and 'weight' in df.columns:
                plot_scheduler_data(df)

if __name__ == "__main__":
    main()
