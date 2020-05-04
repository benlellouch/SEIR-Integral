#include"person.h"


void incubate(Person *person, int i)
{
    person->infectious = false;
    person->exposed = true;
    person->susceptible= false;
    person->removed = false;
    person->t_contaminated = i;
}

void infect(Person *person)
{
    person->infectious = true;
    person->exposed = false;
    person->susceptible = false;
    person->removed = false;
}

void remove_p(Person *person, int i)
{
    person->t_removed = i;
    person->t_contaminated = -1;
    person->removed = true;
    person->susceptible = false;
    person->infectious = false;
}

void set_objective(Person *person, const int obj_x, const int obj_y)
{
    person->obj_x = obj_x;
    person->obj_y = obj_y;
    if(person->quarantined)
    {
        person->delta_x = 0;
        person->delta_y = 0;
    }
    else
    {
        person->delta_x = ( person->obj_x - person->pos_x ) / person->speed;
        person->delta_y = ( person->obj_y - person->pos_y ) / person->speed;
    }
}

void check_contamination(Person *person, int i)
{
    if(person->t_contaminated > -1)
    {
        if ((i - person->t_contaminated - person->t_incubation) > person->t_recovery )
        {
            remove_p(person, i);
        }
        else if ((i - person->t_contaminated ) > person->t_incubation)
        {
            infect(person);
        }
        

    }
}

void update_pos(Person *person, double n_pos_x, double n_pos_y)
{
    if(n_pos_x == 0 && n_pos_y == 0)
    {
        person->pos_x = person->pos_x + person->delta_x;
        person->pos_y = person->pos_y + person->delta_y;
    }
    else
    {
        person->pos_x = n_pos_x;
        person->pos_y = n_pos_y;
    }

    if(abs(person->pos_x - person->obj_x) < 3 && abs(person->pos_y - person->obj_y) < 3 )
    {
        set_objective(person, random_bounded_num(1,100), random_bounded_num(1,100));
    }
    if (person->pos_x > 100) person->pos_x = 100;
    if (person->pos_y > 100) person->pos_y = 100;
    if (person->pos_x < 0) person->pos_x = 0;
    if (person->pos_y < 0) person->pos_y = 0;
}

double get_distance(Person *p1, Person *p2)
{
    return sqrt(pow(p1->pos_x - p2->pos_x, 2) + pow(p1->pos_y - p2->pos_y, 2));
}

int random_bounded_num(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower; 
}
