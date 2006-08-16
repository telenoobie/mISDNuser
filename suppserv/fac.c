/*
 * fac.c
 *
 * Copyright (C) 2006, Nadi Sarrar
 * Nadi Sarrar <nadi@beronet.com>
 *
 * Portions of this file are based on the mISDN sources
 * by Karsten Keil.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License, Version 2.
 *
 */

#include "suppserv.h"
#include "asn1_diversion.h"
#include "l3dss1.h"
#include <string.h>

enum {
	SUPPLEMENTARY_SERVICE 	= 0x91,
} SERVICE_DISCRIMINATOR;

/*
 * Facility IE Encoding
 */

static __u8* encodeInvokeComponentHead (__u8 *p, __u8 ie_id)
{
	*p++ = ie_id; // IE identifier
	*p++ = 0;     // length -- not known yet
	*p++ = 0x91;  // remote operations protocol
	*p++ = 0xa1;  // invoke component
	*p++ = 0;     // length -- not known yet
	return p;
}

static int encodeInvokeComponentLength (__u8 *msg, __u8 *p)
{
	msg[4] = p - &msg[5];
	msg[1] = p - &msg[2];
	return msg[1] + 2;
}

static int encodeFacCDeflection (__u8 *dest, struct FacCDeflection *CD)
{
	__u8 *p;
	p = encodeInvokeComponentHead(dest, IE_FACILITY);
	p += encodeInt(p, 0x02);
	p += encodeInt(p, 13); // Calldefection
	p += encodeInvokeDeflection(p, CD);
	return encodeInvokeComponentLength(dest, p);
}

int encodeFac (__u8 *dest, struct FacParm *fac)
{
	int len = -1;

	switch (fac->Function) {
	case Fac_None:
	case Fac_GetSupportedServices:
	case Fac_Listen:
	case Fac_Suspend:
	case Fac_Resume:
	case Fac_CFActivate:
	case Fac_CFDeactivate:
	case Fac_CFInterrogateParameters:
	case Fac_CFInterrogateNumbers:
	case Fac_AOCDCurrency:
	case Fac_AOCDChargingUnit:
		break;
	case Fac_CD:
		len = encodeFacCDeflection(dest, &(fac->u.CDeflection));
	}
	return len;
}

/*
 * Facility IE Decoding
 */

int decodeFac (__u8 *src, struct FacParm *fac)
{
	struct asn1_parm pc;
	int 	fac_len,
			offset;
	__u8 	*end,
		 	*p = src;

	if (!p)
		goto _dec_err;

	offset = ParseLen(p, p + 3, &fac_len);
	if (offset < 0)
		goto _dec_err;
	p += offset;
	end = p + fac_len;

	ParseASN1(p + 1, end, 0);

	if (*p++ != SUPPLEMENTARY_SERVICE)
		goto _dec_err;

	if (ParseComponent(&pc, p, end) == -1)
		goto _dec_err;

	switch (pc.comp) {
	case invoke:
		switch (pc.u.inv.operationValue) {
		case Fac_CD:
			fac->Function = Fac_CD;
			if (pc.u.inv.o.reqCD.address.partyNumber.type == 0)
				strncpy((char *)fac->u.CDeflection.DeflectedToNumber,
						pc.u.inv.o.reqCD.address.partyNumber.p.unknown,
						sizeof(fac->u.CDeflection.DeflectedToNumber));
			else
				strncpy((char *)fac->u.CDeflection.DeflectedToNumber,
						pc.u.inv.o.reqCD.address.partyNumber.p.publicPartyNumber.numberDigits,
						sizeof(fac->u.CDeflection.DeflectedToNumber));
			fac->u.CDeflection.PresentationAllowed = pc.u.inv.o.reqCD.pres;
			*(fac->u.CDeflection.DeflectedToSubaddress) = 0;
			return 0;
		case Fac_AOCDCurrency:
			fac->Function = Fac_AOCDCurrency;
			fac->u.AOCDcur.chargeNotAvailable = pc.u.inv.o.AOCDcur.chargeNotAvailable;
			fac->u.AOCDcur.freeOfCharge = pc.u.inv.o.AOCDcur.freeOfCharge;
			strncpy((char *)fac->u.AOCDcur.currency, pc.u.inv.o.AOCDcur.currency, 11);
			fac->u.AOCDcur.currencyAmount = pc.u.inv.o.AOCDcur.currencyAmount;
			fac->u.AOCDcur.multiplier = pc.u.inv.o.AOCDcur.multiplier;
			fac->u.AOCDcur.typeOfChargingInfo = pc.u.inv.o.AOCDcur.typeOfChargingInfo;
			fac->u.AOCDcur.billingId = pc.u.inv.o.AOCDcur.billingId;
			return 0;
		case Fac_AOCDChargingUnit:
			fac->Function = Fac_AOCDChargingUnit;
			fac->u.AOCDchu.chargeNotAvailable = pc.u.inv.o.AOCDchu.chargeNotAvailable;
			fac->u.AOCDchu.freeOfCharge = pc.u.inv.o.AOCDchu.freeOfCharge;
			fac->u.AOCDchu.recordedUnits = pc.u.inv.o.AOCDchu.recordedUnits;
			fac->u.AOCDchu.typeOfChargingInfo = pc.u.inv.o.AOCDchu.typeOfChargingInfo;
			fac->u.AOCDchu.billingId = pc.u.inv.o.AOCDchu.billingId;
			return 0;
		default:
			goto _dec_err;
		}
		break;
	case returnResult:
	case returnError:
	case reject:
		goto _dec_err;
	}

_dec_err:
	fac->Function = Fac_None;
	return -1;
} 

