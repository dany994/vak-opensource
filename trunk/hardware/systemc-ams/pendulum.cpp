/*
 * Simulation of a pendulum.
 *
 * Copyright (C) 2014 Serge Vakulenko
 */
#include <systemc-ams.h>

#include "point-mass.h"
#include "spring.h"

int sc_main (int argc, char* argv[])
{
    sc_set_time_resolution(0.001, SC_SEC);

    sca_tdf::sca_signal<double> force;
    sca_tdf::sca_signal<double> velocity;
    sca_tdf::sca_signal<double> position;

    // Connect the bob
    point_mass bob("bob");
    bob.mass     = 1.0;
    bob.position = 1.0;
    bob.force_in     (force);
    bob.velocity_out (velocity);
    bob.position_out (position);

    // Connect the spring
    elastic spring("spring");
    spring.elastic_modulus = 20.0;
    spring.displacement_in (position);
    spring.force_out       (force);

    // Tracing
    //sca_util::sca_trace_file *tfp = sca_util::sca_create_tabular_trace_file("trace");
    sca_util::sca_trace_file *tfp = sca_util::sca_create_vcd_trace_file("trace");
    sca_util::sca_trace(tfp, force, "force");
    sca_util::sca_trace(tfp, velocity, "velocity");
    sca_util::sca_trace(tfp, position, "position");

    sc_start(10.0, SC_SEC);

    // Terminate simulation
    //sca_util::sca_close_tabular_trace_file(tfp);
    sca_util::sca_close_vcd_trace_file(tfp);
    return 0;
}
