# gardump: program to dump waypoints, routes, and tracks from a garmin
# gps unit.
#

PROG=	gardump
DPADD+=	${LIBGARMIN}

.include <bsd.prog.mk>

.if exists(../lib/${__objdir})
LDADD+=	-L${.CURDIR}/../lib/${__objdir} -lgarmin
.else
LDADD+=	-L${.CURDIR}/../lib -lgarmin
.endif
