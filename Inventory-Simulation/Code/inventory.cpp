#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include "util.h"

using namespace std;

#define NUM_EVENTS 4

double expon(double mean) {
    // returns an exponentially distributed random variable with mean "mean"
    return -mean * log(longrand(1));
}

int random_integer(vector<double> prob_distribution) {
    // generates a random integer according to the specified probability distribution
    double u = longrand(1);
    int random_int = 1;
    while (u >= prob_distribution[random_int]) random_int++;
    return random_int;
}

double uniform(double a, double b) {
    // returns a uniformly distributed random variable in the range (a, b)
    return a + (b - a) * longrand(1);
}


class Inventory {
    int amount, bigs, smalls;
    int inventory_level, initial_inventory_level, next_event_type, num_months, num_values_demand;
    double holding_cost, incremental_cost, setup_cost, shortage_cost, total_ordering_cost;
    double min_lag, max_lag, mean_interdemand;
    vector<double> prob_distrib_demand;
    double sim_time, time_last_event, time_next_event[NUM_EVENTS + 1];
    double area_holding, area_shortage;

    void initialize() {
        sim_time = 0.0;
        inventory_level = initial_inventory_level;
        time_last_event = 0.0;

        area_holding = 0.0;
        area_shortage = 0.0;
        total_ordering_cost = 0.0;

        time_next_event[1] = 1.0e+30; // arrival of an order from the supplier
        time_next_event[2] = sim_time + expon(mean_interdemand); // demand from a customer
        time_next_event[3] = num_months; // end of the simulation after num_months
        time_next_event[4] = 0.0; // inventory evaluation and order placement (if needed) at the beginning of a month

    }

    void timing() {
        double min_time_next_event = 1e+29;

        next_event_type = 0;

        for (int i = 1; i <= NUM_EVENTS; i++) {
            if (time_next_event[i] < min_time_next_event) {
                min_time_next_event = time_next_event[i];
                next_event_type = i;
            }
        }

        if (next_event_type == 0) {
            cout << "---------Event list empty at time " << sim_time << "--------\n";
            exit(1);
        }

        sim_time = min_time_next_event; // advancing the simulation clock
    }

    void order_arrival() {
        inventory_level += amount; // increase inventory level by the amount ordered
        time_next_event[1] = 1.0e+30; // no more orders can be placed until the end of the month
    }

    void demand() {
        inventory_level -= random_integer(prob_distrib_demand); // decrease inventory level by the amount demanded
        time_next_event[2] = sim_time + expon(mean_interdemand); // schedule next demand
    }

    void evaluate() {
        if (inventory_level < smalls) {
            // inventory level is below the reorder point, an appropriate order needs to be placed
            amount = bigs - inventory_level; // order the difference between the big and small amounts
            total_ordering_cost += setup_cost + incremental_cost * amount;
            time_next_event[1] = sim_time + uniform(min_lag, max_lag); // schedule arrival of the order
        }
        time_next_event[4] = sim_time + 1.0; // schedule next inventory evaluation
    }

    void update_time_avg_stats() {
        // updates area_holding and area_shortage
        double time_since_last_event = sim_time - time_last_event;
        time_last_event = sim_time;

        // if the inventory level is negative, the amount is on backorder, so area_shortage is decreased
        // if the inventory level is positive, the amount is in inventory, so area_holding is increased
        if (inventory_level > 0) area_holding += inventory_level * time_since_last_event;
        else if(inventory_level < 0) area_shortage -= inventory_level * time_since_last_event;
        else {} // inventory level is 0, so nothing happens
    }

    void generate_report(ofstream &outfile) {
        double avg_ordering_cost = total_ordering_cost / num_months;
        double avg_holding_cost = holding_cost * area_holding / num_months;
        double avg_shortage_cost = shortage_cost * area_shortage / num_months;
        double avg_total_cost = avg_ordering_cost + avg_holding_cost + avg_shortage_cost;

        cout << "total: " << avg_total_cost << " order: " << avg_ordering_cost << " holding: " << avg_holding_cost << " shortage: " << avg_shortage_cost << "\n";

        outfile << "\n(" << std::setw(2) << smalls << "," << std::setw(3) << bigs << ")" << std::setw(20) 
        << avg_total_cost << std::setw(20) << avg_ordering_cost  << std::setw(20) << avg_holding_cost << 
        std::setw(20) << avg_shortage_cost << endl;
    }

public:
    Inventory(int bigs, int smalls, int initial_inventory_level, int num_months, int num_values_demand, 
            double holding_cost, double incremental_cost, double setup_cost, double shortage_cost, 
            double min_lag, double max_lag, double mean_interdemand, vector<double> prob_distrib_demand) {
        this->bigs = bigs;
        this->smalls = smalls;
        this->initial_inventory_level = initial_inventory_level;
        this->num_months = num_months;
        this->num_values_demand = num_values_demand;
        this->holding_cost = holding_cost;
        this->incremental_cost = incremental_cost;
        this->setup_cost = setup_cost;
        this->shortage_cost = shortage_cost;
        this->min_lag = min_lag;
        this->max_lag = max_lag;
        this->mean_interdemand = mean_interdemand;
        this->prob_distrib_demand = prob_distrib_demand;
    }

    void simulate(ofstream &outfile) {

        initialize();
        do {
            timing(); // determine the next event
            update_time_avg_stats(); // update time-average statistical accumulators

            if (next_event_type == 1) order_arrival();
            else if (next_event_type == 2) demand();
            else if (next_event_type == 4) evaluate();
            else if (next_event_type == 3) generate_report(outfile);
        } while (next_event_type != 3);
        
    }  

};


int main() {

    ifstream infile;
    ofstream outfile;

    int initial_inventory_level, num_months, num_policies, num_values_demand;
    double mean_interdemand, setup_cost, incremental_cost, holding_cost, shortage_cost, min_lag, max_lag;
    int smalls, bigs;

    infile.open("in.txt");
    if (!infile) {
        cout << "Error opening file" << endl;
        exit(1);
    }
    outfile.open("out.txt");
    if (!outfile) {
        cout << "Error opening file" << endl;
        exit(1);
    }

    // reading input from file
    infile >> initial_inventory_level >> num_months >> num_policies;
    infile >> num_values_demand >> mean_interdemand;
    infile >> setup_cost >> incremental_cost >> holding_cost >> shortage_cost;
    infile >> min_lag >> max_lag;

    vector<double> prob_distrib_demand(num_values_demand + 1);

    for (int i = 1; i <= num_values_demand; i++) infile >> prob_distrib_demand[i];

    // set precision of output to 2 decimal places
    outfile << std::fixed << std::setprecision(2);

    // writing output to file
    outfile << "------Single-Product Inventory System------" << endl;
    outfile << "\nInitial inventory level: " << initial_inventory_level << " items" << endl;
    outfile << "\nNumber of demand sizes: " << num_values_demand << endl;
    outfile << "\nDistribution function of demand sizes: ";
    for (int i = 1; i <= num_values_demand; i++) outfile << prob_distrib_demand[i] << " ";
    outfile << endl;
    outfile << "\nMean inter-demand time: " << mean_interdemand << " months" << endl; 
    outfile << "\nDelivery lag range: " << min_lag << " to " << max_lag << " months" << endl;
    outfile << "\nLength of simulation: " << num_months <<  " months" << endl;
    outfile << "\nCosts: " << endl;
    outfile << "K = " << setup_cost << endl;
    outfile << "i = " << incremental_cost << endl;
    outfile << "h = " << holding_cost << endl;
    outfile << "pi = " << shortage_cost << endl;
    outfile << "\nNumber of policies: " << num_policies << endl;
    outfile << "\nPolicies: " << endl;
    outfile << "--------------------------------------------------------------------------------------------------" << endl;
    outfile << " Policy        Avg_total_cost     Avg_ordering_cost      Avg_holding_cost     Avg_shortage_cost" << endl;
    outfile << "--------------------------------------------------------------------------------------------------" << endl;

    for (int i = 1; i <= num_policies; i++) {
        infile >> smalls >> bigs;
        Inventory inventory(bigs, smalls, initial_inventory_level, num_months, num_values_demand, holding_cost, 
        incremental_cost, setup_cost, shortage_cost, min_lag, max_lag, mean_interdemand, prob_distrib_demand);
        inventory.simulate(outfile);
    }

    outfile << "--------------------------------------------------------------------------------------------------" << endl;

    infile.close();
    outfile.close();

    return 0;
}