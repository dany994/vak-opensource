VSRC	= top.sv Utopia.sv cpu_ifc.sv
ISRC	= LookupTable.sv squat.sv utopia1_atm_rx.sv utopia1_atm_tx.sv test.sv
SSRC	= definitions.sv atm_cell.sv environment.sv config.sv generator.sv \
	  driver.sv monitor.sv scoreboard.sv coverage.sv cpu_driver.sv
VOPTS	= -sverilog -debug_all +libext+.sv -y . -cm line+tgl -lca
SOPT	= +ntb_solver_mode=1 -l simv.log -cm line+tgl

cov:	simv.vdb
	urg -dir simv.vdb
	urg -dir simv.vdb -format text

dve:	simv.vdb
	dve -lca -cov -covdir simv.cm

run simv.vdb:	simv
	@rm -rf simv.vdb
#	./simv ${SOPT} +ntb_random_seed=19
	./simv ${SOPT} +ntb_random_seed_automatic

simv:   ${VSRC} ${SSRC} ${ISRC}
	vcs ${VOPTS} -l comp.log ${VSRC}

clean:
	@rm -rf *~ *.log *.vpd vc_hdrs.h .vcsmx_rebuild DVEfiles .nfs* .restartSimSession*
	@rm -rf simv* csrc* core* *.vrh *shell.v .__solver* *.key .ucli* .nfs* *.tcl urgReport
