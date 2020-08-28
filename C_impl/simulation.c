// Time is measured in frames

#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<omp.h>

#include "person.h"

#define RAND_SEED 1999
#define POP_SIZE 10000
//#define POP_SIZE 2000
#define INIT_PROB_INFECTED 100   // per 10000
#define INIT_PROB_QUARANTINED 0
//#define R_CONTAGION 1.
#define R_CONTAGION 4.
#define P_CONTAGION 100 // per 10000
#define NDAYS_MAX 365
//#define NFRAMES_PER_DAY 100
#define NFRAMES_PER_DAY 100
#define DURATION_EXPOSURE_DAYS 2.9
#define DURATION_INFECTION_DAYS 6.4

Person population[POP_SIZE];
FILE *file;

void generate_population()
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
            expose(&p, 0);
            contaminated++;
        }
	
	if (random_bounded_num(0,100) < INIT_PROB_QUARANTINED) p.quarantined = true;

        population[ipop] = p;
    }  // for ipop
    printf("generate_population: %d people are initially contaminated\n", contaminated);
}

void update_daily_objectives()
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

void frame_update(int iframe, int *removed)
{
    for (int ipop = 0; ipop < POP_SIZE; ipop++) {
      if (!(population[ipop].removed)) {
	check_contamination(&population[ipop],iframe);
	update_pos(&population[ipop], 0, 0);
	
	//      printf("frame_update: iframe = %d, ipop = %d, removed = % d, t_removed = %d\n",
	//	     iframe, ipop, population[ipop].removed, population[ipop].t_removed);
	
	if ((population[ipop].removed) && (population[ipop].t_removed == iframe))
	  (*removed)++;
	
	if(population[ipop].infected) {
#pragma omp parallel for
	  for (int jpop = 0; jpop < POP_SIZE; jpop++) {
	    if ((jpop != ipop) &&
		(population[jpop].susceptible) &&
		(get_distance(&population[ipop], &population[jpop]) < R_CONTAGION) &&
		(random_bounded_num(0,10000) < P_CONTAGION)) {
	      expose(&population[jpop], iframe);
	      population[ipop].people_infected ++;
	    } // if population[ipop]
	  } // for jpop
	} // if populuation[ipop]
      } // if populuation[ipop]
    } // for ipop
    //    printf("frame_update: new reported cases = %d\n", *removed);
}

double compute_R(int iday)
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
  num_susceptible = POP_SIZE;
  num_exposed = 0;
  num_infected = 0;
  num_removed = 0;

  file = fopen("output.dat", "w");
  if(file == NULL)
    {
      printf("Error with file\n");
      exit(1);
    }
  //    srand(time(0));
  srand(RAND_SEED);
  generate_population();
  int iframe = 0;
  int iday = 0;
  int total_removed = 0;
  
  fprintf(file, "#frame\tday\tnewreported cases\ttotal removed\tR\n");
  printf("#frame\t day\t new reported cases\t total removed\t R\n");
  while((total_removed < POP_SIZE) && (iframe < NDAYS_MAX*NFRAMES_PER_DAY)) {
    int removed = 0;
    update_daily_objectives();
    for (int iframeperday=0; iframeperday < NFRAMES_PER_DAY; iframeperday++) {
      frame_update(iframe, &removed);
      // printf("Time frame %d: susceptible = %d, exposed  = %d, infected = %d, removed = %d, with %d new reported cases\n",
	    //  iframe, num_susceptible,
	    //  num_exposed, num_infected, num_removed, removed);
      iframe++;
    } // for iframeperday
    total_removed += removed;
    iframe--;
    double repro_rate = compute_R(iday);
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
  
}
