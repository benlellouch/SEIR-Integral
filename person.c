#include<stdbool.h>

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

void remove(Person *person)
{
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
            remove(person);
        }
        else if ((i - person->t_contaminated ) > person->t_incubation)
        {
            infect(person);
        }
        

    }
}
