#include "dispatcher.h"

static int inet_pton4(const char *src, u_char *dst) {
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
#define NS_INADDRSZ	4
	u_char tmp[NS_INADDRSZ], *tp;
	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;
		if ((pch = strchr(digits, ch)) != NULL) {
			u_int new1 = *tp * 10 + (pch - digits);
			if (saw_digit && *tp == 0)
				return (0);
			if (new1 > 255)
				return (0);
			*tp = new1;
			if (!saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		}
		else
			return (0);
	}
	if (octets < 4)
		return (0);
	memcpy(dst, tmp, NS_INADDRSZ);
	return (1);
}

// ---------------------------------------------------------------------------
static int inet_pton6(const char *src, u_char *dst) {
	static const char xdigits_l[] = "0123456789abcdef", xdigits_u[] = "0123456789ABCDEF";
	u_char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, seen_xdigits;
	u_int val;
	memset((tp = tmp), '\0', NS_IN6ADDRSZ);
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	seen_xdigits = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;
		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (++seen_xdigits > 4)
				return (0);
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!seen_xdigits) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			}
			else if (*src == '\0') {
				return (0);
			}
			if (tp + NS_INT16SZ > endp)
				return (0);
			*tp++ = (u_char)(val >> 8) & 0xff;
			*tp++ = (u_char) val & 0xff;
			seen_xdigits = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) && inet_pton4(curtok, tp) > 0) {
			tp += NS_INADDRSZ;
			seen_xdigits = 0;
			break;
		}
		return (0);
	}
	if (seen_xdigits) {
		if (tp + NS_INT16SZ > endp)
			return (0);
		*tp++ = (u_char)(val >> 8) & 0xff;
		*tp++ = (u_char) val & 0xff;
	}
	if (colonp != NULL) {
		const int n = tp - colonp;
		int i;
		if (tp == endp)
			return (0);
		for (i = 1; i <= n; i++) {
			endp[-i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memcpy(dst, tmp, NS_IN6ADDRSZ);
	return (1);
}

// ---------------------------------------------------------------------------
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// ---------------------------------------------------------------------------
void get_in_addr4(u_long *a, struct sockaddr *sa) {
	if (AF_INET == sa->sa_family) {
		*a = ((struct sockaddr_in*)sa)->sin_addr.S_un.S_addr;
	}
	else
		*a = 0;
}

// ---------------------------------------------------------------------------
static const char * inet_ntop4(const unsigned char *src, char *dst, socklen_t size) {
	char tmp[sizeof("255.255.255.255")];
	int len;
	len = sprintf(tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3]);
	if (len < 0)
		return NULL;
	if (len > size) {
		errno = ENOSPC;
		return NULL;
	}
	return strcpy(dst, tmp);
}

// ---------------------------------------------------------------------------
static const char * inet_ntop6(const unsigned char *src, char *dst, socklen_t size) {
	char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")], *tp;
	struct {
		int base, len;
	} best, cur;
	unsigned int words[NS_IN6ADDRSZ / NS_INT16SZ];
	int i;
	memset(words, '\0', sizeof(words));
	for (i = 0; i < NS_IN6ADDRSZ; i += 2)
		words[i / 2] = (src[i] << 8) | src[i + 1];
	best.base = -1;
	cur.base = -1;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (words[i] == 0) {
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		}
		else {
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1) {
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}
	if (best.base != -1 && best.len < 2)
		best.base = -1;
	tp = tmp;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (best.base != -1 && i >= best.base && i < (best.base + best.len)) {
			if (i == best.base)
				* tp++ = ':';
			continue;
		}
		if (i != 0)
			* tp++ = ':';
		if (i == 6 && best.base == 0 && (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
			if (!inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp)))
				return (NULL);
			tp += strlen(tp);
			break;
		} {
			int len = sprintf(tp, "%x", words[i]);
			if (len < 0)
				return NULL;
			tp += len;
		}
	}
	if (best.base != -1 && (best.base + best.len) == (NS_IN6ADDRSZ / NS_INT16SZ))
		* tp++ = ':';
	*tp++ = '\0';
	if ((socklen_t)(tp - tmp) > size) {
		errno = ENOSPC;
		return NULL;
	}
	return strcpy(dst, tmp);
}

// ---------------------------------------------------------------------------
const char * inet_ntop(int af, const void * src, char * dst, socklen_t cnt) {
	switch (af) {
	case AF_INET:
		return (inet_ntop4((const unsigned char *)src, dst, cnt));
	case AF_INET6:
		return (inet_ntop6((const unsigned char *)src, dst, cnt));
	default:
		return (NULL);
	}
}

// ---------------------------------------------------------------------------
uint32_t my_crc32(char *base, size_t len) {
	boost::crc_32_type result;
	result.process_bytes(base, len);
	return result.checksum();
}
// ---------------------------------------------------------------------------
