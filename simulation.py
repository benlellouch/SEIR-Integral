from person import Person
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

#SIMULATION PARAMETERS
n=400  #number of individuals
p_infected   = 1    #percentage of infected people at the beginning of the simulation (0-100%)
r_contagion  = 3    #radius of transmission in pixels (0-100)
p_contagion  = 20    #probability of transmission in percentage (0-100%)
p_quarantine = 0   #percentage of the people in quarantine (0-100%)
t_recovery   = 30  #time taken to recover in number of frames (0-infinity)
t_incubation = 60   #incubation period

contaminated=0
personas=[]

#creating all the individuals in random positions. Infecting some of them
for i in range(n):
    p = Person(i,np.random.random()*100, np.random.random()*100,
                np.random.random() * 100, np.random.random() * 100,
                (np.random.random()+0.5)*100,t_recovery,t_incubation, False)

    if np.random.random()<p_infected/100:
        p.incubate(0)
        contaminated=contaminated+1
    if np.random.random()<p_quarantine/100:
        p.quaritined=True

    personas.append(p)


#this creates all the graphics
fig = plt.figure(figsize=(18,9))
ax = fig.add_subplot(1,2,1)
cx = fig.add_subplot(1,2,2)
ax.axis('off')
cx.axis([0,1000,0,n])
scatt=ax.scatter([p.posx for p in personas],
           [p.posy for p in personas],c='blue',s=8)
caja = plt.Rectangle((0,0),100,100,fill=False)
ax.add_patch(caja)
cvst,=cx.plot(contaminated,color="red",label="Currently infected")
rvst,=cx.plot(contaminated,color="gray",label="Recovered")
r0vst,=cx.plot(contaminated,color="blue",label="R0")
cx.legend(handles=[rvst,cvst,r0vst])
cx.set_xlabel("Time")
cx.set_ylabel("People")


ct=[contaminated]
rt=[0]
t=[0]
r0=[0]



#function excecuted frame by frame
def update(frame,rt,ct,t,r0):
    infected = 0
    removed = 0
    colores = []
    new_removed = []
    sizes = [8 for p in personas]
    for p in personas:
        #check how much time the person has been sick
        p.check_contamination(frame)
        #animate the movement of each person
        p.update_pos(0,0)
        if p.removed:
            removed += 1 #count the amount of recovered
            new_removed.append(p.p_infected)
        if p.infectious:
            infected += 1 #count the amount of infected
            #check for people around the sick individual and infect the ones within the
            # transmission radius given the probability
            for per in personas:
                if per.indice==p.indice or per.infectious or per.removed or per.exposed:
                    pass
                else:
                    d=p.get_dist(per.posx,per.posy)
                    if d<r_contagion:
                        if np.random.random() < p_contagion / 100:
                            per.incubate(frame)
                            p.p_infected += 1
                            sizes[per.indice]=80


        colores.append(p.get_color()) #change dot color according to the person's status
    if len(new_removed) > 0:
        t_r0 = sum(new_removed) / len(new_removed)
    else:
        t_r0 = 0
    

    #update the plotting data
    ct.append(infected)
    rt.append(removed)
    t.append(frame)
    r0.append(t_r0)


    #tramsfer de data to the matplotlib graphics
    offsets=np.array([[p.posx for p in personas],
                     [p.posy for p in personas]])
    scatt.set_offsets(np.ndarray.transpose(offsets))
    scatt.set_color(colores)
    scatt.set_sizes(sizes)
    cvst.set_data(t,ct)
    rvst.set_data(t,rt)
    r0vst.set_data(t,r0)
    return scatt,cvst,rvst,r0vst

#run the animation indefinitely
animation = FuncAnimation(fig, update, interval=25,fargs=(rt,ct,t,r0),blit=True)
plt.show()
