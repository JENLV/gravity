#include <iostream>
#include <cmath>
#include <random>
#include <fstream>
#include <vector>
#include <thread>
#include <barrier>
#include <algorithm>
#include <SFML/Graphics.hpp>

using namespace std;

struct particle {
    double x, y, z, dx, dy, dz, mass, size;
};

void read_input(long &particle_count, double &sim_accuracy, double &particle_spread, double &particle_speed, long &sim_length, double &glue, long &render_step, vector<particle> &pi) {
    ifstream f("ievade.txt");
    if (!f.is_open())  { throw runtime_error("input file could not be opened"); }
    if (f.eof())       { throw runtime_error("particle count not set"); }
    f >> particle_count;
    if (f.eof())       { throw runtime_error("simulation accuracy not set"); }
    f >> sim_accuracy;
    sim_accuracy=1/sim_accuracy;
    if (f.eof())       { throw runtime_error("particle starting spread not set"); }
    f >> particle_spread;
    if (f.eof())       { throw runtime_error("particle starting speed not set"); }
    f >> particle_speed;
    if (f.eof())       { throw runtime_error("simulation length not set"); }
    f >> sim_length;
    if (f.eof())       { throw runtime_error("glue factor not set"); }
    f >> glue;
    glue=glue/100;
    if (f.eof())       { throw runtime_error("render step not set"); }
    f >> render_step;

    long n = 0;
    double x, y, z, dx, dy, dz, m, s;
    while (!f.eof() && n < particle_count) {
        f >> x >> y >> z >> dx >> dy >> dz >> m >> s;
        if (s != 0) { n++; pi.push_back(particle(x, y, z, dx, dy, dz, m, s)); }
    }
    f.close();
}

void output1(long particle_count, double sim_accuracy, double particle_spread, double particle_speed, long sim_length, double glue, long render_step, vector<particle> &pi) {
    ofstream f("izvade.txt");
    if (!f.is_open())  { throw runtime_error("output file could not be opened"); }
    f<<particle_count<<" "<<1.0/sim_accuracy<<" "<<particle_spread<<" "<<particle_speed<<" "<<sim_length<<" "<<glue*100<<" "<<render_step<<"\n";
    for (particle i : pi) {
        f<<i.x<<" "<<i.y<<" "<<i.z<<" "<<i.dx<<" "<<i.dy<<" "<<i.dz<<" "<<i.mass<<" "<<i.size<<"\n";
    }
    f.close();
}

void output2(long long steps, vector<particle> &pi) {
    ofstream f("izvade.txt", ios::app);
    if (!f.is_open())  { throw runtime_error("output file could not be opened"); }
    f<<steps<<"\n";
    for (particle i : pi) {
        f<<i.x<<" "<<i.y<<" "<<i.z<<" "<<i.dx<<" "<<i.dy<<" "<<i.dz<<" "<<i.mass<<" "<<i.size<<"\n";
    }
    f.close();
}

void grav_calc(vector<particle> &ps1, vector<particle> &ps2, long start, long end, long total_particles, double sim_accuracy, double glue) {
    double dd, dx, dy, dz, si, k;
    for (long i = start; i < end; i++) {
        for (long j = 0; j < total_particles; j++) {
            dx = ps1[j].x - ps1[i].x;
            dy = ps1[j].y - ps1[i].y;
            dz = ps1[j].z - ps1[i].z;
            dd = sqrt(dx*dx + dy*dy + dz*dz);
            if (dd != 0) {
                si = ps1[i].size + ps1[j].size;
                if (dd<=si) {
                    k=ps1[j].size/ps1[i].size*sim_accuracy*glue;
                    ps2[i].dx-=(ps1[i].dx-ps1[j].dx)*k;
                    ps2[i].dy-=(ps1[i].dy-ps1[j].dy)*k;
                    ps2[i].dz-=(ps1[i].dz-ps1[j].dz)*k;
                }
                dd=1/dd;
                k = (1 - si*si*dd*dd)*ps1[j].mass*dd*sim_accuracy;
                ps2[i].dx += dx * k;
                ps2[i].dy += dy * k;
                ps2[i].dz += dz * k;
            }
        }
        ps2[i].x += ps1[i].dx * sim_accuracy;
        ps2[i].y += ps1[i].dy * sim_accuracy;
        ps2[i].z += ps1[i].dz * sim_accuracy;
    }
}

struct projected_particle {
    float sx, sy;
    float sr;
    float z;
};

projected_particle project(const particle &p, float scale, float cx, float cy) {
    double k=1/p.z;
    float sx =  p.x*k * scale + cx;
    float sy = -p.y*k * scale + cy;
    float sr =  p.size*k * scale;
    return { sx, sy, sr, (float)p.z };
}

void render(sf::RenderWindow &window, const vector<particle> &ps, int screen_size, long particle_spread) {
    window.clear(sf::Color::Black);

    float scale = screen_size / 2.0f;
    float center = screen_size / 2.0f;

    vector<projected_particle> projected;
    projected.reserve(ps.size());
    for (const auto &p : ps) {
        if (p.z <= 0) continue;
        projected.push_back(project(p, scale, center, center));
    }

    sort(projected.begin(), projected.end(),
         [](const projected_particle &a, const projected_particle &b) { return a.z > b.z; });

    sf::CircleShape outer_circle, inner_circle;
    int c;
    for (const auto &p : projected) {
        float r_outer = p.sr;
        float r_inner = p.sr * 0.5;
        outer_circle.setRadius(r_outer);
        outer_circle.setOrigin({r_outer, r_outer});
        outer_circle.setPosition({p.sx, p.sy});
        inner_circle.setRadius(r_inner);
        inner_circle.setOrigin({r_inner, r_inner});
        inner_circle.setPosition({p.sx, p.sy});

        c=p.z/particle_spread*150;
        if (c>510){c=510;}
        if (c<256) {
            outer_circle.setFillColor(sf::Color(0,c,255-c,80));
            inner_circle.setFillColor(sf::Color(100,(int)(c*0.5)+100,227-(int)(c*0.5),255));
        }else {
            outer_circle.setFillColor(sf::Color(c-255,510-c,0,80));
            inner_circle.setFillColor(sf::Color((int)(c*0.5)-28,355-(int)(c*0.5),100,255));
        }
        window.draw(outer_circle);
        window.draw(inner_circle);
    }
    window.display();
}

int main() {
    long particle_count, sim_length, render_step;
    double particle_spread, particle_speed, sim_accuracy, glue;
    vector<particle> pi;
    read_input(particle_count, sim_accuracy, particle_spread, particle_speed, sim_length, glue, render_step, pi);

    int thread_count = 7;
    long particle_per_thread = particle_count / thread_count;

    random_device rd;
    mt19937_64 gen(rd());
    uniform_real_distribution<double> dist_double_l(-particle_spread, particle_spread);
    uniform_real_distribution<double> dist_double_s(-particle_speed,  particle_speed);

    vector<particle> ps1(particle_count);
    vector<particle> ps2(particle_count);

    for (long i = 0; i < particle_count; i++) {
        ps1[i] = ((long)pi.size() > i) ? pi[i] : particle(dist_double_l(gen), dist_double_l(gen), dist_double_l(gen)+2*particle_spread, dist_double_s(gen), dist_double_s(gen), dist_double_s(gen), 1, 10);
        ps2[i] = ps1[i];
    }
    output1(particle_count, sim_accuracy, particle_spread, particle_speed, sim_length, glue, render_step, ps1);

    const int SCREEN_SIZE = 900;
    sf::RenderWindow window(sf::VideoMode({SCREEN_SIZE, SCREEN_SIZE}), "particle sim");
    window.setFramerateLimit(60);

    bool running = true;
    barrier start_barrier(thread_count + 1);
    barrier end_barrier(thread_count + 1);

    auto thred = [&](int i) {
        long begin = i * particle_per_thread;
        long end   = (i != thread_count - 1) ? (i + 1) * particle_per_thread : particle_count;
        while (true) {
            start_barrier.arrive_and_wait();
            if (!running) break;
            grav_calc(ps1, ps2, begin, end, particle_count, sim_accuracy, glue);
            end_barrier.arrive_and_wait();
        }
    };

    vector<thread> threads;
    threads.reserve(thread_count);
    for (int i = 0; i < thread_count; i++)
        threads.emplace_back(thred, i);

    long long step_counter=0;
    while (step_counter<sim_length||sim_length==0) {
        step_counter++;
        
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                running = false;
                start_barrier.arrive_and_wait();
                for (auto &t : threads) t.join();
                output2(step_counter, ps1);
                return 0;
            }
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    running = false;
                    start_barrier.arrive_and_wait();
                    for (auto &t : threads) t.join();
                    output2(step_counter, ps1);
                    return 0;
                }
            }
        }

        start_barrier.arrive_and_wait();
        end_barrier.arrive_and_wait();

        for (long i = 0; i < particle_count; i++) {ps1[i] = ps2[i];}
        if (render_step!=0&&step_counter%render_step==0) {
            render(window, ps1, SCREEN_SIZE, (long)particle_spread);
        }
        cout << step_counter << "\n";
    }

    running = false;
    start_barrier.arrive_and_wait();
    for (auto &t : threads) t.join();
    output2(step_counter, ps1);

    return 0;
}
