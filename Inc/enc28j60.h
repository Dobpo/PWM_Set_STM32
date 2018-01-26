#ifndef __ENC28J60_H
#define __ENC28J60_H

// ******* ETH *******
#define ETH_HEADER_LEN		14
#define ETHTYPE_ARP_H_V 	0x08
#define ETHTYPE_IP_H_V  	0x08
#define ETHTYPE_IP_L_V  	0x00
#define ETH_TYPE_H_P 		12
#define ETH_TYPE_L_P 		13
#define ETH_DST_MAC 		0
#define ETH_SRC_MAC 		6

// ******* ARP *******
#define ETH_ARP_OPCODE_REPLY_H_V 	0x0
#define ETH_ARP_OPCODE_REPLY_L_V 	0x02
#define ETH_ARP_OPCODE_REQ_L_V 		0x01
#define ETHTYPE_ARP_L_V 			0x06
#define ETH_ARP_OPCODE_H_P 			0x14
#define ETH_ARP_OPCODE_L_P 			0x15
#define ETH_ARP_SRC_MAC_P 			0x16
#define ETH_ARP_SRC_IP_P 			0x1c
#define ETH_ARP_DST_MAC_P 			0x20
#define ETH_ARP_DST_IP_P 			0x26

// ******* IP *******
#define IP_HEADER_LEN		20
#define IP_PROTO_ICMP_V		0x01
#define IP_PROTO_TCP_V		0x06
#define IP_P				0x0E
#define IP_TOTLEN_H_P		0x10
#define IP_TOTLEN_L_P		0x11
#define IP_FLAGS_P 			0x14
#define IP_TTL_P			0x16
#define IP_PROTO_P			0x17
#define IP_CHECKSUM_P 		0x18
#define IP_SRC_P 			0x1a
#define IP_DST_P 			0x1e
#define IP_HEADER_LEN_VER_P 0xe

// ******* ICMP *******
#define ICMP_TYPE_ECHOREPLY_V 	0
#define ICMP_TYPE_ECHOREQUEST_V 8
#define ICMP_TYPE_P 			0x22
#define ICMP_CHECKSUM_P 		0x24

// ******* TCP *******
#define TCP_FLAGS_FIN_V		0x01
#define TCP_FLAGS_SYN_V		0x02
#define TCP_FLAGS_RST_V     0x04
#define TCP_FLAGS_PUSH_V    0x08
#define TCP_FLAGS_ACK_V		0x10
#define TCP_FLAGS_SYNACK_V 	0x12
#define TCP_SRC_PORT_H_P 	0x22
#define TCP_SRC_PORT_L_P 	0x23
#define TCP_DST_PORT_H_P 	0x24
#define TCP_DST_PORT_L_P 	0x25
#define TCP_SEQ_H_P      	0x26
#define TCP_SEQACK_H_P   	0x2a
#define TCP_FLAGS_P 		0x2f
#define TCP_CHECKSUM_H_P 	0x32
#define TCP_CHECKSUM_L_P 	0x33
#define TCP_OPTIONS_P 		0x36
#define TCP_HEADER_LEN_P 	0x2e
#define TCP_WIN_SIZE 		0x30
#define TCP_HEADER_LEN_PLAIN 20

#define ERDPTL           (0x00|0x00)
#define EWRPTL           (0x02|0x00)
#define ETXSTL           (0x04|0x00)
#define ETXNDL           (0x06|0x00)
#define ERXSTL           (0x08|0x00)
#define ERXNDL           (0x0A|0x00)
#define ERXRDPTL         (0x0C|0x00)
#define EPMM0            (0x08|0x20)
#define EPMCSL           (0x10|0x20)
#define ERXFCON          (0x18|0x20)
#define EPKTCNT          (0x19|0x20)
#define EREVID           (0x12|0x60)
#define ECOCON           (0x15|0x60)
#define MACON1           (0x00|0x40|0x80)
#define MACON2           (0x01|0x40|0x80)
#define MACON3           (0x02|0x40|0x80)
#define MABBIPG          (0x04|0x40|0x80)
#define MAIPGL           (0x06|0x40|0x80)
#define MAMXFLL          (0x0A|0x40|0x80)
#define MICMD            (0x12|0x40|0x80)
#define MIREGADR         (0x14|0x40|0x80)
#define MIWRL            (0x16|0x40|0x80)
#define MIWRH            (0x17|0x40|0x80)
#define MIRDH            (0x19|0x40|0x80)
#define MAADR0           (0x01|0x60|0x80)
#define MAADR1           (0x00|0x60|0x80)
#define MAADR2           (0x03|0x60|0x80)
#define MAADR3           (0x02|0x60|0x80)
#define MAADR4           (0x05|0x60|0x80)
#define MAADR5           (0x04|0x60|0x80)
#define MISTAT           (0x0A|0x60|0x80)

#define EIE              0x1B
#define EIR              0x1C
#define ECON2            0x1E
#define ECON1            0x1F
#define ADDR_MASK        0x1F
#define BANK_MASK        0x60
#define PHCON2           0x10
#define PHLCON           0x14
#define ERXFCON_UCEN     0x80
#define ERXFCON_CRCEN    0x20
#define ERXFCON_PMEN     0x10
#define ERXFCON_BCEN     0x01
#define EIE_INTIE        0x80
#define EIE_PKTIE        0x40
#define EIR_TXERIF       0x02
#define ECON2_PKTDEC     0x40
#define ECON1_TXRST      0x80
#define ECON1_TXRTS      0x08
#define ECON1_RXEN       0x04
#define ECON1_BSEL1      0x02
#define ECON1_BSEL0      0x01
#define MACON1_TXPAUS    0x08
#define MACON1_RXPAUS    0x04
#define MACON1_MARXEN    0x01
#define MACON3_PADCFG0   0x20
#define MACON3_TXCRCEN   0x10
#define MACON3_FRMLNEN   0x02
#define MICMD_MIIRD      0x01
#define MISTAT_BUSY      0x01
#define PHCON2_HDLDIS    0x0100

#define ENC28J60_READ_CTRL_REG       0x00
#define ENC28J60_READ_BUF_MEM        0x3A
#define ENC28J60_WRITE_CTRL_REG      0x40
#define ENC28J60_WRITE_BUF_MEM       0x7A
#define ENC28J60_BIT_FIELD_SET       0x80
#define ENC28J60_BIT_FIELD_CLR       0xA0
#define ENC28J60_SOFT_RESET          0xFF

#define RXSTART_INIT	0x0
#define MAX_FRAMELEN	1500
#define TXSTOP_INIT		0x1FFF
#define TXSTART_INIT	(0x1FFF-0x0600)
#define RXSTOP_INIT		(0x1FFF-0x0600-1)

#endif
