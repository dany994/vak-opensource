#include <systemc.h>
using namespace std;

#define PRINT_WHILE_RUN

static const int MEM_SIZE = 512;

SC_MODULE(Memory) {

public:
    enum Function {
        FUNC_READ,
        FUNC_WRITE
    };

    enum RetCode {
        RET_READ_DONE,
        RET_WRITE_DONE,
    };

    sc_in<bool>     Port_CLK;
    sc_in<Function> Port_Func;
    sc_in<int>      Port_Addr;
    sc_out<RetCode> Port_Done;
    sc_inout_rv<32> Port_Data;

    SC_CTOR(Memory) {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos();
        dont_initialize();

        m_data = new int[MEM_SIZE];
    }

    ~Memory() {
        delete[] m_data;
    }

private:
    int* m_data;

    void execute() {
        for (;;) {
            wait(Port_Func.value_changed_event());

            Function f = Port_Func.read();
            int addr   = Port_Addr.read();
            int data   = 0;
            if (f == FUNC_WRITE) {
                data = Port_Data.read().to_int();
            }

            wait(100);

            if (f == FUNC_READ) {
                Port_Data.write( (addr < MEM_SIZE) ? m_data[addr] : 0 );
                Port_Done.write( RET_READ_DONE );
#if defined(PRINT_WHILE_RUN)
                cout << "Memory : Finished read request. Addr = " << addr
                     << "  Time = " << sc_time_stamp() << endl;
#endif
                wait();
                Port_Data.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
            } else {
                if (addr < MEM_SIZE) {
                    m_data[addr] = data;
                }
                Port_Done.write( RET_WRITE_DONE );
#if defined(PRINT_WHILE_RUN)
                cout << "Memory : Finished write request. Addr = " << addr
                     << "  Time = " << sc_time_stamp() << endl;
#endif
            }
        }
    }
};

SC_MODULE(CPU) {

public:
    sc_in<bool>              Port_CLK;
    sc_in<Memory::RetCode>   Port_MemDone;
    sc_out<Memory::Function> Port_MemFunc;
    sc_out<int>              Port_MemAddr;
    sc_inout_rv<32>          Port_MemData;

    SC_CTOR(CPU) {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos();
        dont_initialize();
    }

private:
    void execute() {
        for (;;) {
            wait();
            Memory::Function f = (rand() % 10) < 5 ? Memory::FUNC_READ : Memory::FUNC_WRITE;
            int addr           = (rand() % MEM_SIZE);;

            Port_MemAddr.write(addr);
            Port_MemFunc.write(f);

            if (f == Memory::FUNC_WRITE) {
                Port_MemData.write( rand() );
#if defined(PRINT_WHILE_RUN)
                cout << "\nCPU : Sent write request. Addr = " <<  addr
                     << "  Time = " << sc_time_stamp() << endl;
#endif
                wait();
                Port_MemData.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
            } else {
#if defined(PRINT_WHILE_RUN)
                cout << "\nCPU : Sent read request. Addr = " <<  addr
                     << "  Time = " << sc_time_stamp() << endl;
#endif
            }

            wait(Port_MemDone.value_changed_event());

#if defined(PRINT_WHILE_RUN)
            cout << "CPU : Received FIN_SIG from memory."
                 << "  Time = " << sc_time_stamp() << endl;
#endif
        }
    }
};

int sc_main(int argc, char* argv[])
{
    try {
        // Instantiate modules
        Memory mem("main_memory");
        CPU    cpu("cpu");

        // Signals
        sc_buffer<Memory::Function> sigMemFunc;
        sc_buffer<Memory::RetCode>  sigMemDone;
        sc_signal<int>              sigMemAddr;
        sc_signal_rv<32>            sigMemData;

        // The clock that will drive the CPU and memory
        sc_clock clk;

        // Connecting module ports with signals
        mem.Port_Func(sigMemFunc);
        mem.Port_Addr(sigMemAddr);
        mem.Port_Data(sigMemData);
        mem.Port_Done(sigMemDone);

        cpu.Port_MemFunc(sigMemFunc);
        cpu.Port_MemAddr(sigMemAddr);
        cpu.Port_MemData(sigMemData);
        cpu.Port_MemDone(sigMemDone);

        mem.Port_CLK(clk);
        cpu.Port_CLK(clk);

        cout << "Running (press CTRL+C to exit)... " << endl;

        // Start simulation
        sc_start();
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}
