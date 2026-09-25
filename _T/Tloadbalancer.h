
#pragma once


#include <Tbasis.h>


#include <cmath>
#include <random>
#include <fstream>

#include <loba/loadbalancer.h>



class TLoadBalancer {


public:

   static void Tdryrun() {
      // simple sanity checks

      LoadBalancer<5, 5, 2, 10, true> lb1;
      cout << lb1.str() << endl;
      lb1.rebalance();
      cout << lb1.str() << endl;

      cout << "-=-=-=-=-=-\n" << endl;

      LoadBalancer<7, 5, 2, 10, true> lb2;
      cout << lb2.str() << endl;
      lb2.rebalance();
      cout << lb2.str() << endl;
   }


   static void Tdryrun_rebalance1() {
      // rebalance after loading

      LoadBalancer<7, 5, 2, 10, true> lb;

      for (int i = 0; i < 7; ++i) {
         lb[i] = 10.0 + i * 10.0;
      }

      cout << lb.str() << endl;
      lb.rebalance();
      cout << "done rebalancing..." << endl;
      cout << lb.str() << endl;

   }

   static void Tdryrun_rebalance2() {
      // rebalance after loading

      LoadBalancer<6, 2, 2, 10, true> lb;

      lb[0] = 15;
      lb[1] = 50;
      lb[2] = 15;
      lb[3] = 50;
      lb[4] = 15;
      lb[5] = 50;

      cout << lb.str() << endl;
      lb.rebalance();
      cout << "done rebalancing..." << endl;
      cout << lb.str() << endl;

   }



   static void Tdryrun_rebalance3() {
      LoadBalancer<7, 5, 2, 10, true> lb;

      for (int i = 0; i < 7; ++i) {
         lb[i] = (7-i)*10.0;
      }

      cout << lb.str() << endl;
      lb.rebalance();
      cout << "done rebalancing..." << endl;
      cout << lb.str() << endl;

   }


   void Tdryrun_rebalance4() {
      LoadBalancer<10, 4, 2, 10, true> lb;

      lb[0] = 20.0;    lb[5] = 100.0;
      lb[1] = 2.0;    lb[6] = 2.0;
      lb[2] = 3.0;    lb[7] = 3.0;
      lb[3] = 4.0;    lb[8] = 4.0;
      lb[4] = 22.0;    lb[9] = 7.0;

      cout << lb.str() << endl;
      lb.rebalance();
      cout << lb.str() << endl;

   }



   /*
    * The work loads are assigned according to a normal distribution
    * with a gradually oscillating variance.
    */
   template <int nunits, int nstations>
   void Trandom_test1() {
      double mean = 100.0;
      double stddev = 20.0;
      std::random_device rd;
      std::mt19937 gen(rd());
      LoadBalancer<nunits, nstations, 2, 10, true> lb;
      std::normal_distribution<double> dist(mean, stddev);
      // load work
      for (size_t i = 0; i < nunits; ++i) {
         double w_random = dist(gen);
         lb[i] = (w_random < 1)? 1.0: w_random;
      }
      // rebalance
      cout << lb.str() << endl;
      lb.rebalance();
      cout << lb.str() << endl;
   }






   /*
    * The work loads are assigned according to a normal distribution
    * with a gradually oscillating variance.
    * This test reassigns the load between each timestep,
    * so that the load rebalancing is essentially doing the
    * same rebalancing task.
    */
   template <int nunits, int nstations, bool verbose=false>
   static void Tlongrunning_test1() {

      double mean = 50.0;
      double A = 40.0;
      {
         // store params
         std::string fname = "main_test_loadbalancer.params";
         std::ofstream outfile(fname);
         if (!outfile) {
            std::cerr << "Error opening file.\n";
         }
         else {
            outfile << "nunits nstations mean A\n";
            outfile << std::format("{:d} ", nunits) + std::format("{:d} ", nstations) + std::format("{:.12g} ", mean) + std::format("{:.12g} ", A);
         }
      }

      std::random_device rd;
      std::mt19937 gen(rd());
      LoadBalancer<nunits, nstations, 0, 10, verbose> lb;
      double stddev;
      // twice, the second time with rebalancing
      for (int iter = 0; iter < 2; ++iter) {
         std::string out;
         double x = 0.0;
         double dx = 0.01;
         while (x <= 6*M_PI) {
            stddev = A*(1.0 - cos(x)) + 1e-10;
            std::normal_distribution<double> dist(mean, stddev);
            // reload work
            for (size_t i = 0; i < nunits; ++i) {
               double w_random = dist(gen);
               lb[i] = (w_random < 1)? 1.0: w_random;
            }
            // rebalance
            if (iter != 0) lb.rebalance();
            // get aggregate work (bins of work)
            auto agg_work = lb.get_agg_work();
            // >>> write to .dat line
            // x, load at station 0, load at station 1, ... etc.
            out += std::format("{:.12g} ", x);
            for (size_t i = 0; i < nstations; ++i) {
               out += std::format("{:.12g} ", agg_work[i]);
            }
            out += "\n";
            x += dx;
         }
         // store data
         std::string fname = (iter != 0)? "main_test_loadbalancer.reb.dat": "main_test_loadbalancer.ref.dat";
         std::ofstream outfile(fname);
         if (!outfile) {
            std::cerr << "Error opening file.\n";
         }
         else {
            outfile << out;
         }
      }
   }



   /*
    *
    * The work loads are assigned according to a normal distribution
    * with a gradually oscillating variance.
    * This test accumulates loads between each timestep.
    */
   template <int nunits, int nstations>
   static void Tlongrunning_test2() {

      double mean = 50.0;
      double A = 40.0;
      {
         // store params
         std::string fname = "main_test_loadbalancer.params";
         std::ofstream outfile(fname);
         if (!outfile) {
            std::cerr << "Error opening file.\n";
         }
         else {
            outfile << "nunits nstations mean A\n";
            outfile << std::format("{:d} ", nunits) + std::format("{:d} ", nstations) + std::format("{:.12g} ", mean) + std::format("{:.12g} ", A);
         }
      }

      std::random_device rd;
      std::mt19937 gen(rd());

      LoadBalancer<nunits, nstations, 1, 10, false> lb;
      double stddev;
      // twice, the second time with rebalancing
      for (int iter = 0; iter < 2; ++iter) {
         for (size_t i = 0; i < nunits; ++i) lb[i] = mean;
         std::string out;
         double x = 0.0;
         double dx = 0.01;
         while (x <= 6*M_PI) {
            stddev = A*(1.0 - cos(x)) + 1e-10;
            std::normal_distribution<double> dist(0.0, stddev);
            // reload work
            for (size_t i = 0; i < nunits; ++i) {
               double w_random = dist(gen);
               if (lb[i] >= 1)
                  lb[i] += w_random;
               else
                  lb[i] = 1.0;
            }
            // rebalance
            if (iter != 0) lb.rebalance();
            // get aggregate work (bins of work)
            auto agg_work = lb.get_agg_work();
            // >>> write to .dat line
            // x, load at station 0, load at station 1, ... etc.
            out += std::format("{:.12g} ", x);
            for (size_t i = 0; i < nstations; ++i) {
               out += std::format("{:.12g} ", agg_work[i]);
            }
            out += "\n";
            x += dx;
         }
         // store data
         std::string fname = (iter != 0)? "main_test_loadbalancer.reb.dat": "main_test_loadbalancer.ref.dat";
         std::ofstream outfile(fname);
         if (!outfile) {
            std::cerr << "Error opening file.\n";
         }
         else {
            outfile << out;
         }
      }
   }




};





