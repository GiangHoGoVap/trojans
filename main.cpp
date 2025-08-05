#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <fstream>
#include <unordered_set>
#include "header/faculty.h"

using namespace std;

struct Individual {
    vector<int> chromosome; // Permutation of event_ids (0 .. total_events-1)
    int fitness = 0;

    bool operator<(const Individual& other) const {
        return fitness < other.fitness;
    }
};

Individual gen_random_individual(const Faculty& faculty) {
    Individual ind; // Each individual is a whole timetable on its own

    int total_events = 0;
    for (unsigned i = 0; i < faculty.Courses(); ++i) {
        total_events += faculty.CourseVector(i).Lectures();
    }

    ind.chromosome.resize(total_events);
    iota(ind.chromosome.begin(), ind.chromosome.end(), 0);  // Fill with 0..N-1
    shuffle(ind.chromosome.begin(), ind.chromosome.end(), default_random_engine(rand()));

    return ind;
}

struct EventAssignment {
    int course_id;
    int lecture_idx;
    int timeslot;
    int room_id;
};

bool is_feasible(
    const Faculty& faculty,
    const vector<vector<bool>>& room_occupancy,
    const vector<vector<bool>>& course_period_assigned,
    const vector<vector<int>>& curriculum_period_lectures,
    const EventAssignment& assignment) 
{
    unsigned course_id = assignment.course_id;
    unsigned period = assignment.timeslot;
    unsigned room_id = assignment.room_id;
    
    // 1. Room already occupied?
    if (room_occupancy[period][room_id]) return false;

    // 2. Course already assigned in that period?
    if (course_period_assigned[course_id][period]) return false;

    // 3. Curriculum conflict?
    for (unsigned g = 0; g < faculty.Curricula(); ++g) {
        if (faculty.CurriculumMember(course_id, g)) {
            if (curriculum_period_lectures[g][period] > 0) return false;
        }
    }

    // 4. Unavailable
    if (!faculty.Available(course_id, period)) return false;

    // 5. Room too small? (SOFT CONSTRAINT NOW: allow it)
    return true;
}

vector<EventAssignment> decode_individual(const Individual& ind, const Faculty& faculty) {
    vector<EventAssignment> assignments;
    int total_courses = faculty.Courses();
    int total_periods = faculty.Periods();
    int total_rooms = faculty.Rooms();
    int total_curricula = faculty.Curricula();

    // Tracking structures
    vector<vector<bool>> room_occupancy(total_periods, vector<bool>(total_rooms + 1, false));
    vector<vector<bool>> course_period_assigned(total_courses, vector<bool>(total_periods, false));
    vector<vector<int>> curriculum_period_lectures(total_curricula, vector<int>(total_periods, 0));
    vector<unordered_map<int, vector<int>>> curriculum_day_lectures(total_curricula);
    // curriculum_day_lectures[g][day] = list of periods where curriculum g has a lecture on that day

    unordered_map<unsigned, unordered_set<unsigned>> course_assigned_days;
    unordered_map<unsigned, vector<int>> course_assigned_rooms;

    int cumulative = 0;
    for (int event_id : ind.chromosome) {
        // Map event_id to course and lecture index
        int course_id = 0, lecture_idx = 0;
        cumulative = 0;
        for (unsigned i = 0; i < total_courses; ++i) {
            int n_lectures = faculty.CourseVector(i).Lectures();
            if (event_id < cumulative + n_lectures) {
                course_id = i;
                lecture_idx = event_id - cumulative;
                break;
            }
            cumulative += n_lectures;
        }

        bool assigned = false;

        for (int period = 0; period < total_periods && !assigned; ++period) {
            for (int room_id = 1; room_id <= total_rooms && !assigned; ++room_id) {
                EventAssignment assign = {course_id, lecture_idx, period, room_id};
                
                if (is_feasible(faculty, room_occupancy, course_period_assigned, curriculum_period_lectures, assign)) {
                    assignments.push_back(assign);

                    room_occupancy[period][room_id] = true;
                    course_period_assigned[course_id][period] = true;

                    int day = period / faculty.PeriodsPerDay();
                    course_assigned_days[course_id].insert(day);
                    course_assigned_rooms[course_id].push_back(room_id);

                    for (unsigned g = 0; g < total_curricula; ++g) {
                        if (faculty.CurriculumMember(course_id, g)) {
                            curriculum_period_lectures[g][period]++;
                            curriculum_day_lectures[g][day].push_back(period);
                        }
                    }

                    assigned = true;
                }
            }
        }
    }

    return assignments;
}

int evaluate_fitness(const Individual& ind, const Faculty& faculty) {
    auto assignments = decode_individual(ind, faculty);

    int penalty = 0;
    int n_courses = faculty.Courses();
    int n_periods = faculty.Periods();
    int n_curricula = faculty.Curricula();

    vector<set<int>> course_rooms(n_courses);       // for Room Stability
    vector<set<int>> course_days(n_courses);        // for Min Working Days
    vector<vector<int>> curriculum_periods(n_curricula, vector<int>(n_periods, 0));  // for Curriculum Compactness

    for (const auto& a : assignments) {
        if (a.timeslot == -1 || a.room_id == -1) continue;

        int n_students = faculty.CourseVector(a.course_id).Students();
        int cap = faculty.RoomVector(a.room_id).Capacity();
        if (n_students > cap) {
            penalty += (n_students - cap);  // Room Capacity violation
        }

        course_rooms[a.course_id].insert(a.room_id);  // Room Stability
        int day = a.timeslot / faculty.PeriodsPerDay();
        course_days[a.course_id].insert(day);  // Min Working Days

        for (int g = 0; g < n_curricula; ++g) {
            if (faculty.CurriculumMember(a.course_id, g)) {
                curriculum_periods[g][a.timeslot]++;
            }
        }
    }

    for (int c = 0; c < n_courses; ++c) {
        if (course_rooms[c].size() > 1) {
            penalty += static_cast<int>(course_rooms[c].size()) - 1;  // Room Stability violation
        }

        int required_days = faculty.CourseVector(c).MinWorkingDays();
        int actual_days = static_cast<int>(course_days[c].size());
        if (actual_days < required_days) {
            penalty += 5 * (required_days - actual_days);  // Min Working Days violation
        }
    }

    for (int g = 0; g < n_curricula; ++g) {
        for (int p = 0; p < n_periods; ++p) {
            if (curriculum_periods[g][p] == 1) {
                bool left = (p > 0) && curriculum_periods[g][p - 1] > 0;
                bool right = (p + 1 < n_periods) && curriculum_periods[g][p + 1] > 0;
                if (!left && !right) {
                    penalty += 2;  // Isolated lecture
                }
            }
        }
    }

    return penalty;
}

// --- Crossover ---
Individual order_crossover(const Individual& p1, const Individual& p2, double crossover_rate) {
    if ((rand() / (double)RAND_MAX) < crossover_rate) {
        int size = p1.chromosome.size();
        Individual child;
        child.chromosome.resize(size, -1);

        // Random subsequence from parent 1
        int start = rand() % size;
        int end = rand() % size;
        if (start > end) swap(start, end);

        set<int> genes;
        for (int i = start; i <= end; ++i) {
            child.chromosome[i] = p1.chromosome[i];
            genes.insert(p1.chromosome[i]);
        }

        // Fill remaining positions from parent 2
        int j = 0;
        for (int i = 0; i < size; ++i) {
            if (child.chromosome[i] == -1) {
                while (genes.count(p2.chromosome[j])) j++;
                child.chromosome[i] = p2.chromosome[j++];
            }
        }

        return child;
    }
    else {
        // No crossover, return a copy of parent 1
        return p1;
    }
}

// --- Mutation ---
void swap_mutation(Individual& ind, double mutation_rate = 0.1) {
    int size = ind.chromosome.size();
    for (int i = 0; i < size; ++i) {
        if ((rand() / (double)RAND_MAX) < mutation_rate) {
            int j = rand() % size;
            swap(ind.chromosome[i], ind.chromosome[j]);
        }
    }
}

// --- Selection (Tournament) ---
Individual tournament_selection(const vector<Individual>& population, int k = 3) {
    Individual best;
    best.fitness = INT_MAX;
    for (int i = 0; i < k; ++i) {
        int r = rand() % population.size();
        if (population[r].fitness < best.fitness) {
            best = population[r];
        }
    }
    return best;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    Faculty faculty(argv[1]);

    const int POP_SIZE = 30;
    const int MAX_GENERATIONS = 100;
    const double CROSSOVER_RATE = 0.8;
    const double MUTATION_RATE = 0.1;
    const bool USE_ELITISM = true;
    const int ELITE_COUNT = 5;

    vector<Individual> population(POP_SIZE);

    // Initialize population
    for (auto& ind : population) {
        ind = gen_random_individual(faculty);
        ind.fitness = evaluate_fitness(ind, faculty);
    }

    sort(population.begin(), population.end()); // Best first

    for (int gen = 0; gen < MAX_GENERATIONS; ++gen) {
        vector<Individual> new_population;

        // Elitism: carry over best individual
        if (USE_ELITISM) {
            for (int i = 0; i < ELITE_COUNT && i < population.size(); ++i) {
                new_population.push_back(population[i]);
            }
        }

        // Generate rest of the population
        while (new_population.size() < POP_SIZE) {
            Individual parent1 = tournament_selection(population);
            Individual parent2 = tournament_selection(population);

            Individual child = order_crossover(parent1, parent2, CROSSOVER_RATE);
            swap_mutation(child, MUTATION_RATE);
            child.fitness = evaluate_fitness(child, faculty);

            new_population.push_back(child);
        }

        population = std::move(new_population);
        sort(population.begin(), population.end());

        // Print progress
        cout << "Generation " << gen + 1 << ": Best fitness = " << population[0].fitness << endl;
    }

    // Output best solution
    const Individual& best = population[0];
    cout << "\nBest solution found:\nFitness: " << best.fitness << endl;

    ofstream outfile("output/my_sol00.out");
    if (!outfile) {
        cerr << "Failed to open output file.\n";
        return 1;
    }

    auto final_assignments = decode_individual(best, faculty);
    for (const auto& a : final_assignments) {
        int day = a.timeslot / faculty.PeriodsPerDay();
        int day_period = a.timeslot % faculty.PeriodsPerDay();
        outfile << faculty.CourseVector(a.course_id).Name() << " " << faculty.RoomVector(a.room_id).Name() << " " << day << " " << day_period << "\n";
    }

    outfile.close();

    return 0;
}