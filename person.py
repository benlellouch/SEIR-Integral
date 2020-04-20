import math
import numpy as np

class Person:
    def __init__(self,i, posx, posy, objx, objy, v, t_recovery, t_incubation, quarantined):
        # movement speed
        self.v = v
        # target position
        self.objx = objx
        self.objy = objy
        #ID and name
        self.indice = i
        self.name   = "Person "+str(i)
        #State: Susceptible, Infected or Retired
        self.infectious = False
        self.exposed    = False
        self.suceptible = True
        self.removed    = False
        #Current position
        self.posx = posx
        self.posy = posy
        #is it fixed (in quarantine)?
        self.quaritined = quarantined

        # displacement per iteration
        if self.quaritined:
            self.deltax = 0
            self.deltay = 0
        else:
            self.deltax = (self.objx - self.posx) / self.v
            self.deltay = (self.objy - self.posy) / self.v
        #time in which the person was infected
        self.t_contaminated = -1
        #time that the infection lasts, recover time
        self.t_recovery = t_recovery
        #time before the person becomes infection, incubation period
        self.t_incubation = t_incubation


    def __str__(self):
        return self.name+" en posicin "+str(self.posx)+", "+str(self.posy)

    # The person becomes infected but is not yet infectious
    def incubate(self,i):
        self.infectious = False
        self.exposed    = True
        self.suceptible = False
        self.removed    = False
        self.t_contaminated = i

    # The person becomes infectious after the incubation period has passed
    def infect(self):
        self.infectious = True
        self.exposed    = False
        self.suceptible = False
        self.removed    = False

    def remove(self):
        #heal
        self.removed=True
        self.suceptible=False
        self.infectious=False

    def set_objective(self,objx,objy):
        #this function is used to create a new target position
        self.objx=objx
        self.objy=objy
        if self.quaritined:
            self.deltax = 0
            self.deltay=0
        else:
            self.deltax = (self.objx - self.posx) / self.v
            self.deltay = (self.objy - self.posy) / self.v
        print("New objective  ", self.objx,self.objy,"  ",self.indice)

    def check_contamination(self,i):
        #this function is used to heal the person if the established infection time has passed
        if self.t_contaminated>-1:

            if i-self.t_contaminated-self.t_incubation >self.t_recovery:
                self.remove()

            elif i-self.t_contaminated > self.t_incubation:
                self.infect()


    def update_pos(self, n_posx, n_posy):
        #this funcion animates the movement
        if(n_posx==0 and n_posy==0):
            self.posx=self.posx+self.deltax
            self.posy=self.posy+self.deltay
        else:
            self.posx=n_posx
            self.posy=n_posy

        if abs(self.posx-self.objx)<3 and abs(self.posy-self.objy)<3:
            self.set_objective(np.random.random()*100, np.random.random()*100)
        if self.posx>100:
            self.posx=100
        if self.posy>100:
            self.posy=100
        if self.posx<0:
            self.posx=0
        if self.posy<0:
            self.posy=0

    def get_color(self):
        if self.infectious:
            return 'red'
        if self.exposed:
            return 'orange'
        if self.suceptible:
            return 'blue'
        if self.removed:
            return 'gray'

    def get_pos(self):
        return (self.posx,self.posy)

    def get_dist(self,x,y):
        #this funcion calculates the distance between this person an another.
        return math.sqrt((self.posx-x)**2+(self.posy-y)**2)
