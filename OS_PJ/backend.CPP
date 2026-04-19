#include <bits/stdc++.h>
using namespace std;

struct Process {
    int id, arrival, burst, remaining;
    int waiting, turnaround, response, completion;
};

struct Gantt {
    int pid, start, end;
};

// ---------------- RR FUNCTION ----------------
void roundRobin(vector<Process> procs, int quantum, string name) {
    int n = procs.size();

    for (auto &p : procs) {
        p.remaining = p.burst;
        p.response = -1;
    }

    queue<int> q;
    vector<bool> inQueue(n, false);
    vector<Gantt> gantt;

    int time = 0, completed = 0;

    // initial push
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival == 0) {
            q.push(i);
            inQueue[i] = true;
        }
    }

    while (completed < n) {
        if (q.empty()) {
            time++;
            continue;
        }

        int i = q.front(); q.pop();

        if (procs[i].response == -1)
            procs[i].response = time - procs[i].arrival;

        int exec = min(quantum, procs[i].remaining);
        gantt.push_back({procs[i].id, time, time + exec});

        procs[i].remaining -= exec;
        time += exec;

        for (int j = 0; j < n; j++) {
            if (!inQueue[j] && procs[j].arrival <= time && procs[j].remaining > 0) {
                q.push(j);
                inQueue[j] = true;
            }
        }

        if (procs[i].remaining > 0)
            q.push(i);
        else {
            completed++;
            procs[i].completion = time;
            procs[i].turnaround = time - procs[i].arrival;
            procs[i].waiting = procs[i].turnaround - procs[i].burst;
        }
    }

    // -------- PRINT OUTPUT --------
    cout << "\n=== " << name << " ===\n";
    double wt = 0, tat = 0, rt = 0;

    cout << "ID  AT  BT  WT  TAT  RT\n";
    for (auto &p : procs) {
        cout << p.id << "   " << p.arrival << "   " << p.burst
             << "   " << p.waiting << "   " << p.turnaround
             << "   " << p.response << "\n";

        wt += p.waiting;
        tat += p.turnaround;
        rt += p.response;
    }

    cout << "Avg WT: " << wt / n << "\n";
    cout << "Avg TAT: " << tat / n << "\n";
    cout << "Avg RT: " << rt / n << "\n";

    // Gantt Chart
    cout << "Gantt Chart:\n";
    for (auto &g : gantt)
        cout << "| P" << g.pid << " ";
    cout << "|\n";
}

// ---------------- MAIN ----------------
int main() {

    // 🔹 HARDCODED INPUT (EDIT HERE IF NEEDED)
    vector<Process> processes = {
        {1, 0, 8},
        {2, 1, 4},
        {3, 2, 9},
        {4, 3, 5},
        {5, 4, 2}
    };

    int fixedQuantum = 3;

    // 🔹 Compute Adaptive Quantums
    vector<int> bursts;
    for (auto &p : processes) bursts.push_back(p.burst);

    // Average
    int avgQ = accumulate(bursts.begin(), bursts.end(), 0) / bursts.size();

    // Median
    sort(bursts.begin(), bursts.end());
    int medianQ = bursts[bursts.size() / 2];

    // 🔹 RUN ALGORITHMS
    roundRobin(processes, fixedQuantum, "Standard Round Robin");
    roundRobin(processes, avgQ, "Adaptive RR (Average)");
    roundRobin(processes, medianQ, "Adaptive RR (Median)");

    return 0;
}