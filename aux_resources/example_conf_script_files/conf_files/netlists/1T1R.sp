simulator lang=spice

1T1R testbench
.option post ingold=1
.include '65nm_bulk.pm'
.hdl 'rram.va'

.param tr=10ps          $ Voltage pulse rise time
.param tf=10ps          $ Voltage pulse fall time
.param WL_SET=1.0V		$ WL SET voltage amplitude
.param BL_SET=2.0V		$ BL SET voltage amplitude
.param WL_RESET=2.9V  	$ WL RESET voltage amplitude
.param SL_RESET=1.9V	$ SL RESET voltage amplitude
.param SET_PULSE=10ns   $ SET pulse width
.param SET_START=5ns    $ SET operation delay
.param RESET_PULSE=10ns $ RESET pulse width
.param RESET_START='SET_START+5n+SET_PULSE' $ RESET operation delay
.param T_FINAL='RESET_START+RESET_PULSE+5n' $ Final simulation time

X1  BL  Nb  rram    gap_ini=1.367n  tstep=10ps  $ RRAM
M1  Nb  WL  SL  0   nmos    w=130n  l=65n       $ Transistor

V_WL_SET    WL  WL2 PULSE (0V WL_SET   'SET_START-1n'   tr  tf  'SET_PULSE+2n'   T_FINAL)
V_WL_RESET  WL2 0   PULSE (0V WL_RESET 'RESET_START-1n' tr  tf  'RESET_PULSE+2n' T_FINAL)
V_BL_SET    BL  0   PULSE (0V BL_SET   'SET_START'      tr  tf  SET_PULSE        T_FINAL)
V_SL_RESET  SL  0   PULSE (0V SL_RESET 'RESET_START'    tr  tf  RESET_PULSE      T_FINAL)

.tran   40ps T_FINAL
.print  V(WL) V(BL) V(SL) V(BL,Nb)
.print  V(X1.gap_out) V(X1.R_out)
.end
