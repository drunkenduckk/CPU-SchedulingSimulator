#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;

class Process {
private:
    int processID;
    int arrivalTime;
    int burstTime;
    int priority;
    int waitingTime;
    int turnaroundTime;
    int completionTime;
    int remainingTime;
    
public:
    // Constructor
    Process(int pid = 0, int at = 0, int bt = 0, int pr = 0) 
        : processID(pid), arrivalTime(at), burstTime(bt), priority(pr),
          waitingTime(0), turnaroundTime(0), completionTime(0), remainingTime(bt) {}
    
    // Getters
    int getPID() const { return processID; }
    int getArrivalTime() const { return arrivalTime; }
    int getBurstTime() const { return burstTime; }
    int getPriority() const { return priority; }
    int getWaitingTime() const { return waitingTime; }
    int getTurnaroundTime() const { return turnaroundTime; }
    int getCompletionTime() const { return completionTime; }
    int getRemainingTime() const { return remainingTime; }
    
    // Setters
    void setPID(int pid) { processID = pid; }
    void setArrivalTime(int at) { arrivalTime = at; }
    void setBurstTime(int bt) { burstTime = bt; remainingTime = bt; }
    void setPriority(int pr) { priority = pr; }
    void setWaitingTime(int wt) { waitingTime = wt; }
    void setTurnaroundTime(int tat) { turnaroundTime = tat; }
    void setCompletionTime(int ct) { completionTime = ct; }
    void setRemainingTime(int rt) { remainingTime = rt; }
    
    // Calculate derived times
    void calculateTimes() {
        turnaroundTime = completionTime - arrivalTime;
        waitingTime = turnaroundTime - burstTime;
    }
    
    // Display process info
    void display() const {
        cout << setw(5) << processID << setw(5) << arrivalTime << setw(5) << burstTime 
             << setw(5) << priority << setw(5) << completionTime << setw(5) << waitingTime 
             << setw(5) << turnaroundTime << endl;
    }
};

class CPUScheduler {
private:
    vector<Process> processes;
    vector<pair<int, int>> ganttChart; // (processID, duration)
    
public:
    void addProcess(const Process& p) {
        processes.push_back(p);
    }
    
    void clearProcesses() {
        processes.clear();
        ganttChart.clear();
    }
    
    void inputProcessDetails() {
        int n;
        cout << "\nHow Much Process You Wanna Add?  ";
        cin >> n;
        
        clearProcesses();
        
        for (int i = 0; i < n; i++) {
            int pid, at, bt, pr;
            cout << "\nProcess " << (i + 1) << ":\n";
            cout << "Process ID: ";
            cin >> pid;
            cout << "Arrival Time: ";
            cin >> at;
            cout << "Burst Time: ";
            cin >> bt;
            cout << "Priority? (1=highest, 10=lowest): ";
            cin >> pr;
            
            addProcess(Process(pid, at, bt, pr));
        }
        cout << "\nProcess Added :)\n";
    }
    
    void fcfsScheduling() {
        if (processes.empty()) {
            cout << "\nThere No Process Right Now , First Add Some Process Then Continue..\n";
            return;
        }
        
        vector<Process> temp = processes;
        ganttChart.clear();
        
        // Sort by arrival time
        sort(temp.begin(), temp.end(), [](const Process& a, const Process& b) {
            return a.getArrivalTime() < b.getArrivalTime();
        });
        
        int currentTime = 0;
        
        for (auto& p : temp) {
            if (currentTime < p.getArrivalTime()) {
                currentTime = p.getArrivalTime();
            }
            
            p.setCompletionTime(currentTime + p.getBurstTime());
            p.calculateTimes();
            ganttChart.push_back({p.getPID(), p.getBurstTime()});
            currentTime = p.getCompletionTime();
        }
        
        cout << "\n=== FIRST COME FIRST SERVE (FCFS) SCHEDULING ===\n";
        displayResults(temp);
    }
    
    void sjfScheduling() {
        if (processes.empty()) {
            cout << "\nThere No Process Right Now , First Add Some Process Then Continue..\n";
            return;
        }
        
        vector<Process> temp = processes;
        vector<Process> completed;
        ganttChart.clear();
        
        int currentTime = 0;
        
        while (!temp.empty()) {
            vector<Process> available;
            
            // Find processes that have arrived
            for (const auto& p : temp) {
                if (p.getArrivalTime() <= currentTime) {
                    available.push_back(p);
                }
            }
            
            if (available.empty()) {
                // No process has arrived, advance time to next arrival
                int nextArrival = temp[0].getArrivalTime();
                for (const auto& p : temp) {
                    if (p.getArrivalTime() < nextArrival) {
                        nextArrival = p.getArrivalTime();
                    }
                }
                currentTime = nextArrival;
                continue;
            }
            
            // Select process with shortest burst time
            auto shortest = min_element(available.begin(), available.end(),
                [](const Process& a, const Process& b) {
                    return a.getBurstTime() < b.getBurstTime();
                });
            
            Process selected = *shortest;
            selected.setCompletionTime(currentTime + selected.getBurstTime());
            selected.calculateTimes();
            completed.push_back(selected);
            ganttChart.push_back({selected.getPID(), selected.getBurstTime()});
            currentTime = selected.getCompletionTime();
            
            // Remove selected process from temp
            temp.erase(remove_if(temp.begin(), temp.end(),
                [&selected](const Process& p) {
                    return p.getPID() == selected.getPID();
                }), temp.end());
        }
        
        cout << "\n=== SHORTEST JOB FIRST (SJF) SCHEDULING ===\n";
        displayResults(completed);
    }
    
    void roundRobinScheduling() {
        if (processes.empty()) {
            cout << "\nThere No Process Right Now , First Add Some Process Then Continue..\n";
            return;
        }
        
        int timeQuantum;
        cout << "\nEnter time quantum: ";
        cin >> timeQuantum;
        
        vector<Process> temp = processes;
        queue<Process*> readyQueue;
        ganttChart.clear();
        
        // Sort by arrival time
        sort(temp.begin(), temp.end(), [](const Process& a, const Process& b) {
            return a.getArrivalTime() < b.getArrivalTime();
        });
        
        int currentTime = 0;
        int processIndex = 0;
        
        while (processIndex < temp.size() || !readyQueue.empty()) {
            // Add arrived processes to ready queue
            while (processIndex < temp.size() && temp[processIndex].getArrivalTime() <= currentTime) {
                readyQueue.push(&temp[processIndex]);
                processIndex++;
            }
            
            if (readyQueue.empty()) {
                currentTime = temp[processIndex].getArrivalTime();
                continue;
            }
            
            Process* current = readyQueue.front();
            readyQueue.pop();
            
            int executeTime = min(timeQuantum, current->getRemainingTime());
            current->setRemainingTime(current->getRemainingTime() - executeTime);
            ganttChart.push_back({current->getPID(), executeTime});
            currentTime += executeTime;
            
            // Add newly arrived processes
            while (processIndex < temp.size() && temp[processIndex].getArrivalTime() <= currentTime) {
                readyQueue.push(&temp[processIndex]);
                processIndex++;
            }
            
            if (current->getRemainingTime() > 0) {
                readyQueue.push(current);
            } else {
                current->setCompletionTime(currentTime);
                current->calculateTimes();
            }
        }
        
        cout << "\n=== ROUND ROBIN SCHEDULING (Time Quantum = " << timeQuantum << ") ===\n";
        displayResults(temp);
    }
    
    void priorityScheduling() {
        if (processes.empty()) {
            cout << "\nThere No Process Right Now , First Add Some Process Then Continue..\n";
            return;
        }
        
        vector<Process> temp = processes;
        vector<Process> completed;
        ganttChart.clear();
        
        int currentTime = 0;
        
        while (!temp.empty()) {
            vector<Process> available;
            
            // Find processes that have arrived
            for (const auto& p : temp) {
                if (p.getArrivalTime() <= currentTime) {
                    available.push_back(p);
                }
            }
            
            if (available.empty()) {
                // No process has arrived, advance time to next arrival
                int nextArrival = temp[0].getArrivalTime();
                for (const auto& p : temp) {
                    if (p.getArrivalTime() < nextArrival) {
                        nextArrival = p.getArrivalTime();
                    }
                }
                currentTime = nextArrival;
                continue;
            }
            
            // Select process with highest priority (lowest number)
            auto highest = min_element(available.begin(), available.end(),
                [](const Process& a, const Process& b) {
                    return a.getPriority() < b.getPriority();
                });
            
            Process selected = *highest;
            selected.setCompletionTime(currentTime + selected.getBurstTime());
            selected.calculateTimes();
            completed.push_back(selected);
            ganttChart.push_back({selected.getPID(), selected.getBurstTime()});
            currentTime = selected.getCompletionTime();
            
            // Remove selected process from temp
            temp.erase(remove_if(temp.begin(), temp.end(),
                [&selected](const Process& p) {
                    return p.getPID() == selected.getPID();
                }), temp.end());
        }
        
        cout << "\n=== PRIORITY SCHEDULING ===\n";
        displayResults(completed);
    }
    
    void displayResults(const vector<Process>& results) {
        cout << "\nProcess Table:\n";
        cout << string(50, '=') << endl;
        cout << setw(5) << "PID" << setw(5) << "AT" << setw(5) << "BT" 
             << setw(5) << "PR" << setw(5) << "CT" << setw(5) << "WT" << setw(5) << "TAT" << endl;
        cout << string(50, '-') << endl;
        
        double totalWT = 0, totalTAT = 0;
        for (const auto& p : results) {
            p.display();
            totalWT += p.getWaitingTime();
            totalTAT += p.getTurnaroundTime();
        }
        
        cout << string(50, '=') << endl;
        cout << "\nAverage Waiting Time: " << fixed << setprecision(2) 
             << totalWT / results.size() << " units" << endl;
        cout << "Average Turnaround Time: " << fixed << setprecision(2) 
             << totalTAT / results.size() << " units" << endl;
        
        displayGanttChart();
    }
    
    void displayGanttChart() {
        cout << "\nGantt Chart:\n";
        cout << string(60, '=') << endl;
        
        // Top border
        cout << "|";
        for (const auto& entry : ganttChart) {
            for (int i = 0; i < entry.second; i++) {
                cout << "--";
            }
            cout << "|";
        }
        cout << endl;
        
        // Process IDs
        cout << "|";
        for (const auto& entry : ganttChart) {
            int spaces = entry.second*2-to_string(entry.first).length();
            cout << string(spaces/2, ' ') << "P" << entry.first 
                 << string(spaces - spaces/2, ' ') << "|";
        }
        cout << endl;
        
        // Bottom border
        cout << "|";
        for (const auto& entry : ganttChart) {
            for (int i = 0; i < entry.second; i++) {
                cout << "--";
            }
            cout << "|";
        }
        cout << endl;
        
        // Time markers
        int time = 0;
        cout << time;
        for (const auto& entry : ganttChart) {
            time += entry.second;
            int spaces = entry.second * 2 - to_string(time).length();
            cout << string(spaces, ' ') << time;
        }
        cout << endl;
    }
    
    void saveToFile() {
        if (processes.empty()) {
            cout << "\nNo processes to save, add something first..\n";
            return;
        }
        
        string filename;
        cout << "\nGive this process a name: ";
        cin >> filename;
        
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "'t open the file faced some error :0\n";
            return;
        }
        
        file << processes.size() << endl;
        for (const auto& p : processes) {
            file << p.getPID() << " " << p.getArrivalTime() << " " 
                 << p.getBurstTime() << " " << p.getPriority() << endl;
        }
        
        file.close();
        cout << "Processes saved to " << filename << " successfully!\n";
    }
    
    void loadFromFile() {
        string filename;
        cout << "\nName of the file you wanna open: ";
        cin >> filename;
        
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Brother i cant find any file known as "<<filename<<endl;
            return;
        }
        
        clearProcesses();
        
        int n;
        file >> n;
        
        for (int i = 0; i < n; i++) {
            int pid, at, bt, pr;
            file >> pid >> at >> bt >> pr;
            addProcess(Process(pid, at, bt, pr));
        }
        
        file.close();
        cout << "Processes loaded from " << filename << " successfully!\n";
    }
    
    void displayMenu() {
        cout << "\n" << string(60, '=') << endl;
        cout << "          CPU SCHEDULING SIMULATOR BY DRUNKENDUCK" << endl;
        cout << string(60, '=') << endl;
        cout << "1. Input Process Details" << endl;
        cout << "2. Run FCFS (First Come First Serve)" << endl;
        cout << "3. Run SJF (Shortest Job First)" << endl;
        cout << "4. Run Round Robin" << endl;
        cout << "5. Run Priority Scheduling" << endl;
        cout << "6. Save Processes to File" << endl;
        cout << "7. Load Processes from File" << endl;
        cout << "8. Display Current Processes" << endl;
        cout << "9. Exit" << endl;
        cout << string(60, '=') << endl;
        cout << "So what you wanna do first? : ";
    }
    
    void displayCurrentProcesses() {
        if (processes.empty()) {
            cout << "\nThere's Nothing To SHOW!!!\n";
            return;
        }
        
        cout << "\nCurrent Processes:\n";
        cout << string(40, '=') << endl;
        cout << setw(5) << "PID" << setw(5) << "AT" << setw(5) << "BT" 
             << setw(5) << "PR" << endl;
        cout << string(40, '-') << endl;
        
        for (const auto& p : processes) {
            cout << setw(5) << p.getPID() << setw(5) << p.getArrivalTime() 
                 << setw(5) << p.getBurstTime() << setw(5) << p.getPriority() << endl;
        }
        cout << string(40, '=') << endl;
    }
    
    void run() {
        int choice;
        do {
            displayMenu();
            cin >> choice;
            
            switch (choice) {
                case 1:
                    inputProcessDetails();
                    break;
                case 2:
                    fcfsScheduling();
                    break;
                case 3:
                    sjfScheduling();
                    break;
                case 4:
                    roundRobinScheduling();
                    break;
                case 5:
                    priorityScheduling();
                    break;
                case 6:
                    saveToFile();
                    break;
                case 7:
                    loadFromFile();
                    break;
                case 8:
                    displayCurrentProcesses();
                    break;
                case 9:
                    cout << "\nThank you from DrunkenDuck for using CPU Scheduling Simulator!\n";
                    cout << "Peace Out :)\n";
                    break;
                default:
                    cout << "\nWTF Man ? Can't You Just Select From 1 to 9?.......\n";
            }
            
            if (choice != 9) {
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
            }
            
        } while (choice != 9);
    }
};

int main() {
    CPUScheduler scheduler;
    scheduler.run();
    return 0;
}