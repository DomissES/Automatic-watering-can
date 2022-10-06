/*
 * defines.h
 *
 * Created: 17.08.2022 18:27:44
 *  Author: domis
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_

#define GLUE(a,b)	a##b

#define SET_(what, p, m) GLUE(what, p) |= (1<<(m))
#define CLR_(what, p, m) GLUE(what, p) &= ~(1<<(m))
#define TOGGLE_(what, p, m) GLUE(what, p) ^=(1<<(m))
#define GET_(/*PIN*/ p, m) GLUE(PIN, p) & (1<<(m))
#define SET(what, x) SET_(what, x)
#define CLR(what, x) CLR_(what, x)
#define TOGGLE(what, x) TOGGLE_(what, x)
#define GET(/*PIN*/ x) GET_(x)

/*
* Dla pliku hd44780_hw.c
*/
#define BACKLIGHT	C, 7
#define HD44780_RS	C, 4
#define HD44780_RW	C, 5
#define HD44780_E	C, 6
#define HD44780_D4	C, 0

#define USE_BUSY_BIT 1

/*
* Definicje wszystkich pinow GPIO, pod komentarzem nieuzywane/stosowane w przyszlosci
*/

//#define RXD		E, 0
//#define TXD		E, 1
#define OUT_POFF	E, 2
//#define GL_WDSA		E, 3
//#define GL_HTA		E, 4
//#define GL_LTA		E, 5
#define INB_STA		E, 6
#define INB_STB		E, 7

//#define SCL		D, 0
//#define SDA		D, 1
#define INB_ENCB	D, 2
//#define DHT_THMA	D, 3
#define DHT_THMI	D, 4
//#define DHT_THMB	D, 5
#define DHT_THMM	D, 6
//#define GL_FNB		D, 7

//#define GL_LUVA		B, 0
//#define GL_PMPA		B, 1
//#define GL_FNA		B, 2
//#define GL_LUVB		B, 3
#define GL_PMPB		B, 4
//#define GL_WDSB		B, 5
//#define GL_HTB		B, 6
//#define GL_LTB		B, 7

#define LED_WRKB	A, 0
#define LED_WRKA	A, 1
#define LED_ERR		A, 2
#define LED_RDM		A, 3
#define OUT_BUZZ	A, 4
#define INB_ENC_A	A, 5
#define INB_ENC_B	A, 6
//#define GL_GLOB		A, 7

/*
#define BCU		F, 0
#define BVL		F, 1
#define SVL		F, 2
#define HGB		F, 3
#define HGA		F, 4
#define LPHB		F, 5
#define LPHM		F, 6
#define LPHA		F, 7
*/

//#define GL_CNNB		G, 0
//#define GL_GLOA		G, 1
//#define GL_CNNA		G, 2



#endif /* DEFINES_H_ */