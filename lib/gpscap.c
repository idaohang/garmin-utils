/*
 *	$snafu: gpscap.c,v 1.8 2003/04/11 01:21:49 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gpslib.h"

/*
 * Garmin GPS protocol capability application protocol
 *
 * gps -> host:	protocol array
 *
 * The array contains tag/value pairs.   The only tag/values this program
 * cares about is A{protocol indicator} and D{data type indicator}.  The
 * D{data type indicator} relate to the immediately preceeding A{protocol
 * indicator}.   The protocols A100, A200, A201, A300, and A301 are
 * processed.  All others are ignored.
 */
static void
gps_protocol_parse(gps_handle gps, const unsigned char *data, int datalen)
{
	int ix;
	int tag;
	int val;
	int proto = 0;
	int dix = 0;

	if (data[0] == p_cap) {
		for (ix = 1; ix + 2 < datalen; ix += 3) {
			tag = data[ix];
			val = data[ix + 1] + (data[ix + 2] << 8);
			if (gps_debug(gps) > 2)
				warnx("Capability %c%03d", tag, val);
			switch (tag) {
			case 'A':
				proto = val;
				dix = 0;
				break;
			case 'D':
				switch (proto) {
				default:
					break;
				case 100:
					gps_set_wpt_type(gps, val);
					break;
				case 200:
					if (dix == 0)
						gps_set_rte_hdr_type(gps, val);
					else
						gps_set_rte_wpt_type(gps, val);
					break;
				case 201:
					switch (dix) {
					case 0:
						gps_set_rte_hdr_type(gps, val);
						break;
					case 1:
						gps_set_rte_wpt_type(gps, val);
						break;
					case 2:
						gps_set_rte_lnk_type(gps, val);
						break;
					}
					break;
				case 300:
					if (dix == 0)
						gps_set_trk_type(gps, val);
					break;
				case 301:
					if (dix == 0)
						gps_set_trk_hdr_type(gps, val);
					else
						gps_set_trk_type(gps, val);
					break;
				}
				dix++;
				break;
			default:
				break;
			}
		}
	} else
		if (gps_debug(gps) > 1)
			warnx("unknown packet type %d", data[0]);
}

/*
 * See if the gps unit will send the supported protocol array.
 * Garmin says that some products will send this immediatly
 * following the product data protocol.  We'll wait about
 * 5 seconds for the data.  If it is not found assume it is
 * not supported.
 *
 * procedure returns -1 on error, otherwise 0.
 */

#define RCV_TO	5

int
gps_protocol_cap(gps_handle gps)
{
	int retries = 5;
	unsigned char *data = malloc(GPS_FRAME_MAX);
	int datalen;

	/* Start with a set of default capabilities.   These will be
	   overridden if the device sends up a capability packet. */

	gps_set_wpt_type(gps, D100);
	gps_set_rte_hdr_type(gps, D200);
	gps_set_rte_wpt_type(gps, D100);
	gps_set_trk_type(gps, D300);

	if (! data) {
		if (gps_debug(gps))
			warnx("no memory: protocol capabilities");
		return -1;
	}
	if (gps_debug(gps) > 2)
		warnx("recv: protocol capabilities");
	while (retries--) {
		datalen = GPS_FRAME_MAX;
		switch (gps_recv(gps, RCV_TO, data, &datalen)) {
		case -1:
			gps_send_nak(gps, *data);
			if (gps_debug(gps) > 2)
				warnx("retry: protocol capabilities ");
			break;
		case 0:
			goto done;
		case 1:
			gps_protocol_parse(gps, data, datalen);
			gps_send_ack(gps, *data);
			free(data);
			if (gps_debug(gps) > 2)
				warnx("rcvd: protocol capabilities");
			return 0;
		}
	}
done:
	free(data);
	return -1;
}

