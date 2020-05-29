#include<stdbool.h>
#include<stdlib.h>
#include<math.h>
#include<stdio.h>
#include<cuda.h>
#include<curand_kernel.h>

//#define CITY_WIDTH_METERS 1000
#define CITY_WIDTH_METERS 100

int *num_susceptible, *num_exposed, *num_infected, *num_removed;

typedef struct
{
  double speed;
  
  //current position 
  double pos_x;
  double pos_y;

  // target position
  double obj_x;
  double obj_y;

  //displacement per frame
  double delta_x;
  double delta_y;

  // state
  bool susceptible;
  bool exposed;
  bool infected;
  bool removed;
  bool quarantined;
  
  // frame number of event
  int t_exposed;
  int t_infected;
  int t_removed;

  // duration of exposed and infection period
  int duration_exposure;
  int duration_infection;

  int people_infected;
} Person;


__device__ void expose(Person* person, int iframe);
void infect(Person *person, int iframe);
void remove_p(Person *person, int iframe);
void check_contamination(Person *person, int iframe);
void set_objective(Person *person, const int obj_x, const int obj_y);
void update_pos(Person *person, double n_pos_x, double n_pos_y);
__device__ double get_distance(Person *p1, Person *p2);
int random_bounded_num(int lower, int upper);


