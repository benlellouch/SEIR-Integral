#include<time.h>
#include<stdio.h>

#include "person.h"

#define SIZE 10000
#define P_INFECTED 3
#define R_CONTAGION 3
#define P_CONTAGION 30
#define P_QUARANTINE 0 
#define T_RECOVERY 30
#define T_INCUBATION 60

Person population[SIZE];
int contaminated = 0;
FILE *file;

void generate_population()
{
    for(int i=0; i < SIZE; i++)
    {
        Person p;
        p.index = i;
        p.speed = random_bounded_num(1,100) + 0.5;
        p.pos_x = random_bounded_num(1,100);
        p.pos_y = random_bounded_num(1,100);
        p.obj_x = random_bounded_num(1,100);
        p.obj_y = random_bounded_num(1,100);
        p.t_contaminated = -1;
        p.t_removed = -1;
        p.t_incubation = T_INCUBATION;
        p.t_recovery = T_RECOVERY;
        p.infectious = false;
        p.exposed = false;
        p.susceptible = true;
        p.removed = false;
        p.p_infected = 0;

        if (random_bounded_num(1,100) < P_INFECTED)
        {
            incubate(&p, 0);
            contaminated ++;
        }
        if (random_bounded_num(1,100) < P_QUARANTINE) p.quarantined = true;
        set_objective(&p, p.obj_x, p.obj_y);

        population[i] = p;
    }
}

void update(int frame, int *total_recovered)
{
    int removed = 0;
    int p_infected = 0;
    for (int i = 0; i < SIZE; i++)
    {
        check_contamination(&population[i],frame);

        update_pos(&population[i], 0, 0);

        if(population[i].t_removed == frame)
        {
            removed ++;
            *total_recovered = *total_recovered + 1;
            p_infected += population[i].p_infected;
        }
        if(population[i].infectious)
        {
            for (int j = 0; j < SIZE; j++)
            {
                if((population[i].index == population[j].index) ||
                 population[j].infectious || population[j].exposed || population[j].removed)
                {

                }
                else 
                {
                    double dist = get_distance(&population[i], &population[j]);
                    if (dist < R_CONTAGION)
                    {
                        if(random_bounded_num(1,100) < P_CONTAGION)
                        {
                            incubate(&population[j], frame);
                            population[i].p_infected ++;
                        }
                    }
                }
            }
        }

    }

    float r0 = 0;
    if (removed > 0) r0 = p_infected / removed;

    fprintf(file, "%i,%i,%i,%.2f\n", frame, removed, *total_recovered, r0);
    printf(" with %i new reported cases.\n", removed);

}

int main()
{
    file = fopen("output.csv", "w");
    if(file == NULL)
    {
        printf("Error with file");
        exit(1);
    }
    fprintf(file, "frame,new reported cases,total recovered,r0\n");
    srand(time(0));
    generate_population();
    int i = 0;
    int *total_recovered = malloc(sizeof(int));
    *total_recovered = 0;
    while(i < 1000)
    {
        printf("Time frame: %i completed", i);
        update(i, total_recovered);
        i++;
    }
    fclose(file);
}