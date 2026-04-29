#include <iostream>
#include <cmath>
#include <random>
#include <fstream>

using namespace std;

struct particle {
    double x;
    double y;
    double z;
    double dx;
    double dy;
    double dz;
};

int main() {

    ifstream f("parameters.txt");
    int particle_count, sim_accuracy, particle_spread;
    f>>particle_count;
    f>>sim_accuracy;
    f>>particle_spread;

    random_device rd;
    mt19937_64 gen(rd());


    uniform_real_distribution<double> dist_double_l(-particle_spread, particle_spread); // particle starting position randomness
    uniform_real_distribution<double> dist_double_s(100.0/sim_accuracy, 100.0/sim_accuracy); // starting speed of particles randomness

    particle ps1[particle_count];
    particle ps2[particle_count];
    for (int i=0;i<particle_count;i++) { // make random particles
        ps1[i] = particle(dist_double_l(gen),dist_double_l(gen),dist_double_l(gen),dist_double_s(gen),dist_double_s(gen),dist_double_s(gen));
        ps2[i] = ps1[i];
    }

    double dx,dy,dz,dd,rdx,rdy,rdz;
    for (int _=0;_<1000;_++) {
        for (int i=0;i<particle_count;i++) {
            for (int j=i+1;j<particle_count;j++) {
                rdx=ps1[i].x-ps1[j].x;
                rdy=ps1[i].y-ps1[j].y;
                rdz=ps1[i].z-ps1[j].z;
                dd=sqrt(rdx*rdx+rdy*rdy+rdz*rdz)*sim_accuracy; // distance coefficient
                if (dd!=0){ // if distance is 0 (divide by zero error)
                    dd=1/dd;
                    dx=rdx*dd;dy=rdy*dd;dz=rdz*dd; // change in velocity
                    ps2[i].dx+=dx;ps2[i].dy+=dy;ps2[i].dz+=dz; // apply velocity change to 1
                    ps2[j].dx-=dx;ps2[j].dy-=dy;ps2[j].dz-=dz; // apply velocity change to 2
                }
            }
        }
        for (int i=0;i<particle_count;i++) {
            ps2[i].x-=ps2[i].dx;ps2[i].y-=ps2[i].dy;ps2[i].z-=ps2[i].dz; // change position
            ps1[i]=ps2[i]; // copy
        }
        cout<<_<<"\n";
    }


    return 0;
}
