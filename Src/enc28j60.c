#include "stm32f7xx_hal.h"
#include "enc28j60.h"
#include "string.h"

extern SPI_HandleTypeDef hspi1;
void (*icmp_callback)(uint8_t *ip);

uint8_t encBank;
uint8_t seqnum = 0xa;
uint16_t gNextPacketPtr;
uint16_t info_data_len = 0;

#define FLASH_IP_SIZE 		4
#define FLASH_MAC_SIZE 		6
extern uint8_t FLASH_IPADDR[FLASH_IP_SIZE];
extern uint8_t FLASH_MACADDR[FLASH_MAC_SIZE];

#define TCP_BUFFER_SIZE 			1500
uint8_t tmp_buffer[TCP_BUFFER_SIZE];

#define enableChip 				GPIOB->BSRR = GPIO_PIN_6 << 16;
#define disableChip 			GPIOB->BSRR = GPIO_PIN_6;


//uint8_t enc28j60SendByte(uint8_t data) {
//	uint8_t value;
//	HAL_SPI_TransmitReceive(&hspi1, &data, &value, 1, 1);
//	return value;
//}

//void enc28j60ReadBuffer (uint16_t len, uint8_t* data) {
//    enableChip;
//    enc28j60SendByte(ENC28J60_READ_BUF_MEM);
//    while (len--) *data++ = enc28j60SendByte(0x00);
//    disableChip;
//}
//void enc28j60WriteBuffer (uint16_t len, uint8_t* data) {
//    enableChip;
//    enc28j60SendByte(ENC28J60_WRITE_BUF_MEM);
//    while (len--) enc28j60SendByte(*data++);
//    disableChip;
//}

uint8_t enc28j60SendByte(uint8_t data) {
	HAL_SPI_TransmitReceive(&hspi1, &data, &tmp_buffer[0], 1, 1);
	return tmp_buffer[0];
}

void enc28j60ReadBuffer (uint16_t len, uint8_t* data) {
	enableChip;
	enc28j60SendByte(ENC28J60_READ_BUF_MEM);
	HAL_SPI_TransmitReceive_DMA(&hspi1, tmp_buffer, data, len);
	while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY){};
	disableChip;
}

void enc28j60WriteBuffer (uint16_t len, uint8_t* data) {
	enableChip;
	enc28j60SendByte(ENC28J60_WRITE_BUF_MEM);
	HAL_SPI_TransmitReceive_DMA(&hspi1, data, tmp_buffer, len);
	while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY){};
	disableChip;
}

uint8_t enc28j60ReadOp (uint8_t op, uint8_t address) {
	uint8_t temp;
	enableChip;
	enc28j60SendByte(op | (address & ADDR_MASK));
	temp = enc28j60SendByte(0xFF);
	if (address & 0x80) temp = enc28j60SendByte(0xFF);
	disableChip;
	return temp;
}

void enc28j60WriteOp (uint8_t op, uint8_t address, uint8_t data) {
	enableChip;
	enc28j60SendByte(op | (address & ADDR_MASK));
	enc28j60SendByte(data);
	disableChip;
}

static uint16_t enc28j60ReadBufferWord (void) {
	uint16_t result;
	enc28j60ReadBuffer(2, (uint8_t*) &result);
	return result;
}

void enc28j60SetBank (uint8_t address) {
	if ((address & BANK_MASK) != encBank) {
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
		encBank = address & BANK_MASK;
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, encBank >> 5);
	}
}

void enc28j60Write (uint8_t address, uint8_t data) {
	enc28j60SetBank(address);
	enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

uint8_t enc28j60Read (uint8_t address) {
	enc28j60SetBank(address);
	return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}

void enc28j60WriteWord(uint8_t address, uint16_t data) {
	enc28j60Write(address, data & 0xff);
	enc28j60Write(address + 1, data >> 8);
}

uint16_t enc28j60PhyReadH (uint8_t address) {
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MICMD, MICMD_MIIRD);
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);
	enc28j60Write(MICMD, 0x00);
	return (enc28j60Read(MIRDH));
}

void enc28j60PhyWrite (uint8_t address, uint16_t data) {
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MIWRL, data);
	enc28j60Write(MIWRH, data >> 8);
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);
}

void enc28j60PacketSend (uint16_t len, uint8_t* packet) {
	while (enc28j60ReadOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS) {
		if( (enc28j60Read(EIR) & EIR_TXERIF) ) {
			enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
			enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
		}
	}
	enc28j60WriteWord(EWRPTL, TXSTART_INIT);
	enc28j60WriteWord(ETXNDL, (TXSTART_INIT + len));
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	enc28j60WriteBuffer(len, packet);
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

uint16_t enc28j60PacketReceive (uint16_t maxlen, uint8_t* packet) {
	uint16_t rxstat, len;
	if(enc28j60Read(EPKTCNT) == 0) return(0);
	enc28j60WriteWord(ERDPTL, gNextPacketPtr);
	gNextPacketPtr = enc28j60ReadBufferWord();
	len = enc28j60ReadBufferWord() - 4;
	rxstat = enc28j60ReadBufferWord();
	if (len > maxlen - 1) len = maxlen - 1;
	if ((rxstat & 0x80) == 0) len = 0; else enc28j60ReadBuffer(len, packet);
	enc28j60WriteWord(ERXRDPTL, gNextPacketPtr );
	if ((gNextPacketPtr - 1 < RXSTART_INIT) || (gNextPacketPtr - 1 > RXSTOP_INIT)) enc28j60WriteWord(ERXRDPTL, RXSTOP_INIT); 
	else enc28j60WriteWord(ERXRDPTL, (gNextPacketPtr - 1));
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return(len);
}

void MX_ENC28J60_Init(void) {
	disableChip;
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	gNextPacketPtr = RXSTART_INIT;
	enc28j60WriteWord(ERXSTL, RXSTART_INIT);
	enc28j60WriteWord(ERXRDPTL, RXSTART_INIT);
	enc28j60WriteWord(ERXNDL, RXSTOP_INIT);
	enc28j60WriteWord(ETXSTL, TXSTART_INIT);
	enc28j60WriteWord(ETXNDL, TXSTOP_INIT);
	enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN | ERXFCON_BCEN);
	enc28j60WriteWord(EPMM0, 0x303f);
	enc28j60WriteWord(EPMCSL, 0xf7f9);
	enc28j60Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
	enc28j60Write(MACON2, 0x00);
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 |MACON3_TXCRCEN | MACON3_FRMLNEN);
	enc28j60WriteWord(MAIPGL, 0x0C12);
	enc28j60Write(MABBIPG, 0x12);
	enc28j60WriteWord(MAMXFLL, MAX_FRAMELEN);
	enc28j60Write(MAADR5, FLASH_MACADDR[0]);
	enc28j60Write(MAADR4, FLASH_MACADDR[1]);
	enc28j60Write(MAADR3, FLASH_MACADDR[2]);
	enc28j60Write(MAADR2, FLASH_MACADDR[3]);
	enc28j60Write(MAADR1, FLASH_MACADDR[4]);
	enc28j60Write(MAADR0, FLASH_MACADDR[5]);
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
	enc28j60SetBank(ECON1);
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	enc28j60Write(ECOCON, 2 & 0x7);
	enc28j60PhyWrite(PHLCON,0x3476);
}

uint16_t checksum (uint8_t *buf, uint16_t len, uint8_t type) {
	uint32_t sum = 0;
	if (type == 2) {
		sum += IP_PROTO_TCP_V;
		sum += len - 8;
	}
	while (len > 1) {
		sum += 0xFFFF & (((uint32_t) * buf << 8) |* (buf + 1));
		buf += 2; len -= 2;
	}
	if (len) sum += ((uint32_t)(0xFF & *buf)) << 8;
	while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
	return ((uint16_t) sum ^ 0xFFFF);
}

uint8_t eth_type_is_arp_and_my_ip (uint8_t *buf, uint16_t len) {
	uint8_t i = 0;
	if (len < 41) return(0);
	if (buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V ||	buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V) return(0);
	while (i < 4) {
		if (buf[ETH_ARP_DST_IP_P+i] != FLASH_IPADDR[i]) return(0);
		i++;
	}
	return(1);
}

uint8_t eth_type_is_ip_and_my_ip (uint8_t *buf, uint16_t len) {
	uint8_t i = 0;
	if (len < 42) return(0);
	if (buf[ETH_TYPE_H_P] != ETHTYPE_IP_H_V || buf[ETH_TYPE_L_P] != ETHTYPE_IP_L_V) return(0);
	if (buf[IP_HEADER_LEN_VER_P] != 0x45) return(0);
	while (i < 4) {
		if (buf[IP_DST_P+i] != FLASH_IPADDR[i]) return(0);
		i++;
	}
	return(1);
}

void make_eth (uint8_t *buf) {
	uint8_t i = 0;
	while (i < 6) {
		buf[ETH_DST_MAC+i] = buf[ETH_SRC_MAC + i];
		buf[ETH_SRC_MAC+i] = FLASH_MACADDR[i]; i++;
	}
}

void fill_ip_hdr_checksum (uint8_t *buf) {
	uint16_t ck;
	buf[IP_CHECKSUM_P] = 0;
	buf[IP_CHECKSUM_P+1] = 0;
	buf[IP_FLAGS_P] = 0x40;
	buf[IP_FLAGS_P+1] = 0;
	buf[IP_TTL_P] = 64;
	ck = checksum(&buf[IP_P], IP_HEADER_LEN, 0);
	buf[IP_CHECKSUM_P] = ck >> 8;
	buf[IP_CHECKSUM_P+1] = ck & 0xff;
}

void make_ip (uint8_t *buf) {
	uint8_t i = 0;
	while (i < 4) {
		buf[IP_DST_P+i] = buf[IP_SRC_P+i];
		buf[IP_SRC_P+i] = FLASH_IPADDR[i]; i++;
	}
	fill_ip_hdr_checksum(buf);
}

void step_seq (uint8_t *buf, uint16_t rel_ack_num, uint8_t cp_seq) {
	uint8_t i = 4;
	uint8_t tseq;
	while (i > 0) {
		rel_ack_num = buf[TCP_SEQ_H_P+i-1] + rel_ack_num;
		tseq = buf[TCP_SEQACK_H_P+i-1];
		buf[TCP_SEQACK_H_P+i-1] = 0xff&rel_ack_num;
		if (cp_seq) {
			buf[TCP_SEQ_H_P+i-1] = tseq;
		} else {
			buf[TCP_SEQ_H_P+i-1] = 0;
		}
		rel_ack_num = rel_ack_num >> 8;	i--;
	}
}

void make_tcphead(uint8_t *buf, uint16_t rel_ack_num, uint8_t cp_seq) {
	uint8_t i;
	i = buf[TCP_DST_PORT_H_P];
	buf[TCP_DST_PORT_H_P] = buf[TCP_SRC_PORT_H_P];
	buf[TCP_SRC_PORT_H_P] = i;
	i = buf[TCP_DST_PORT_L_P];
	buf[TCP_DST_PORT_L_P] = buf[TCP_SRC_PORT_L_P];
	buf[TCP_SRC_PORT_L_P] = i;
	step_seq(buf,rel_ack_num,cp_seq);
	buf[TCP_CHECKSUM_H_P] = 0;
	buf[TCP_CHECKSUM_L_P] = 0;
	buf[TCP_HEADER_LEN_P] = 0x50;
}

void make_arp_answer_from_request (uint8_t *buf) {
	uint8_t i = 0;
	make_eth(buf);
	buf[ETH_ARP_OPCODE_H_P] = ETH_ARP_OPCODE_REPLY_H_V;
	buf[ETH_ARP_OPCODE_L_P] = ETH_ARP_OPCODE_REPLY_L_V;
	while (i < 6) {
		buf[ETH_ARP_DST_MAC_P+i] = buf[ETH_ARP_SRC_MAC_P+i];
		buf[ETH_ARP_SRC_MAC_P+i] = FLASH_MACADDR[i]; i++;
	}
	i = 0;
	while (i < 4) {
		buf[ETH_ARP_DST_IP_P+i] = buf[ETH_ARP_SRC_IP_P+i];
		buf[ETH_ARP_SRC_IP_P+i] = FLASH_IPADDR[i]; i++;
	}
	enc28j60PacketSend(42,buf);
}

void make_echo_reply_from_request (uint8_t *buf, uint16_t len) {
	make_eth(buf); make_ip(buf);
	buf[ICMP_TYPE_P] = ICMP_TYPE_ECHOREPLY_V;
	if (buf[ICMP_CHECKSUM_P] > (0xff-0x08)) buf[ICMP_CHECKSUM_P+1]++;
	buf[ICMP_CHECKSUM_P] += 0x08;
	enc28j60PacketSend(len,buf);
}

void make_tcp_synack_from_syn (uint8_t *buf) {
	uint16_t ck;
	make_eth(buf);
	buf[IP_TOTLEN_H_P] = 0;
	buf[IP_TOTLEN_L_P] = IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + 4;
	make_ip(buf);
	buf[TCP_FLAGS_P] = TCP_FLAGS_SYNACK_V;
	make_tcphead(buf,1,0);
	buf[TCP_SEQ_H_P+0] = 0;
	buf[TCP_SEQ_H_P+1] = 0;
	buf[TCP_SEQ_H_P+2] = seqnum;
	buf[TCP_SEQ_H_P+3] = 0;
	seqnum += 3;
	buf[TCP_OPTIONS_P] = 2;
	buf[TCP_OPTIONS_P+1] = 4;
	buf[TCP_OPTIONS_P+2] = 0x05;
	buf[TCP_OPTIONS_P+3] = 0x0;
	buf[TCP_HEADER_LEN_P] = 0x60;
	buf[TCP_WIN_SIZE] = 0x5;
	buf[TCP_WIN_SIZE+1] = 0x78;
	ck = checksum(&buf[IP_SRC_P], TCP_HEADER_LEN_PLAIN + 12, 2);
	buf[TCP_CHECKSUM_H_P] = ck >> 8;
	buf[TCP_CHECKSUM_L_P] = ck & 0xff;
	enc28j60PacketSend(IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + ETH_HEADER_LEN + 4, buf);
}

uint16_t get_tcp_data_len (uint8_t *buf) {
	int16_t i;
	i =(((int16_t)buf[IP_TOTLEN_H_P]) << 8) | (buf[IP_TOTLEN_L_P] & 0xff);
	i -= IP_HEADER_LEN;
	i -= (buf[TCP_HEADER_LEN_P] >> 4) * 4;
	if (i <= 0) i = 0;
	return ((uint16_t)i);
}

void make_tcp_ack_from_any (uint8_t *buf, int16_t datlentoack, uint8_t addflags) {
	uint16_t j;
	make_eth(buf);
	buf[TCP_FLAGS_P] = TCP_FLAGS_ACK_V | addflags;
	if (addflags == TCP_FLAGS_RST_V) {
		make_tcphead(buf,datlentoack,1);
	} else {
		if (datlentoack == 0) datlentoack = 1;
		make_tcphead(buf,datlentoack,1);
	}
	j = IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN;
	buf[IP_TOTLEN_H_P] = j >> 8;
	buf[IP_TOTLEN_L_P] = j & 0xff;
	make_ip(buf);
	buf[TCP_WIN_SIZE] = 0x4;
	buf[TCP_WIN_SIZE+1] = 0x0;
	j = checksum(&buf[IP_SRC_P], TCP_HEADER_LEN_PLAIN + 8, 2);
	buf[TCP_CHECKSUM_H_P] = j >> 8;
	buf[TCP_CHECKSUM_L_P] = j & 0xff;
	enc28j60PacketSend(IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + ETH_HEADER_LEN, buf);
}

void make_tcp_ack_with_data (uint8_t *buf, uint16_t dlen) {
	uint16_t j;
	j = IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + dlen;
	buf[IP_TOTLEN_H_P] = j >> 8;
	buf[IP_TOTLEN_L_P] = j & 0xff;
	fill_ip_hdr_checksum(buf);
	buf[TCP_CHECKSUM_H_P] = 0;
	buf[TCP_CHECKSUM_L_P] = 0;
	j = checksum(&buf[IP_SRC_P], TCP_HEADER_LEN_PLAIN + dlen + 8, 2);
	buf[TCP_CHECKSUM_H_P] = j >> 8;
	buf[TCP_CHECKSUM_L_P] = j & 0xff;
	enc28j60PacketSend(IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + ETH_HEADER_LEN + dlen,buf);
}

uint16_t fill_data_len (uint8_t *buf, uint16_t pos, const char *s, uint16_t len) {
	while (len) {
		buf[TCP_CHECKSUM_L_P + pos + 3] = *s;
		pos++; s++; len--;
	}
	return(pos);
}

uint16_t tcp_fill_data (uint8_t *buf, const uint8_t *s, uint16_t len) {
	return(fill_data_len(buf, 0,(char*)s, len));
}

uint16_t www_fill_data (uint8_t *buf,uint16_t pos, const char *s) {
	return(fill_data_len(buf, pos,(char*)s, strlen(s)));
}

void tcp_server_reply(uint8_t *buf, uint16_t dlen) {
	make_tcp_ack_from_any(buf,info_data_len,0);
	buf[TCP_FLAGS_P] = TCP_FLAGS_ACK_V | TCP_FLAGS_PUSH_V;
	make_tcp_ack_with_data(buf,dlen);
}

uint16_t packetloop_icmp_tcp(uint8_t *buf, uint16_t plen) {
	if(eth_type_is_arp_and_my_ip(buf,plen)) {
		if (buf[ETH_ARP_OPCODE_L_P] == ETH_ARP_OPCODE_REQ_L_V) make_arp_answer_from_request(buf);
		return(0);
	}
	if(eth_type_is_ip_and_my_ip(buf,plen) == 0) return(0);
	if(buf[IP_PROTO_P] == IP_PROTO_ICMP_V && buf[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V) {
		if (icmp_callback) (*icmp_callback)(&(buf[IP_SRC_P]));
		make_echo_reply_from_request(buf,plen);
		return(0);
    }
	if (plen < 54 && buf[IP_PROTO_P] != IP_PROTO_TCP_V ) return(0);

	// TCP Server	Port 9670 (0x25C6)
	if (buf[TCP_DST_PORT_H_P] == 0x25 && buf[TCP_DST_PORT_L_P] == 0xC6) {
		if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V) {
			make_tcp_synack_from_syn(buf);
			return(0);
		}
		if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V) {
			info_data_len = get_tcp_data_len(buf);
			if (info_data_len == 0) {
				if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V) make_tcp_ack_from_any(buf,0,0);
				return(0);
			}
			return(info_data_len);
		}
		return(0);
	}
	return(0);
}
