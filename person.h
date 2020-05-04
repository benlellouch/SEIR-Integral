typedef struct
{
    int speed;

    //target position
    float obj_x;
    float obj_y;

    int index;
    char* name;


    bool infectious;
    bool exposed;
    bool susceptible;
    bool removed;
    bool quarantined;

    //current position 
    float pos_x;
    float pos_y;

    //displacement per frame
    float delta_x;
    float delta_y;

    int t_contaminated;
    int t_recovery;
    int t_incubation;

    int p_infected;
} Person;


void incubate(Person* person);
void infect(Person *person);
void remove(Person *person);
void set_objective(Person *person, const int obj_x, const int obj_y);
void check_contamination(Person *person, int i);
