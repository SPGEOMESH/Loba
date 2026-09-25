
#pragma once

#include <Tbasis.h>

#include <cmath>
#include <random>

#include <loba/loadbalancer.h>


class TLoadBalancer_Clinic {
public:


    template <int nunits, int nstations>
    static void clinic1(int ntimesteps) {
        double mean = 100.0;
        double A = 10.0;
        std::random_device rd;
        std::mt19937 gen(rd());
        LoadBalancer<nunits, nstations, 0, 10, true> lb;
        double stddev;
        std::string out;
        double x = 0.0;
        double dx = 0.1;
        int ii = 0;
        while (ii < ntimesteps) {
            stddev = A*(1.0 - cos(x)) + 1e-10;
            std::normal_distribution<double> dist(mean, stddev);
            cout << "reloading w/ normal dist" << endl;
            // reload work
            for (size_t i = 0; i < nunits; ++i) {
                double w_random = dist(gen);
                lb[i] = (w_random < 1)? 1.0: w_random;
            }
            // rebalance
            lb.rebalance();
            out += lb.str();
            // get aggregate work (bins of work)
            auto agg_work = lb.get_agg_work();
            out += "agg_work: ";
            for (size_t i = 0; i < nstations; ++i) {
                out += std::format("{:.12g} ", agg_work[i]);
            }
            out += "\n";
            x += dx;
            ++ii;
        }
        cout << out << endl;
    }


};




