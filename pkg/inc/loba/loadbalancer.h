
#pragma once

#include <iostream>
#include <queue>
#include <array>
#include <format>
#include <algorithm>



/*!
\brief Generic load balancer
\author Lucius Schoenbaum

Load balancing proceeds by recursive binary division of work at weighted midpoints.

template arguments:
\param[in] nunits: how many work units contributing work
\param[in] nstations: how many work stations performing work
\param[in] tolerance: the imbalance tolerated, in units of current average load (CAL).
   It is not guaranteed that, after rebalance, the imbalance will be below this threshold,
   but the rebalance should always (1) do no harm (2) remove all major imbalances or "spikes".
\param[in] rebalance_threshold: the size that the current average load (CAL) should exceed before
   the balancer imposes extra work on the system by rebalancing.
\param[in] verbose: whether to run with verbose messages to standard output.
\param[in] workfactor_t: the type of the work factor (weight) assigned to each work unit (default: double)
\param[in] UnitId_t: the type of the work unit identifier (default: size_t)
\param[in] StationId_t: the type of the work station identifier (default: size_t)
 */
template <int nunits, int nstations, int tolerance = 2, int rebalance_threshold = 10, bool verbose = false, typename workfactor_t = double, typename UnitId_t = size_t, typename StationId_t = size_t>
class LoadBalancer {

   struct Score {
      size_t mark;
      workfactor_t value;
      [[nodiscard]] std::string str() const {
         return "(" + std::to_string(mark) + ", " + std::to_string(value) + ")";
      }
   };

   std::array<workfactor_t, nunits> work;

   std::array<size_t, nstations+1> dividers;

   struct Pair {
      size_t beg;
      size_t end;
   };

   static_assert(nunits > 0, "Number of units must be a positive integer.");
   static_assert(nstations > 0, "Number of stations must be a positive integer.");
   static_assert(tolerance >= 0, "Tolerance (multiple of the average work per unit) must be a positive integer.");
   static_assert(rebalance_threshold > 0, "Rebalance threshold (for average work per unit) must be a positive integer.");
   static_assert(nunits >= nstations, "There cannot be fewer work units than work stations.");

public: // ctor:

   LoadBalancer() {
      // Populate dividers
      rebalance_flat();
      // Caller is responsible for populating work after computing work factors.
   }

   void rebalance_flat() {
      // Invariant: avg >= 1
      int avg = nunits/nstations;
      int rem = nunits % nstations;
      dividers[0] = 0;
      for (int i = 1; i < nstations; ++i) {
         dividers[i] = avg*i;
         if (avg > 1 && rem > 0) {
            dividers[i]++;
            rem--;
         }
      }
      dividers[nstations] = nunits;
   }

   void rebalance_impl() {
      // Let N = `nunits`, n = `nstations`.
      // Each score assigns a value to a divider of an
      // interval [a,b) into [a,c)+[c,b). Intervals must always
      // be nonempty. Therefore possible values of c
      // are 0 < c < N. This c is denoted `mark`.
      // The value `value` is defined as the load that
      // would result if the c were removed from the
      // list of dividers. As long as work on each unit
      // is > 0 (the case in applications) this is
      // a failproof procedure because parents cannot
      // be added before children, and even in that case
      // it should not throw exceptions.
      std::array<Score, nunits-1> scores;
      std::queue<Pair> queue;
      size_t i = 0;
      workfactor_t value, weighted_mid;
      queue.emplace(Pair{0,nunits});
      while (!queue.empty()) {
         auto p = queue.front();
         msg(std::format("beg {} end {}", p.beg, p.end));
         queue.pop();
         value = 0;
         for (size_t j = p.beg; j < p.end; ++j) {
            value += work[j];
         }
         weighted_mid = value/2;
         size_t mark = p.beg;
         workfactor_t tmp = 0.0;
         while (mark < p.end-1) {
            tmp += work[mark];
            mark++;
            if (tmp > weighted_mid) break;
         }
         // todo add condition to decrement mark, and test
         msg("mark " + std::to_string(mark));
         if (tmp > weighted_mid)
            msg("below mark receives the heavier load.");
         else
            msg("above mark receives the heavier load.");
         if (mark != p.beg) {
            scores[i++] = {mark, value};
            if (mark == p.beg+1) {
                  // do nothing
            }
            else {
               queue.emplace(Pair{p.beg, mark});
            }
            if (p.end == mark+1) {
               // do nothing
            }
            else {
               queue.emplace(Pair{mark, p.end});
            }
            msg("push " + std::to_string(p.beg) + " " + std::to_string(mark));
            msg("push " + std::to_string(mark) + " " + std::to_string(p.end));
         }

         msg("i " + std::to_string(i));
      }
      show_array("scores", scores);
      std::ranges::sort(scores, std::greater<>{}, &Score::value);
      show_array("scores", scores);
      dividers[0] = 0;
      // store the n-1 marks of highest value
      for (i = 0; i < nstations; ++i) {
         dividers[i+1] = scores[i].mark;
      }
      // we bound the dividers on both sides;
      // this makes the code a tad more robust
      dividers[nstations] = nunits;
      // sort the intermediate dividers
      std::ranges::sort(dividers, {});
   }

public:

   // Bracket notation
   workfactor_t & operator[](UnitId_t index) {
      return work[index];
   }

   const workfactor_t & operator[](size_t index) const {
      return work[index];
   }

   void rebalance() {
      if constexpr (nstations == nunits) {
         // nothing to do
         return;
      }
      else {
         workfactor_t min_agg_work = 1e30;
         workfactor_t max_agg_work =-1e30;
         workfactor_t total = 0.0;
         workfactor_t agg_work[nstations];
         for (int i = 0; i < nstations; ++i) agg_work[i] = 0.0;
         size_t aggi = 0;
         // evaluate delta and avg
         for (size_t i = 0; i < nunits; ++i) {
            if (i == dividers[aggi+1]) {
//               msg(" i " + std::to_string(i) + " aggi " + std::to_string(aggi));
               aggi++;
            }
            workfactor_t wi = work[i];
            total += wi;
            agg_work[aggi] += wi;
         }
         for (size_t i = 0; i < nstations; ++i) {
            auto awi = agg_work[i];
            min_agg_work = (min_agg_work < awi)? min_agg_work: awi;
            max_agg_work = (max_agg_work > awi)? max_agg_work: awi;
         }
         // use the average work per unit as a tolerance;
         // the balancer would struggle to do better
         workfactor_t delta = max_agg_work - min_agg_work;
         // the average load
         workfactor_t avg = total/static_cast<workfactor_t>(nstations);
         // std::cout << "avg " << avg << " min " << min_agg_work << " max " << max_agg_work << std::endl;
         workfactor_t tolerance_ = tolerance*avg/static_cast<workfactor_t>(nunits);
         if (delta < tolerance_) {
            std::cout << "delta < tol" << std::endl;
            auto msg_ = std::format("Load imbalance under tolerance; no rebalance. delta {} tolerance {} avg {}", delta, tolerance_, avg);
            msg(msg_);
         }
         else if (avg < static_cast<workfactor_t>(rebalance_threshold)) {
            // When loads are low, there's no need to balance them.
            std::cout << "avg < thresh" << std::endl;
            auto msg_ = std::format("Average work below threshold; no rebalance. delta {} tolerance {} avg {}", delta, tolerance_, avg);
            msg(msg_);
         }
         else {
            auto msg_ = std::format("rebalancing... delta {} tolerance {} avg {}", delta, tolerance_, avg);
            msg(msg_);
            rebalance_impl();
         }
      } // if constexpr (nunits == nstations)
   }


   std::array<workfactor_t, nstations> get_agg_work() {
      std::array<workfactor_t, nstations> out;
      for (int i = 0; i < nstations; ++i) out[i] = 0.0;
      size_t aggi = 0;
      for (size_t i = 0; i < nunits; ++i) {
         if (i == dividers[aggi+1]) {
            aggi++;
         }
         out[aggi] += work[i];
      }
      return out;
   }

   std::array<size_t, nstations+1> get_dividers() {
      std::array<size_t, nstations+1> out = dividers;
      return out;
   }

   [[nodiscard]] std::string str_dat() const {
      std::string out;
      for (size_t i = 0; i < nunits; ++i) {
         out += std::to_string(work[i]) + " ";
      }
      out += "\n";
      for (size_t i = 0; i <= nstations; ++i) {
         out += std::to_string(dividers[i]) + " ";
      }
      out += "\n";
      out += std::to_string(tolerance);
      return out;
   }

   [[nodiscard]] std::string str() const {
      std::string out = "work: (";
      for (size_t i = 0; i < nunits-1; ++i) {
         out += std::to_string(work[i]) + ", ";
      }
      out += std::to_string(work[nunits-1]) + ")\ndividers: (";
      for (size_t i = 0; i < nstations; ++i) {
         out += std::to_string(dividers[i]) + ", ";
      }
      out += std::to_string(dividers[nstations]) + ")\n";
      out += "tolerance: " + std::to_string(tolerance) + "\n";
      return out;
   }

   void msg(std::string x) {
      if constexpr (verbose) {
         std::cout << "[LoadBalancer] " << x << std::endl;
      }
   }

   template <typename T, size_t N>
   void show_array(std::string name, std::array<T, N> arr) {
      if constexpr (verbose) {
         std::cout << "[LoadBalancer] " << name << " ";
         for (size_t i = 0; i < N; ++i) {
            std::cout << arr[i].str() << " ";
         }
         std::cout << std::endl;
      }
   }

};


