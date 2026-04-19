/*
 * ============================================================
 *   Adaptive Time-Quantum Round Robin Scheduler
 *   Backend: C++ (scheduler.cpp)
 *
 *   Features:
 *     - Standard Round Robin (fixed quantum)
 *     - Adaptive RR with Average Burst Time quantum
 *     - Adaptive RR with Median Burst Time quantum
 *     - Computes: Waiting Time, Turnaround Time, Response Time
 *     - Outputs results to data/output.json
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include <string>
#include <climits>

using namespace std;

// ─────────────────────────────────────────────
//  Data Structures
// ─────────────────────────────────────────────

struct Process {
    int id;           // Process ID (1-based)
    int arrival;      // Arrival time
    int burst;        // Original burst time
    int remaining;    // Remaining burst time (used during simulation)
    int waiting;      // Waiting time (computed)
    int turnaround;   // Turnaround time (computed)
    int response;     // Response time (computed, -1 = not started yet)
    int completion;   // Completion time
};

struct GanttEntry {
    int processId;   // -1 means CPU idle
    int start;
    int end;
};

struct SchedulerResult {
    string algorithmName;
    int quantum;
    vector<Process> processes;
    vector<GanttEntry> gantt;
    double avgWaiting;
    double avgTurnaround;
    double avgResponse;
};

// ─────────────────────────────────────────────
//  Utility: Compute Quantum from Burst Times
// ─────────────────────────────────────────────

// Method 1: Average Burst Time
int computeAverageQuantum(const vector<Process>& procs) {
    if (procs.empty()) return 1;
    double total = 0;
    for (const auto& p : procs) total += p.burst;
    return max(1, (int)round(total / procs.size()));
}

// Method 2: Median Burst Time
int computeMedianQuantum(const vector<Process>& procs) {
    if (procs.empty()) return 1;
    vector<int> bursts;
    for (const auto& p : procs) bursts.push_back(p.burst);
    sort(bursts.begin(), bursts.end());
    int n = bursts.size();
    if (n % 2 == 0)
        return max(1, (int)round((bursts[n/2 - 1] + bursts[n/2]) / 2.0));
    else
        return bursts[n / 2];
}

// ─────────────────────────────────────────────
//  Core: Round Robin Simulation
// ─────────────────────────────────────────────

SchedulerResult runRoundRobin(
    vector<Process> procs,   // pass by value so we can modify
    int quantum,
    const string& name
) {
    int n = procs.size();

    // Reset runtime fields
    for (auto& p : procs) {
        p.remaining  = p.burst;
        p.waiting    = 0;
        p.turnaround = 0;
        p.response   = -1;   // -1 means not started
        p.completion = 0;
    }

    // Sort processes by arrival time for initial ordering
    sort(procs.begin(), procs.end(), [](const Process& a, const Process& b){
        return a.arrival < b.arrival;
    });

    queue<int> readyQueue;  // stores index into procs[]
    vector<GanttEntry> gantt;

    int currentTime = 0;
    int completed   = 0;
    vector<bool> inQueue(n, false);

    // Enqueue all processes that arrive at time 0
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival == 0) {
            readyQueue.push(i);
            inQueue[i] = true;
        }
    }

    while (completed < n) {
        // If queue is empty, jump time to next arrival
        if (readyQueue.empty()) {
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!inQueue[i] && procs[i].remaining > 0) {
                    nextArrival = min(nextArrival, procs[i].arrival);
                }
            }
            if (nextArrival == INT_MAX) break;

            // Record idle time in Gantt
            gantt.push_back({-1, currentTime, nextArrival});
            currentTime = nextArrival;

            // Enqueue newly arrived processes
            for (int i = 0; i < n; i++) {
                if (!inQueue[i] && procs[i].arrival <= currentTime && procs[i].remaining > 0) {
                    readyQueue.push(i);
                    inQueue[i] = true;
                }
            }
            continue;
        }

        int idx = readyQueue.front();
        readyQueue.pop();

        // Record response time (first time this process gets CPU)
        if (procs[idx].response == -1) {
            procs[idx].response = currentTime - procs[idx].arrival;
        }

        // Execute for min(quantum, remaining)
        int execTime = min(quantum, procs[idx].remaining);
        gantt.push_back({procs[idx].id, currentTime, currentTime + execTime});
        procs[idx].remaining -= execTime;
        currentTime          += execTime;

        // Enqueue any new arrivals that came during this slice
        for (int i = 0; i < n; i++) {
            if (!inQueue[i] && procs[i].arrival <= currentTime && procs[i].remaining > 0) {
                readyQueue.push(i);
                inQueue[i] = true;
            }
        }

        if (procs[idx].remaining == 0) {
            // Process finished
            completed++;
            procs[idx].completion  = currentTime;
            procs[idx].turnaround  = procs[idx].completion - procs[idx].arrival;
            procs[idx].waiting     = procs[idx].turnaround - procs[idx].burst;
        } else {
            // Not done; re-enqueue
            readyQueue.push(idx);
        }
    }

    // Compute averages
    double totalWT = 0, totalTAT = 0, totalRT = 0;
    for (const auto& p : procs) {
        totalWT  += p.waiting;
        totalTAT += p.turnaround;
        totalRT  += (p.response >= 0 ? p.response : 0);
    }

    SchedulerResult result;
    result.algorithmName = name;
    result.quantum       = quantum;
    result.processes     = procs;
    result.gantt         = gantt;
    result.avgWaiting    = (n > 0) ? totalWT  / n : 0;
    result.avgTurnaround = (n > 0) ? totalTAT / n : 0;
    result.avgResponse   = (n > 0) ? totalRT  / n : 0;

    return result;
}

// ─────────────────────────────────────────────
//  JSON Output Writer
// ─────────────────────────────────────────────

string escapeJson(const string& s) {
    string out;
    for (char c : s) {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

void writeResultToJson(ofstream& file, const SchedulerResult& r, bool last) {
    file << "    {\n";
    file << "      \"algorithm\": \"" << escapeJson(r.algorithmName) << "\",\n";
    file << "      \"quantum\": "     << r.quantum << ",\n";
    file << "      \"avgWaiting\": "    << r.avgWaiting    << ",\n";
    file << "      \"avgTurnaround\": " << r.avgTurnaround << ",\n";
    file << "      \"avgResponse\": "   << r.avgResponse   << ",\n";

    // Processes array
    file << "      \"processes\": [\n";
    for (int i = 0; i < (int)r.processes.size(); i++) {
        const auto& p = r.processes[i];
        file << "        {\n";
        file << "          \"id\": "          << p.id         << ",\n";
        file << "          \"arrival\": "     << p.arrival    << ",\n";
        file << "          \"burst\": "       << p.burst      << ",\n";
        file << "          \"waiting\": "     << p.waiting    << ",\n";
        file << "          \"turnaround\": "  << p.turnaround << ",\n";
        file << "          \"response\": "    << p.response   << ",\n";
        file << "          \"completion\": "  << p.completion << "\n";
        file << "        }";
        if (i + 1 < (int)r.processes.size()) file << ",";
        file << "\n";
    }
    file << "      ],\n";

    // Gantt chart array
    file << "      \"gantt\": [\n";
    for (int i = 0; i < (int)r.gantt.size(); i++) {
        const auto& g = r.gantt[i];
        file << "        {\"pid\": " << g.processId
             << ", \"start\": " << g.start
             << ", \"end\": "   << g.end << "}";
        if (i + 1 < (int)r.gantt.size()) file << ",";
        file << "\n";
    }
    file << "      ]\n";

    file << "    }";
    if (!last) file << ",";
    file << "\n";
}

// ─────────────────────────────────────────────
//  Input Reader (from data/input.json)
// ─────────────────────────────────────────────

// A minimal hand-written JSON parser for our specific input format:
// {
//   "quantum": 3,
//   "processes": [
//     {"id":1,"arrival":0,"burst":8},
//     ...
//   ]
// }

int parseIntAfter(const string& line, const string& key) {
    size_t pos = line.find("\"" + key + "\"");
    if (pos == string::npos) return -1;
    pos = line.find(":", pos);
    if (pos == string::npos) return -1;
    pos++;
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) pos++;
    string num;
    while (pos < line.size() && (isdigit(line[pos]) || line[pos] == '-')) {
        num += line[pos++];
    }
    return num.empty() ? -1 : stoi(num);
}

bool readInput(const string& filepath, vector<Process>& procs, int& fixedQuantum) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open input file: " << filepath << "\n";
        return false;
    }

    string content((istreambuf_iterator<char>(file)),
                    istreambuf_iterator<char>());
    file.close();

    // Extract top-level "quantum"
    {
        size_t pos = content.find("\"quantum\"");
        if (pos != string::npos) {
            pos = content.find(":", pos) + 1;
            while (pos < content.size() && !isdigit(content[pos])) pos++;
            string num;
            while (pos < content.size() && isdigit(content[pos])) num += content[pos++];
            if (!num.empty()) fixedQuantum = stoi(num);
        }
    }

    // Extract processes array by scanning for { blocks
    procs.clear();
    size_t arrStart = content.find("\"processes\"");
    if (arrStart == string::npos) {
        cerr << "ERROR: No 'processes' key in input file.\n";
        return false;
    }
    arrStart = content.find("[", arrStart);
    if (arrStart == string::npos) return false;

    size_t pos = arrStart;
    while ((pos = content.find("{", pos)) != string::npos) {
        size_t end = content.find("}", pos);
        if (end == string::npos) break;

        string block = content.substr(pos, end - pos + 1);

        int id      = parseIntAfter(block, "id");
        int arrival = parseIntAfter(block, "arrival");
        int burst   = parseIntAfter(block, "burst");

        if (id >= 0 && arrival >= 0 && burst > 0) {
            Process p;
            p.id        = id;
            p.arrival   = arrival;
            p.burst     = burst;
            p.remaining = burst;
            p.waiting   = p.turnaround = p.response = p.completion = 0;
            procs.push_back(p);
        }

        pos = end + 1;
    }

    return !procs.empty();
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────

int main(int argc, char* argv[]) {
    string inputPath  = "../data/input.json";
    string outputPath = "../data/output.json";

    // Allow custom paths via command-line arguments
    if (argc >= 3) {
        inputPath  = argv[1];
        outputPath = argv[2];
    }

    cout << "=== Adaptive Round Robin Scheduler ===\n";
    cout << "Reading input from: " << inputPath << "\n";

    // --- Read input ---
    vector<Process> processes;
    int fixedQuantum = 3;   // default if not in file

    if (!readInput(inputPath, processes, fixedQuantum)) {
        cerr << "Failed to read input. Exiting.\n";
        return 1;
    }

    cout << "Loaded " << processes.size() << " processes.\n";
    cout << "Fixed Quantum = " << fixedQuantum << "\n";

    // --- Compute adaptive quanta ---
    int avgQuantum    = computeAverageQuantum(processes);
    int medianQuantum = computeMedianQuantum(processes);

    cout << "Average Burst Quantum  = " << avgQuantum    << "\n";
    cout << "Median Burst Quantum   = " << medianQuantum << "\n";

    // --- Run all three algorithms ---
    SchedulerResult rr1 = runRoundRobin(processes, fixedQuantum,  "Standard Round Robin");
    SchedulerResult rr2 = runRoundRobin(processes, avgQuantum,    "Adaptive RR (Average)");
    SchedulerResult rr3 = runRoundRobin(processes, medianQuantum, "Adaptive RR (Median)");

    // --- Write output JSON ---
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "ERROR: Cannot open output file: " << outputPath << "\n";
        return 1;
    }

    outFile << fixed;
    outFile.precision(2);

    outFile << "{\n";
    outFile << "  \"status\": \"success\",\n";
    outFile << "  \"processCount\": " << processes.size() << ",\n";
    outFile << "  \"results\": [\n";
    writeResultToJson(outFile, rr1, false);
    writeResultToJson(outFile, rr2, false);
    writeResultToJson(outFile, rr3, true);
    outFile << "  ]\n";
    outFile << "}\n";

    outFile.close();
    cout << "Output written to: " << outputPath << "\n";
    cout << "=== Scheduling Complete ===\n";

    return 0;
}