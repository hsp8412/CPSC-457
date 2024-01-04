
/// this is the only file you should modify and submit for grading

#include "scheduler.h"
#include <algorithm>
#include <iostream>

// this is the function you should implement
//
// simulate_rr() implements a Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrivals, and bursts
// output:
//   seq[] - will contain the compressed execution sequence
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//         - sequence will be trimmed to contain only first max_seq_len entries
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//

// optimization for the "Best Solution"
void optimization(const std::vector<int> &rq, std::vector<Process> &processes, std::vector<int64_t> &remaining_bursts, int64_t &curr_time, const int64_t quantum, const int64_t N, const int64_t max_seq_len, std::vector<int> &seq)
{
    // loop through the ready queue
    for (int i = 0; i < (int)rq.size(); i++)
    {
        auto proc_idx = rq[i];
        // if the process is being executed for the first time, set its start time
        if (processes[proc_idx].burst == remaining_bursts[proc_idx])
        {
            processes[proc_idx].start_time = curr_time + i * quantum;
        }
        // decrement the remaining burst time of the process by N * quantum
        remaining_bursts[proc_idx] -= N * quantum;
    }

    // record the processes executed in the seq vector
    int64_t space_count = max_seq_len - seq.size();
    int rq_size = (int)rq.size();

    // if there are remaining spaces in the seq vector
    if(space_count>0){
        // if the ready queue is consist of a single process, and the process is not the same as the last entry in seq, add it to the seq one time
        if(rq_size == 1 && (seq.empty() || rq[0] != seq.back())){
            seq.push_back(rq[0]);
        }
        // if there are multiple processes in the ready queue
        else if(rq_size > 1){
            // check if the first process in the queue is the same as the last entry in seq. If not, add it to seq.
            if(seq.empty() || rq[0] != seq.back()){
                seq.push_back(rq[0]);
                space_count--;
            }

            // total number of records that can be created (exclude the first one that has been handled)
            int64_t record_count = N * rq_size - 1;

            // get the number of records to insert
            int64_t num_to_insert = std::min(space_count, record_count);
           
            // insert records
            for(int64_t i = 0; i < num_to_insert; i++){
                seq.push_back(rq[(i+1)%rq_size]);
            }
        }
    }

    // increment the current time by N * quantum * size of ready queue
    int64_t time_increment = N * quantum * rq.size();
    curr_time += time_increment;
}

void simulate_rr(
    int64_t quantum,
    int64_t max_seq_len,
    std::vector<Process> &processes,
    std::vector<int> &seq)
{
    std::vector<int> jq;
    std::vector<int> rq;

    std::vector<int64_t> remaining_bursts;

    int64_t curr_time = 0;

    seq.clear();
    seq.shrink_to_fit();

    // initialize job queue remaining_bursts vector
    for (int i = 0; i < (int)processes.size(); i++)
    {
        // add processes to job queue/ready queue
        if (processes[i].arrival_time > 0)
        {
            jq.push_back(i);
        }
        else
        {
            rq.push_back(i);
        }

        // add initial burst time of processes to remaining_bursts
        remaining_bursts.push_back(processes[i].burst);
    }

    // main loop, stop when jq and rq are both empty
    while (jq.size() != 0 || rq.size() != 0)
    {
        // when rq is empty but jq is not empty
        if (rq.size() == 0)
        {
            // pop out next process to arrive in job queue
            auto p = jq[0];
            jq.erase(jq.begin());

            // add the process to the empty ready queue
            rq.push_back(p);

            // skip to the arrival time of next process in job queue
            curr_time = processes[p].arrival_time;
            
            // if there are other processes arrive at the same time, move them to the ready queue as well
            int counter = 0;
            for(auto i : jq){
                if(processes[i].arrival_time == curr_time){
                    rq.push_back(i);
                    counter++;
                }else{
                    break;
                }
            }

            for(int i = 0; i<counter;i++){
                jq.erase(jq.begin());
            }

            // record -1 in the seq vector
            seq.push_back(-1);
        }

        // when jq is empty but rq is not empty
        else if (jq.size() == 0)
        {
            // find the minimum remaining burst time of all process in ready queue
            int64_t min_remaining_burst = remaining_bursts[rq[0]];
            for (auto i : rq)
            {
                if (remaining_bursts[i] < min_remaining_burst)
                {
                    min_remaining_burst = remaining_bursts[i];
                }
            }

            // if to optimize, N * quantum should be smaller than the minimum remaining burst time to ensure that no process finishes during the skipped time
            int64_t N = (min_remaining_burst-1) / quantum;

            // if N==0, minimum remaining burst time is smaller or equal to a quantum, so no optimization
            if (N == 0)
            {
                // pop a process from ready queue
                auto cur_idx = rq[0];
                rq.erase(rq.begin());

                // get the original remaining burst of the current process
                auto remaining_burst = remaining_bursts[cur_idx];

                // if the current process is being executed the first time, set its start time
                if (processes[cur_idx].burst == remaining_burst)
                {
                    processes[cur_idx].start_time = curr_time;
                }

                // if the current process finishes within the quantum
                if (remaining_burst <= quantum)
                {
                    // add its remaining burst time to current time
                    curr_time += remaining_burst;
                    // set finish time
                    processes[cur_idx].finish_time = curr_time;
                    // remaining burst is 0
                    remaining_bursts[cur_idx] = 0;
                }

                // if the process does not finish after executing a quantum
                else
                {
                    // add a quantum to the current time
                    curr_time += quantum;
                    // subtract the quantum value from the remaining burst time of the current process
                    remaining_bursts[cur_idx] -= quantum;
                    // push the process to the back of the ready queue
                    rq.push_back(cur_idx);
                }

                // record the current process index in the seq vector
                if (((int)seq.size() == 0) || ((int)seq.size() < max_seq_len && cur_idx != seq.back()))
                {
                    seq.push_back(cur_idx);
                }
            }

            // with optimization
            else
            {
                optimization(rq, processes, remaining_bursts, curr_time, quantum, N, max_seq_len, seq);
            }
        }
        // jq and rq are not empty
        else
        {
            // calculate N that ensures no process finish
            // find the minimum remaining burst time of all process in ready queue
            int64_t min_remaining_burst = remaining_bursts[rq[0]];
            for (auto i : rq)
            {
                if (remaining_bursts[i] < min_remaining_burst)
                {
                    min_remaining_burst = remaining_bursts[i];
                }
            }

            // if to optimize, N * quantum should be smaller than the minimum remaining burst time to ensure that no process finishes during the skipped time
            int64_t N1 = (min_remaining_burst-1) / quantum;

            // calculate N that ensures no new process arrive during the skipped time
            int next_arr_idx = jq[0];
            int64_t next_arr_time = processes[next_arr_idx].arrival_time - curr_time;

            // N * quantum * size of ready queue should be less than the time that next process will arrive at ready queue
            int64_t N2 = (next_arr_time-1) / (quantum * rq.size());

            // take the smaller one between N1 and N2 to ensure that no processes will finish during the skipped time, and no process will arrive
            int64_t N = std::min(N1, N2);

            // without optimization
            if (N == 0)
            {
                // pop a process from ready queue
                auto cur_idx = rq[0];
                rq.erase(rq.begin());

                auto remaining_burst = remaining_bursts[cur_idx];

                // if the process is being executed for the first time, set its start time
                if (remaining_burst == processes[cur_idx].burst)
                {
                    processes[cur_idx].start_time = curr_time;
                }

                // if the process finishes within the quantum
                if (remaining_burst <= quantum)
                {
                    curr_time += remaining_burst;
                    processes[cur_idx].finish_time = curr_time;
                    remaining_bursts[cur_idx] = 0;
                }

                // if the process does not finish after executing a quantum
                else
                {
                    curr_time += quantum;
                    remaining_bursts[cur_idx] -= quantum;
                }

                // check if any process arrived during the execution of this quantum
                int counter = 0;
                for (auto proc_idx : jq)
                {
                    // if the process arrive before the current process executing, increment counter add it to the ready queue
                    if (processes[proc_idx].arrival_time < curr_time)
                    {
                        rq.push_back(proc_idx);
                        counter++;
                    }
                    else
                    {
                        break;
                    }
                }

                // remove arrived processes from job queue
                for (int i = 0; i < counter; i++)
                {
                    jq.erase(jq.begin());
                }

                // if the current process does not finish yet, put it back to the ready queue
                if (remaining_bursts[cur_idx] > 0)
                {
                    rq.push_back(cur_idx);
                }

                // check if any process arrived at the same time when the execution of this quantum finishes
                counter = 0;
                for (auto proc_idx : jq)
                {
                    // if the process arrive at the same time when the execution of this quantum finishes, increment counter add it to the ready queue
                    if (processes[proc_idx].arrival_time == curr_time)
                    {
                        rq.push_back(proc_idx);
                        counter++;
                    }
                    else
                    {
                        break;
                    }
                }

                // remove arrived processes from job queue
                for (int i = 0; i < counter; i++)
                {
                    jq.erase(jq.begin());
                }

                // record the current process index in the seq vector
                if (((int)seq.size() == 0) || ((int)seq.size() < max_seq_len && cur_idx != seq.back()))
                {
                    seq.push_back(cur_idx);
                }
            }
            // with optimization
            else
            {
                optimization(rq, processes, remaining_bursts, curr_time, quantum, N, max_seq_len, seq);
            }
        }
    }
}
