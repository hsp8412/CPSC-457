
/// this is the ONLY file you should edit and submit to D2L

#include "deadlock_detector.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <set>

/// feel free to use the code below if you like:
/// ----------------------------------------------------------------
/// split(s) splits string s into vector of strings (words)
/// the delimiters are 1 or more whitespaces
static std::vector<std::string> split(const std::string &s)
{
    auto linec = s + " ";
    std::vector<std::string> res;
    bool in_str = false;
    std::string curr_word = "";
    for (auto c : linec)
    {
        if (isspace(c))
        {
            if (in_str)
                res.push_back(curr_word);
            in_str = false;
            curr_word = "";
        }
        else
        {
            curr_word.push_back(c);
            in_str = true;
        }
    }
    return res;
}

/// this is the function you need to (re)implement
/// -------------------------------------------------------------------------
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with 'index' set to the index that caused the deadlock, and 'procs' set
/// to contain names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with index=-1 and empty procs.

class Graph
{
private:
    // instead of ordered maps, two vectors are used to keep the adjacent list and out degree counts of each node
    // each node is assigned an index number, and its adjacent list and out degree can be accessed through the index
    std::vector<std::vector<int>> adj_list;
    std::vector<int> out_counts;


    // bidirectional mapping between the node name and the index
    int acc = 0;
    std::unordered_map<int, std::string> index_to_string;
    std::unordered_map<std::string, int> string_to_index;

public:
    // method to parse an incoming edge added
    bool parse_edge(std::string edge)
    {
        // get the process & resource node name and the arrow direction (assignment or request)
        std::vector<std::string> splitted = split(edge);
        std::string process = splitted[0];
        std::string resource = splitted[2];

        // mark the resource node with '$' at the end
        resource = resource + "$";

        // check whether the process and resource have both been added to the graph
        bool existingResource = string_to_index.find(resource) != string_to_index.end();
        bool existingProcess = string_to_index.find(process) != string_to_index.end();

        bool flag = existingProcess && existingResource;

        // if the edge is a request
        if (splitted[1] == "->")
        {
            // processing the process
            int process_index;
            // if the process already added
            if (existingProcess)
            {
                // get the index of the process node
                process_index = string_to_index[process];
                // increment the node's out count
                out_counts[process_index]++;
            }
            // if the process is new, add the process node 
            else
            {   // assign an index
                process_index = acc;

                // create mapping between index and process name
                index_to_string[acc] = process;
                string_to_index[process] = acc;

                // add out degree count entry(1) and empty adjacent list of the process node
                out_counts.push_back(1);
                std::vector<int> list;
                adj_list.push_back(list);
                acc++;
            }
            

            // processing resource
            // if the resource already exists
            if (existingResource)
            {   // get the index of the resource node
                // add the index of the process node to the resource node's adjacent list
                int index = string_to_index[resource];
                adj_list[index].push_back(process_index);
            }
            // if the resource node is new
            else
            {
                // create mapping between resource name and index
                index_to_string[acc] = resource;
                string_to_index[resource] = acc;

                // add out degree count entry(0) and the adjacent list(contains the process index) of the resource node
                out_counts.push_back(0);
                std::vector<int> list;
                list.push_back(process_index);
                adj_list.push_back(list);
                acc++;
            }
        }
        // if the edge is an assignment
        else
        {
            // processing the resource
            int resource_index;
            if (existingResource)
            {
                resource_index = string_to_index[resource];
                out_counts[resource_index]++;
            }
            else
            {
                resource_index = acc;

                index_to_string[acc] = resource;
                string_to_index[resource] = acc;

                out_counts.push_back(1);
                std::vector<int> list;
                adj_list.push_back(list);
                acc++;
            }

            // processing the process
            if (existingProcess)
            {
                int index = string_to_index[process];
                adj_list[index].push_back(resource_index);
            }
            else
            {
                index_to_string[acc] = process;
                string_to_index[process] = acc;

                out_counts.push_back(0);
                std::vector<int> list;
                list.push_back(resource_index);
                adj_list.push_back(list);
                acc++;
            }
        }

        // if both the resource node and the process node are existing nodes, then a deadlock may exist, return true
        // else return false
        return flag;
    }

    // perform topological sorting
    void topological_sort(std::vector<std::string> &result)
    {
        // copy out counts vector
        std::vector<int> out_counts_copy = out_counts;

        // vector for storing nodes with zero out counts
        std::vector<int> zeros;

        // initialization: get all nodes with zero out counts
        for (int i = 0; i < int(out_counts_copy.size()); i++)
        {
            if (out_counts_copy[i] == 0)
            {
                zeros.push_back(i);
            }
        }

        // main loop, runs until no more node in zeros vector
        while (zeros.size() != 0)
        {
            int node_index = zeros.back();
            zeros.pop_back();
            for (const auto &n : adj_list[node_index])
            {
                out_counts_copy[n]--;
                if (out_counts_copy[n] == 0)
                {
                    zeros.push_back(n);
                }
            }
        }

        // loop through the out counts vector to find out non-zero nodes
        for (int i = 0; i < int(out_counts_copy.size()); i++)
        {
            if (out_counts_copy[i] > 0)
            {
                std::string name = index_to_string[i];
                // only add processes to the deadlock processes list
                if (name.back() != '$')
                {
                    result.push_back(name);
                }
            }
        }
    }
};

// Main function to detect deadlock
Result detect_deadlock(const std::vector<std::string> &edges)
{
    // let's try to guess the results :)
    // Result result;
    // result.procs = split(" 12 7  7 ");
    // result.index = 6;
    // return result;

    Result result;
    Graph graph;
    for (int i = 0; i < (int)edges.size(); i++)
    {
        bool flag = graph.parse_edge(edges[i]);
        // perform the topological sort only if both of the two nodes are old nodes that already exist in the graph
        if (flag)
        {
            graph.topological_sort(result.dl_procs);
            if (result.dl_procs.size() != 0)
            {
                result.edge_index = i;
                return result;
            }
        }
    }
    result.edge_index = -1;
    return result;
}
