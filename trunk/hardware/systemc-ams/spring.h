/*
 * Single-dimentional model of torsion spring.
 *
 * Copyright (C) 2014 Serge Vakulenko
 */
#include <systemc-ams>

SCA_TDF_MODULE(elastic)
{
    sca_tdf::sca_in<double> displacement_in;    // input ports

    sca_tdf::sca_out<double> force_out;         // output ports

    double elastic_modulus;             // parameters

    SCA_CTOR(elastic) {                 // constructor
        elastic_modulus = 0;
    }

    void set_attributes()               // called at elaboration
    {
        set_timestep(0.001, SC_SEC);    // time between activations
    }

    void processing()                   // executed at each activation
    {
        double displacement = displacement_in.read();
        double force = - displacement * elastic_modulus;

        force_out.write(force);
    }
};
