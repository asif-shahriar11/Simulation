#include <iostream>
#include <fstream>
#include <cmath>
#include "util.h"

using namespace std;

#define Q_LIMIT 100
#define BUSY 1
#define IDLE 0

# define NUM_EVENTS 2

float expon(float mean) {
    // returns an exponentially distributed random variable with mean "mean"
    return -mean * log(longrand(1));
}

class Event {
    public:
        int type;
        float time;
        Event(int type, float time) {
            this->type = type;
            this->time = time;
        }
};

/* single-server queueing system */
class SSQS {
    int next_event_type, num_custs_delayed, num_delays_required, num_events, num_in_q, server_status;
    float area_num_in_q, area_server_status, mean_interarrival, mean_service, sim_time, time_arrival[Q_LIMIT + 1], time_last_event, time_next_event[3], total_num_of_delays;
    int event_counter, arrival_counter, departure_counter;

    ofstream eventsfile; // for recording the order of events

    void initialize() {
        sim_time = 0.0;
        server_status = IDLE;
        num_in_q = 0;
        time_last_event = 0.0;

        num_custs_delayed = 0;
        total_num_of_delays = 0.0;
        area_num_in_q = 0.0; // for avg queue length metric
        area_server_status = 0.0; // for server utilization metric

        time_next_event[1] = sim_time + expon(mean_interarrival);
        time_next_event[2] = 1.0e+30;

        event_counter = 0;
        arrival_counter = 0;
        departure_counter = 0;
    }

    /* function to determine the next event */
    void timing() {
        float min_time_next_event = 1.0e+29;

        next_event_type = 0;

        for (int i = 1; i <= num_events; i++) {
            if (time_next_event[i] < min_time_next_event) {
                min_time_next_event = time_next_event[i];
                next_event_type = i;
            }
        }

        if (next_event_type == 0) {
            eventsfile << "---------Event list empty at time " << sim_time << "--------\n";
            exit(1);
        }

        sim_time = min_time_next_event; // advancing the simulation clock
    }

    void update_time_avg_stats() {
        // updates area accumulators for time-average statistics
        float time_since_last_event = sim_time - time_last_event;
        time_last_event = sim_time;

        area_num_in_q += num_in_q * time_since_last_event;

        area_server_status += server_status * time_since_last_event;
    }

    void arrive() {
        event_counter++;
        arrival_counter++;
        eventsfile << event_counter << ". Next event: Customer " << arrival_counter << " " << "Arrival" << "\n";

        float delay;

        time_next_event[1] = sim_time + expon(mean_interarrival); // scheduling next arrival

        if (server_status == BUSY) {
            num_in_q++; // incrementing queue length

            if (num_in_q > Q_LIMIT) {
                eventsfile << "---------Overflow of the array time_arrival at " << sim_time << "--------\n\n";
                exit(2);
            }

            time_arrival[num_in_q] = sim_time; // storing arrival time of this customer

        } 
        else {
            delay = 0.0;
            total_num_of_delays += delay;

            num_custs_delayed++;
            server_status = BUSY; // server is now busy
            eventsfile << "\n---------No. of customers delayed: "   << num_custs_delayed << "--------\n\n";

            time_next_event[2] = sim_time + expon(mean_service); // scheduling departure for this customer
        }
    }

    void depart() {
        event_counter++;
        departure_counter++;
        eventsfile << event_counter << ". Next event: Customer " << departure_counter << " " << "Departure" << "\n";

        float delay;

        if (num_in_q == 0) {
            // queue is empty
            server_status = IDLE;
            time_next_event[2] = 1.0e+30;
        } 
        else {
            num_in_q--;
            // computing delay of the customer in front of q who is beginning service
            delay = sim_time - time_arrival[1];
            total_num_of_delays += delay;

            num_custs_delayed++;
            time_next_event[2] = sim_time + expon(mean_service);
            eventsfile << "\n---------No. of customers delayed: "   << num_custs_delayed << "--------\n\n";

            // moving each customer in queue (if any) up one place
            for (int i = 1; i <= num_in_q; ++i) {
                time_arrival[i] = time_arrival[i + 1];
            }
        }
    }

public:
    SSQS(float mean_interarrival, float mean_service, int num_delays_required) {
        this->mean_interarrival = mean_interarrival;
        this->mean_service = mean_service;
        this->num_delays_required = num_delays_required;
        this->num_events = NUM_EVENTS;
    }


    void simulate() {
        initialize();

        eventsfile.open("event_orders.txt");
        if (!eventsfile) {
            cout << "Error opening file" << endl;
            exit(1);
        }

        // simulation runs as long as the number of customers delayed is less than the number of customers required
        while (num_custs_delayed < num_delays_required) {
            timing(); // determines the next event
            update_time_avg_stats(); // updates time-average statistical accumulators

            if(next_event_type == 1) arrive();
            else depart(); 
        }

        eventsfile.close();
        if (!eventsfile) {
            cout << "Error closing file" << endl;
            exit(1);
        }

    }

    void generate_report(ofstream &resultsfile) {

        resultsfile << "----Single-Server Queueing System----\n\n";
        resultsfile << "Mean inter-arrival time: " << mean_interarrival << " minutes\n";
        resultsfile << "Mean service time: " << mean_service << " minutes\n";
        resultsfile << "Number of customers: " << num_delays_required << "\n\n";

        resultsfile << "Average delay in queue: " << total_num_of_delays / num_custs_delayed << " minutes\n";
        resultsfile << "Average number in queue: " << area_num_in_q / sim_time << "\n";
        resultsfile << "Server utilization: " << area_server_status / sim_time << "\n";
        resultsfile << "Time simulation ended: " << sim_time << " minutes\n";

    }


};



int main() {

    // file pointers
    ifstream infile;
    ofstream resultsfile;

    float mean_interarrival, mean_service, num_delays_required;

    // reading input from file
    infile.open("in.txt");
    if (!infile) {
        cout << "Error opening file" << endl;
        exit(1);
    }
    infile >> mean_interarrival >> mean_service >> num_delays_required;
    infile.close();
    if (!infile) {
        cout << "Error closing file" << endl;
        exit(1);
    }

    SSQS ssqs(mean_interarrival, mean_service, num_delays_required); // initializing ssqs
    ssqs.simulate(); // running simulation

    // writing measures of performance to output file
    resultsfile.open("results.txt");
    if (!resultsfile) {
        cout << "Error opening file" << endl;
        exit(1);
    }
    ssqs.generate_report(resultsfile);    
    resultsfile.close();
    

    return 0;
}