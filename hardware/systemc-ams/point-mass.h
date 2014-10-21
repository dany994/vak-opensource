/*
 * Single-dimentional model of a point mass.
 *
 * Copyright (C) 2014 Serge Vakulenko
 */
#include <systemc-ams>

SCA_TDF_MODULE(point_mass)
{
    sca_tdf::sca_in<double> force_in;           // input ports

    sca_tdf::sca_out<double> velocity_out;      // output ports
    sca_tdf::sca_out<double> position_out;

    double mass;                        // parameters
    double position;
    double velocity;
    double time_last;

    SCA_CTOR(point_mass) {              // constructor
        mass = 0;
        position = 0;
        velocity = 0;
        time_last = 0;
    }

    void set_attributes()               // called at elaboration
    {
        set_timestep(0.001, SC_SEC);    // time between activations
        position_out.set_delay(1);      // delay for output signal
    }

    void processing()                   // executed at each activation
    {
        double force = force_in.read();
        double time_now = get_time().to_seconds();
        double delta_t = time_now - time_last;

        if (delta_t > 0) {
            velocity += delta_t * force / mass;
            position += velocity * delta_t;
//printf("(%g) force=%g, position=%g, velocity=%g \n", time_now, force, position, velocity);

            velocity_out.write(velocity);
            position_out.write(position);
            time_last = time_now;
        }
    }
};
