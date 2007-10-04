#ifndef BCHANNEL_H
#define BCHANNEL_H

enum {
	BC_CSTATE_NULL,
	BC_CSTATE_ICALL,
	BC_CSTATE_OCALL,
	BC_CSTATE_OVERLAP_REC,
	BC_CSTATE_PROCEED,
	BC_CSTATE_ALERTING,
	BC_CSTATE_ACTIV,
	BC_CSTATE_DISCONNECT,
	BC_CSTATE_DISCONNECTED,
	BC_CSTATE_RELEASE,
};

enum {
	BC_BSTATE_NULL,
	BC_BSTATE_SETUP,
	BC_BSTATE_ACTIVATE,
	BC_BSTATE_ACTIV,
	BC_BSTATE_DEACTIVATE,
	BC_BSTATE_CLEANUP,
};

#define BC_SETUP		0x0e0100
#define BC_CLEANUP		0x0e0200

#define ISDN_PID_L2_B_USER      0x420000ff
#define ISDN_PID_L3_B_USER	0x430000ff


extern	int		init_bchannel(bchannel_t *bc, int channel);
extern	int		term_bchannel(bchannel_t *bc);


#endif