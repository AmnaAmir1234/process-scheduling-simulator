# CPU Scheduling Simulator

A comprehensive GTK-based graphical application for simulating and visualizing various CPU scheduling algorithms used in operating systems. This educational tool provides an interactive environment to understand how different scheduling algorithms work and compare their performance.

## Features

### Supported Scheduling Algorithms
- **First-Come, First-Served (FCFS)** - Non-preemptive FIFO scheduling
- **Shortest Job First (SJF)** - Non-preemptive shortest burst time first
- **Shortest Remaining Time First (SRTF)** - Preemptive version of SJF
- **Priority Scheduling** - Non-preemptive priority-based scheduling
- **Round Robin (RR)** - Preemptive time-slice based scheduling
- **Preemptive Priority Scheduling** - Preemptive version of priority scheduling

### Interactive Features
- **Process Management**: Add, delete, and manage up to 50 processes
- **Visual Gantt Chart**: Real-time visualization of process execution timeline
- **Performance Matrix**: Graphical comparison of turnaround time, waiting time, and response time
- **Statistical Analysis**: Detailed statistics including averages for all timing metrics
- **Algorithm Information**: Built-in documentation for each scheduling algorithm
- **Algorithm Comparison**: Side-by-side comparison of all scheduling algorithms
- **Sample Data**: Pre-loaded sample processes for quick testing

### User Interface
- **Tabbed Interface**: Organized tabs for processes, Gantt chart, performance matrix, statistics, algorithm info, and comparison
- **Color-coded Visualization**: Each process is assigned a unique color for easy identification
- **Interactive Dialogs**: User-friendly forms for process input and algorithm parameters
- **Real-time Updates**: Instant visualization updates after running algorithms

## Screenshots

The application provides multiple views:
- Process list with arrival time, burst time, priority, and calculated metrics
- Gantt chart showing process execution timeline
- Performance matrix with bar charts for timing comparisons
- Detailed statistics and algorithm comparisons

## Installation

### Prerequisites
- GTK+ 3.0 development libraries
- Cairo graphics library
- GCC compiler
- Linux/Unix environment (tested on Ubuntu/Debian)

### Ubuntu/Debian Installation
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev libcairo2-dev gcc make
```

### Fedora/CentOS Installation
```bash
sudo dnf install gtk3-devel cairo-devel gcc make
# or for older versions:
sudo yum install gtk3-devel cairo-devel gcc make
```

### Build and Run
```bash
# Clone the repository
git clone https://github.com/AmnaAmir1234/cpu-scheduling-simulator.git
cd cpu-scheduling-simulator

# Compile the application
gcc -o cpu_scheduler main.c `pkg-config --cflags --libs gtk+-3.0 cairo`

# Run the application
./cpu_scheduler
```

## Usage

### Adding Processes
1. Click "Add Process" button
2. Enter process details:
   - **Process Name**: Identifier for the process (e.g., P1, P2)
   - **Arrival Time**: When the process arrives in the ready queue
   - **Burst Time**: CPU execution time required
   - **Priority**: Priority level (1-10, where 1 is highest priority)

### Running Simulations
1. Add processes or click "Load Sample" for pre-defined processes
2. Select a scheduling algorithm button (FCFS, SJF, SRTF, Priority, Round Robin, Preemptive Priority)
3. For Round Robin, specify the time quantum when prompted
4. View results in different tabs:
   - **Processes**: Updated process table with calculated times
   - **Gantt Chart**: Visual timeline of process execution
   - **Performance Matrix**: Bar chart comparison of metrics
   - **Statistics**: Detailed numerical analysis

### Understanding Results
- **Completion Time (CT)**: When the process finishes execution
- **Turnaround Time (TAT)**: Total time from arrival to completion (CT - AT)
- **Waiting Time (WT)**: Time spent waiting in ready queue (TAT - BT)
- **Response Time (RT)**: Time from arrival to first CPU allocation

## Algorithm Details

### First-Come, First-Served (FCFS)
- Simple queue-based scheduling
- Non-preemptive
- Poor performance with long processes first
- No starvation

### Shortest Job First (SJF)
- Optimal for minimizing average waiting time
- Non-preemptive
- May cause starvation of long processes
- Requires knowledge of burst times

### Shortest Remaining Time First (SRTF)
- Preemptive version of SJF
- Optimal for minimizing average waiting time
- High context switch overhead
- May cause starvation

### Priority Scheduling
- Processes scheduled based on priority
- Non-preemptive version
- May suffer from starvation
- Good for systems with varying process importance

### Round Robin (RR)
- Time-slice based preemptive scheduling
- Fair CPU allocation
- Performance depends on time quantum
- No starvation

### Preemptive Priority Scheduling
- Preemptive version of priority scheduling
- High priority processes get immediate attention
- May cause starvation of low priority processes
- Good for real-time systems

## Code Structure

```
main.c
├── Data Structures
│   ├── Process - stores process information and metrics
│   └── GanttBlock - stores Gantt chart visualization data
├── GUI Components
│   ├── Main window setup and event handlers
│   ├── Tabbed interface management
│   └── Drawing functions for visualizations
├── Scheduling Algorithms
│   ├── FCFS implementation
│   ├── SJF implementation
│   ├── SRTF implementation
│   ├── Priority scheduling implementation
│   ├── Round Robin implementation
│   └── Preemptive Priority implementation
└── Utility Functions
    ├── Process management
    ├── Statistics calculation
    └── Visualization updates
```

## Educational Value

This simulator is designed for:
- **Operating Systems Students**: Understanding CPU scheduling concepts
- **Computer Science Education**: Visual learning of algorithm behavior
- **Algorithm Comparison**: Performance analysis between different approaches
- **Research**: Testing scheduling algorithm efficiency with custom process sets

## Limitations

- Maximum of 50 processes per simulation
- Simplified model (no I/O operations)
- Single CPU simulation only
- Fixed priority range (1-10)
- No dynamic priority adjustment

## Contributing

Contributions are welcome! Areas for improvement:
- Multi-core CPU simulation
- Additional scheduling algorithms (CFS, EDF, etc.)
- Process arrival pattern generation
- Export functionality for results
- Enhanced visualization options
- Performance optimization

## License

This project is open source and available under the MIT License.

## Technical Requirements

- **Language**: C
- **GUI Framework**: GTK+ 3.0
- **Graphics**: Cairo
- **Platform**: Linux/Unix
- **Compiler**: GCC with GTK development packages

## Future Enhancements

- Multi-level queue scheduling
- Aging mechanism for priority scheduling
- Process migration simulation
- Real-time scheduling algorithms
- Web-based version
- Mobile application port

## Acknowledgments

Developed as an educational tool for understanding CPU scheduling algorithms in operating systems courses. The implementation focuses on clarity and educational value while providing accurate algorithm simulations.
