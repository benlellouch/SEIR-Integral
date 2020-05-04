#include<stdbool.h>
#include<stdlib.h>
#include<math.h>
#include<stdio.h>

typedef struct
{
    double speed;

    //target position
    double obj_x;
    double obj_y;

    int index;

    bool infectious;
    bool exposed;
    bool susceptible;
    bool removed;
    bool quarantined;

    //current position 
    double pos_x;
    double pos_y;

    //displacement per frame
    double delta_x;
    double delta_y;

    int t_contaminated;
    int t_removed;
    int t_recovery;
    int t_incubation;

    int p_infected;
} Person;


void incubate(Person* person, int i);
void infect(Person *person);
void remove_p(Person *person, int i);
void set_objective(Person *person, const int obj_x, const int obj_y);
void check_contamination(Person *person, int i);
void update_pos(Person *person, double n_pos_x, double n_pos_y);
double get_distance(Person *p1, Person *p2);
int random_bounded_num(int lower, int upper);



