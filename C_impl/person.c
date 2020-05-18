#include"person.h"


void expose(Person *person, int iframe)
{
    person->susceptible= false;
    person->exposed = true;
    person->infected = false;
    person->removed = false;
    person->t_exposed = iframe;
    printf("expose: New exposed person\n");
}

void infect(Person *person, int iframe)
{
    person->susceptible = false;
    person->exposed = false;
    person->infected = true;
    person->removed = false;
    person->t_infected = iframe;
    printf("infect: New infectious person\n");

}

void remove_p(Person *person, int iframe)
{
    person->susceptible = false;
    person->exposed = false;
    person->infected = false;
    person->removed = true;
    person->t_removed = iframe;
    printf("remove_p: New declared case\n");
}

void check_contamination(Person *person, int iframe)
{
  if ((person->exposed) && (iframe - person->t_exposed >= person->duration_exposure)) {
    infect(person, iframe);
  }
  if ((person->infected) && (iframe - person->t_infected >= person->duration_infection)) {
    remove_p(person, iframe);
  }
}


void set_objective(Person *person, int obj_x, int obj_y)
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
}

double get_distance(Person *p1, Person *p2)
{
    return sqrt(pow(p1->pos_x - p2->pos_x, 2) + pow(p1->pos_y - p2->pos_y, 2));
}

int random_bounded_num(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower; 
}
