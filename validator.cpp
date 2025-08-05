#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "header/room.h"
#include "header/curriculum.h"
#include "header/course.h"
#include "header/faculty.h"
#include "header/timetable.h"

using namespace std;

class Validator {
    private:
        const Faculty& in;
        const Timetable& out;
    private:
        unsigned CostsOnLectures() const {
            unsigned c, p, cost = 0, lectures;
            for (c = 0; c < in.Courses(); c++) {
                lectures = 0;
                for (p = 0; p < in.Periods(); p++) {
                    if (out(c, p) != 0) {
                        lectures++;
                    }
                }
                if (lectures < in.CourseVector(c).Lectures()) {
                    cost += in.CourseVector(c).Lectures() - lectures;
                } else if (lectures > in.CourseVector(c).Lectures()) {
                    cost += lectures - in.CourseVector(c).Lectures();
                }
            }
            return cost;
        }	      

        unsigned CostsOnConflicts() const {
            unsigned c1, c2, p, cost = 0;
            for (c1 = 0; c1 < in.Courses(); c1++) {
                for (c2 = c1 + 1; c2 < in.Courses(); c2++) {
                    if (in.Conflict(c1, c2)) {
                        for (p = 0; p < in.Periods(); p++) {
                            if (out(c1, p) != 0 && out(c2, p) != 0) {
                                cost++;
                            }
                        }
                    }
                }
            }
            return cost;
        }

        unsigned CostsOnAvailability() const {
            unsigned c, p, cost = 0;
            for (c = 0; c < in.Courses(); c++) {
                for (p = 0; p < in.Periods(); p++) {
                    if (out(c, p) != 0 && !in.Available(c, p)) {
                        cost++;
                    }
                }
            }
            return cost;
        }       
        
        unsigned CostsOnRoomOccupation() const {
            unsigned r, p, cost = 0;
            for (p = 0; p < in.Periods(); p++) {
                for (r = 1; r <= in.Rooms(); r++) {
                    if (out.RoomLectures(r, p) > 1) {
                        cost += out.RoomLectures(r, p) - 1;
                    }
                }
            }
            return cost;
        }   

        unsigned CostsOnRoomCapacity() const {
            unsigned c, p, r, cost = 0;
            for (c = 0; c < in.Courses(); c++) {
                for (p = 0; p < in.Periods(); p++) {
                    r = out(c, p);
                    if (r != 0 && in.RoomVector(r).Capacity() < in.CourseVector(c).Students()) {
                        cost += in.CourseVector(c).Students() - in.RoomVector(r).Capacity();
                    }
                }
            }
            return cost;
        }  

        unsigned CostsOnMinWorkingDays() const {
            unsigned c, cost = 0;
            for (c = 0; c < in.Courses(); c++) {
                if (out.WorkingDays(c) < in.CourseVector(c).MinWorkingDays()) {
                    cost += in.CourseVector(c).MinWorkingDays() - out.WorkingDays(c);
                }
            }
            return cost;
        }

        unsigned CostsOnCurriculumCompactness() const {
            unsigned g, p, cost = 0, ppd = in.PeriodsPerDay();
            for (g = 0; g < in.Curricula(); g++) {
                for (p = 0; p < in.Periods(); p++) {
                    if (out.CurriculumPeriodLectures(g, p) > 0) {
                        if (p % ppd == 0 && out.CurriculumPeriodLectures(g, p + 1) == 0) {
                            cost += out.CurriculumPeriodLectures(g, p);
                        } else if (p % ppd == ppd - 1 && out.CurriculumPeriodLectures(g, p - 1) == 0) {
                            cost += out.CurriculumPeriodLectures(g, p);
                        } else if (out.CurriculumPeriodLectures(g, p + 1) == 0 && out.CurriculumPeriodLectures(g, p - 1) == 0) {
                            cost += out.CurriculumPeriodLectures(g, p);
                        }
                    }
                }
            }
            return cost;
        }

        unsigned CostsOnRoomStability() const {
            unsigned c, cost = 0;
            for (c = 0; c < in.Courses(); c++) {
                if (out.UsedRoomsNo(c) > 1) {
                    cost += out.UsedRoomsNo(c) - 1;
                }
            }
            return cost;
        }

        void PrintViolationsOnLectures(std::ostream& os) const {
            unsigned c, p, lectures;
            for (c = 0; c < in.Courses(); c++) {
                lectures = 0;
                for (p = 0; p < in.Periods(); p++) {
                    if (out(c, p) != 0) {
                        lectures++;
                    }
                }
                if (lectures < in.CourseVector(c).Lectures()) {
                    os << "[H] Too few lectures for course " << in.CourseVector(c).Name() << endl;
                } else if (lectures > in.CourseVector(c).Lectures()) {
                    os << "[H] Too many lectures for course " << in.CourseVector(c).Name() << endl;
                }
            }
        }

        void PrintViolationsOnConflicts(std::ostream& os) const {
            unsigned c1, c2, p;
            for (c1 = 0; c1 < in.Courses(); c1++) {
                for (c2 = c1 + 1; c2 < in.Courses(); c2++) {
                    if (in.Conflict(c1, c2)) {
                        for (p = 0; p < in.Periods(); p++) {
                            if (out(c1, p) != 0 && out(c2, p) != 0) {
                                os << "[H] Courses " << in.CourseVector(c1).Name() << " and " << in.CourseVector(c2).Name() << " have both a lecture at period " << p << " (day " << p/in.PeriodsPerDay() << ", timeslot " << p % in.PeriodsPerDay() << ")" << endl;
                            }
                        }
                    }
                }
            }
        }

        void PrintViolationsOnAvailability(std::ostream& os) const {
            unsigned c, p;
            for (c = 0; c < in.Courses(); c++) {
                for (p = 0; p < in.Periods(); p++) {
                    if (out(c, p) != 0 && !in.Available(c, p)) {
                        os << "[H] Course " << in.CourseVector(c).Name() << " has a lecture at unavailable period " << p << " (day " << p/in.PeriodsPerDay() << ", timeslot " << p % in.PeriodsPerDay() << ")" << std::endl;
                    }
                }
            }
        }

        void PrintViolationsOnRoomOccupation(std::ostream& os) const {
            unsigned r, p;
            for (p = 0; p < in.Periods(); p++) {
                for (r = 1; r <= in.Rooms(); r++) {
                    if (out.RoomLectures(r, p) > 1) {
                        os << "[H] " << out.RoomLectures(r,p) << " lectures in room " << in.RoomVector(r).Name() << " the period " << p << " (day " << p/in.PeriodsPerDay() << ", timeslot " << p % in.PeriodsPerDay() << ")"; 
                        if (out.RoomLectures(r, p) > 2) {
                            os << " [" << out.RoomLectures(r,p) - 1 << " violations]";
                        }
                        os << endl;
                    }
                }
            }
        }

        void PrintViolationsOnRoomCapacity(std::ostream& os) const {
            unsigned c, p, r;
            for (c = 0; c < in.Courses(); c++) {
                for (p = 0; p < in.Periods(); p++) {
                    r = out(c, p);
                    if (r != 0 && in.RoomVector(r).Capacity() < in.CourseVector(c).Students()) {
                        os << "[S(" << in.CourseVector(c).Students() - in.RoomVector(r).Capacity() << ")] Room " << in.RoomVector(r).Name() << " too small for course " << in.CourseVector(c).Name() << " the period " << p << " (day " << p/in.PeriodsPerDay() << ", timeslot " << p % in.PeriodsPerDay() << ")" << endl;
                    }
                }
            }
        }

        void PrintViolationsOnMinWorkingDays(std::ostream& os) const {
            unsigned c;
            for (c = 0; c < in.Courses(); c++) {
                if (out.WorkingDays(c) < in.CourseVector(c).MinWorkingDays()) {
                    os << "[S(" << in.MIN_WORKING_DAYS_COST << ")] The course " << in.CourseVector(c).Name() << " has only " << out.WorkingDays(c) << " days of lecture" << std::endl;
                }
            }
        }

        void PrintViolationsOnCurriculumCompactness(std::ostream& os) const {
            unsigned g, p, ppd = in.PeriodsPerDay();
            for (g = 0; g < in.Curricula(); g++) {
                for (p = 0; p < in.Periods(); p++) {
                    if (out.CurriculumPeriodLectures(g, p) > 0) {
                        if ((p % ppd == 0 && out.CurriculumPeriodLectures(g, p + 1) == 0) || (p % ppd == ppd - 1 && out.CurriculumPeriodLectures(g, p - 1) == 0) || (out.CurriculumPeriodLectures(g, p + 1) == 0 && out.CurriculumPeriodLectures(g, p - 1) == 0))
                            os << "[S(" << in.CURRICULUM_COMPACTNESS_COST << ")] Curriculum " << in.CurriculaVector(g).Name() << " has an isolated lecture at period " << p << " (day " << p/in.PeriodsPerDay() << ", timeslot " << p % in.PeriodsPerDay() << ")" << endl;
                    }
                }
            }
        }

        void PrintViolationsOnRoomStability(std::ostream& os) const {
            vector<unsigned> used_rooms;
            unsigned c;
            for (c = 0; c < in.Courses(); c++) {
                if (out.UsedRoomsNo(c) > 1) {
                    os << "[S(" << (out.UsedRoomsNo(c) - 1) * in.ROOM_STABILITY_COST << ")] Course " << in.CourseVector(c).Name() << " uses " << out.UsedRoomsNo(c) << " different rooms" << std::endl;
                }
            }
        }

    public:
        Validator(const Faculty& f, const Timetable& t) : in(f), out(t) {}
        
        void PrintCosts(ostream& os) const {
            os << "Violations of Lectures (hard) : " << CostsOnLectures() << endl;
            os << "Violations of Conflicts (hard) : " << CostsOnConflicts() << endl;
            os << "Violations of Availability (hard) : " << CostsOnAvailability() << endl;
            os << "Violations of RoomOccupation (hard) : " << CostsOnRoomOccupation() << endl;
            os << "Cost of RoomCapacity (soft) : " << CostsOnRoomCapacity() << endl;
            os << "Cost of MinWorkingDays (soft) : " << CostsOnMinWorkingDays() * in.MIN_WORKING_DAYS_COST << endl;
            os << "Cost of CurriculumCompactness (soft) : " << CostsOnCurriculumCompactness() * in.CURRICULUM_COMPACTNESS_COST << endl;
            os << "Cost of RoomStability (soft) : " << CostsOnRoomStability() * in.ROOM_STABILITY_COST << endl;
        }

        void PrintViolations(ostream& os) const {
            PrintViolationsOnLectures(os);
            PrintViolationsOnConflicts(os);
            PrintViolationsOnAvailability(os);
            PrintViolationsOnRoomOccupation(os);
            PrintViolationsOnRoomCapacity(os);
            PrintViolationsOnMinWorkingDays(os);
            PrintViolationsOnCurriculumCompactness(os);
            PrintViolationsOnRoomStability(os);
        }
        
        void PrintTotalCost(ostream& os) const {
            unsigned violations = CostsOnLectures() + CostsOnConflicts() + CostsOnAvailability() + CostsOnRoomOccupation();
            if (violations > 0)
                os << "Violations = " << violations << ", ";
            os << "Total Cost = " << CostsOnRoomCapacity() + CostsOnMinWorkingDays() * in.MIN_WORKING_DAYS_COST
               + CostsOnCurriculumCompactness() * in.CURRICULUM_COMPACTNESS_COST + CostsOnRoomStability() * in.ROOM_STABILITY_COST << endl;
        }    
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage:  " << argv[0] << " <input_file> <solution_file> " << endl;
        exit(1);
    }

    Faculty input(argv[1]);
    Timetable output(input, argv[2]);
    Validator validator(input, output);

    validator.PrintViolations(cout);
    cout << endl;
    validator.PrintCosts(cout);
    cout << endl;
    if (output.Warnings() > 0)
        cout << "There are " << output.Warnings() << " warnings!" << endl;
    cout << "Summary: ";
    validator.PrintTotalCost(cout);
    return 0;
}