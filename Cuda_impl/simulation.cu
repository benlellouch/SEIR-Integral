// Time is measured in frames

#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>
#include<stdio.h>
#include<curand_kernel.h>

#define CUDA_CORES 384 //number of cuda cores

#define RAND_SEED 1999
#define POP_SIZE 100000
//#define POP_SIZE 2000
#define INIT_PROB_INFECTED 100 // per 10000
#define INIT_PROB_QUARANTINED 0
//#define R_CONTAGION 1.
#define R_CONTAGION 4.
#define P_CONTAGION 0.1  // x100 to get percentage
#define NDAYS_MAX 30
//#define NFRAMES_PER_DAY 100
#define NFRAMES_PER_DAY 100
#define DURATION_EXPOSURE_DAYS 2.9
#define DURATION_INFECTION_DAYS 6.4

#define CITY_WIDTH_METERS 1000


FILE *file;


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

// __global__ void expose(Person *person, int iframe)
// {
//     person->susceptible= false;
//     person->exposed = true;
//     person->infected = false;
//     person->removed = false;
//     person->t_exposed = iframe;
//     //    printf("expose: New exposed person\n");
//     (*num_exposed) ++;
//     (*num_susceptible) --;
// }

void infect(Person *person, int iframe, int *num_infected, int *num_exposed)
{
    person->susceptible = false;
    person->exposed = false;
    person->infected = true;
    person->removed = false;
    person->t_infected = iframe;
    //    printf("infect: New infectious person\n");
    (*num_infected)++;
    (*num_exposed)--;
}

void remove_p(Person *person, int iframe, int *num_removed, int *num_infected)
{
    person->susceptible = false;
    person->exposed = false;
    person->infected = false;
    person->removed = true;
    person->t_removed = iframe;
    //    printf("remove_p: New declared case\n");
    (*num_removed)++;
    (*num_infected)--;
}

void check_contamination(Person *person, int iframe, int *num_removed, int *num_infected, int *num_exposed)
{
  if ((person->exposed) && (iframe - person->t_exposed >= person->duration_exposure)) {
    infect(person, iframe, num_infected, num_exposed);
  }
  if ((person->infected) && (iframe - person->t_infected >= person->duration_infection)) {
    remove_p(person, iframe, num_removed, num_infected);
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

__device__ double get_distance(Person *p1, Person *p2)
{
  return sqrtf(powf(p1->pos_x - p2->pos_x, 2) + pow(p1->pos_y - p2->pos_y, 2));
}

int random_bounded_num(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower; 
}

__global__ void setup_kernel(curandState *state, int* max_i_per_core)
{
    int id = threadIdx.x;
    if(id < POP_SIZE)
        curand_init(RAND_SEED, id, 0, &state[id]);
        if (id == CUDA_CORES - 1)
        {
          max_i_per_core[id] = POP_SIZE - 1;
        }
        else
        {
          max_i_per_core[id] = (id + 1) * (POP_SIZE / CUDA_CORES);
        }
}

void generate_population(Person *population, int *num_exposed, int *num_susceptible)
{
  //#pragma omp parallel for
  int contaminated = 0;
  
  for(int ipop=0; ipop < POP_SIZE; ipop++)
    {
        Person p;
        p.pos_x = (double)random_bounded_num(0,CITY_WIDTH_METERS);
        p.pos_y = (double)random_bounded_num(0,CITY_WIDTH_METERS);

	p.duration_exposure = DURATION_EXPOSURE_DAYS * NFRAMES_PER_DAY;
	p.duration_infection = DURATION_INFECTION_DAYS * NFRAMES_PER_DAY;

        p.susceptible = true;
        p.exposed = false;
        p.infected = false;
        p.removed = false;

	p.t_exposed = -1;
	p.t_infected = -1;p.t_removed = -1;

	p.people_infected = 0;

    if (random_bounded_num(0,10000) < INIT_PROB_INFECTED)
    {
        p.susceptible= false;
        p.exposed = true;
        p.infected = false;
        p.removed = false;
        p.t_exposed = 0;
        (*num_exposed) ++;
        (*num_susceptible) --;
        contaminated++;
    }
	
	if (random_bounded_num(0,100) < INIT_PROB_QUARANTINED) p.quarantined = true;

        population[ipop] = p;
    }  // for ipop
    printf("generate_population: %d people are initially contaminated\n", contaminated);
}

void update_daily_objectives(Person *population)
{
  Person *p;

  for(int ipop=0; ipop < POP_SIZE; ipop++) {
    p = &(population[ipop]);
    if (!(p->removed)) {
	p->obj_x = (double)random_bounded_num(0,CITY_WIDTH_METERS);
	p->obj_y = (double)random_bounded_num(0,CITY_WIDTH_METERS);
	p->speed = sqrt(pow(p->obj_x-p->pos_x,2.)+pow(p->obj_y-p->pos_y,2.))/
	  ((double)NFRAMES_PER_DAY);
	set_objective(p, p->obj_x, p->obj_y);
      } // p->removed
  } // for ipop

}

__global__ void frame_update_helper(Person *person, Person *population, curandState *states, int iframe, int *num_exposed, int *num_susceptible, int* max_i_per_core)
{
    int i = threadIdx.x;
    int lower_bound, upper_bound;
    if(i == 0)
    {
      lower_bound = 0;
      upper_bound = max_i_per_core[i];
    }
    else
    {
      lower_bound = max_i_per_core[i-1];
      upper_bound = max_i_per_core[i];
    }


    if (i < POP_SIZE)
    {
      // printf("Id: %i, lower bound: %i, upper bound: %i\n", i, lower_bound, upper_bound);
      curandState localState = states[i];
      for (int j = lower_bound; j < upper_bound; j++)
      {
        float random_num = (curand_uniform(&localState));
        // printf("randnum : %f\n", random_num);
        if((person != &population[j]) 
        &&  population[j].susceptible 
        && (get_distance(person, &population[j]) < R_CONTAGION) 
        && ( random_num < P_CONTAGION))
        {
            population[j].susceptible = false;
            population[j].exposed = true;
            population[j].infected = false;
            population[j].removed = false;
            population[j].t_exposed = iframe;
            (*num_exposed) ++;
            (*num_susceptible) --;
            person->people_infected ++;
            // printf("I get here");
        }
      }
      states[i] = localState;
    }

}

void frame_update(int iframe,Person* population, curandState *states, int *removed, int *num_susceptible, int *num_exposed, int *num_infected, int* num_removed, int* max_i_per_core)
{
    for (int ipop = 0; ipop < POP_SIZE; ipop++) {
      if (!(population[ipop].removed)) {
	check_contamination(&population[ipop],iframe, num_removed, num_infected, num_exposed);
	update_pos(&population[ipop], 0, 0);
	
	//      printf("frame_update: iframe = %d, ipop = %d, removed = % d, t_removed = %d\n",
	//	     iframe, ipop, population[ipop].removed, population[ipop].t_removed);
	
	if ((population[ipop].removed) && (population[ipop].t_removed == iframe))
	  (*removed)++;
	
	if(population[ipop].infected) {
    frame_update_helper<<<1,CUDA_CORES>>>(&population[ipop], population, states, iframe, num_exposed, num_susceptible, max_i_per_core);
    cudaDeviceSynchronize();
	} // if populuation[ipop]
      } // if populuation[ipop]
    } // for ipop
    //    printf("frame_update: new reported cases = %d\n", *removed);
}


double compute_R(int iday, Person *population)
{
  div_t ratio;
  int ninfected = 0;
  int ninfectors = 0;

  for (int ipop = 0; ipop < POP_SIZE; ipop++) {
    if (population[ipop].removed) {
	ratio = div(population[ipop].t_removed, NFRAMES_PER_DAY);
	if (ratio.quot == iday) {
	  //	  printf("compute_R: ipop = %d was removed on day = %d, having infected %d others\n", ipop, ratio.quot,
	  //		 population[ipop].people_infected); 
	  ninfected += population[ipop].people_infected;
	  ninfectors++;
	} // if ratio-quot
    } // if population[ipop]
  } // for ipop

  double repro_rate = 0.;
  if (ninfectors != 0) repro_rate = ((double)ninfected)/((double)ninfectors);

  return repro_rate;
}


int main()
{
  clock_t begin = clock();
  int *num_susceptible, *num_exposed, *num_infected, *num_removed;
  cudaMallocManaged(&num_susceptible, sizeof(int), cudaMemAttachGlobal);
  cudaMallocManaged(&num_exposed, sizeof(int), cudaMemAttachGlobal);
  cudaMallocManaged(&num_infected, sizeof(int), cudaMemAttachGlobal);
  cudaMallocManaged(&num_removed, sizeof(int), cudaMemAttachGlobal);
  *num_susceptible = POP_SIZE;
  *num_exposed = 0;
  *num_infected = 0;
  *num_removed = 0;

  // printf("susceptible = %i, exposed  = %i, infected = %i, removed = %i\n",
  // *num_exposed, *num_infected, *num_removed);

  file = fopen("output.dat", "w");
  if(file == NULL)
  {
    printf("Error with file\n");
    exit(1);
  }

  srand(RAND_SEED);

  Person *population;
  cudaMallocManaged(&population, POP_SIZE * sizeof(Person), cudaMemAttachGlobal);

  curandState *devStates;
  cudaMalloc((void **)&devStates, CUDA_CORES * sizeof(curandState));

  int *max_i_per_core;
  cudaMalloc ((void **) &max_i_per_core, CUDA_CORES * sizeof(int));

  setup_kernel<<<1, CUDA_CORES>>>(devStates, max_i_per_core);
  
  generate_population(population, num_exposed, num_susceptible);
  int iframe = 0;
  int iday = 0;
  int total_removed = 0;
  
  fprintf(file, "#frame\tday\tnewreported cases\ttotal removed\tR\n");
  printf("#frame\t day\t new reported cases\t total removed\t R\n");
  while((total_removed < POP_SIZE) && (iframe < NDAYS_MAX*NFRAMES_PER_DAY)) {
    int removed = 0;
    update_daily_objectives(population);
    for (int iframeperday=0; iframeperday < NFRAMES_PER_DAY; iframeperday++) {
      frame_update(iframe, population, devStates,  &removed, num_susceptible, num_exposed, num_infected, num_removed, max_i_per_core);
      printf("Time frame %d: susceptible = %d, exposed  = %d, infected = %d, removed = %d, with %d new reported cases\n",
	          iframe, *num_susceptible,
	          *num_exposed, *num_infected, *num_removed, removed);
      iframe++;
    } // for iframeperday
    total_removed += removed;
    iframe--;
    double repro_rate = compute_R(iday, population);
    fprintf(file, "%d\t%d\t%d\t%d\t%.2f\n", iframe, iday, removed, total_removed,
	    repro_rate);
    printf("frame = %d, day = %d, new reported cases = %d, total declared cases = %d, R = %.2f\n", iframe, iday, removed, total_removed,
	   repro_rate);
    iframe++;
    iday++;
  } // while total_removed
  
  fclose(file);
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("\n\nExecution time = %.5f\n", time_spent);

  cudaFree(population);
  cudaFree(devStates);
  cudaFree(num_removed);
  cudaFree(num_exposed);
  cudaFree(num_infected);
  cudaFree(num_susceptible);
  
}
