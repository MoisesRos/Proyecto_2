/*
;******************************************************************************
; Universidad del Valle de Guatemala
; Programación de Microcrontroladores
; Proyecto: Proyecto 2
; Archivo: 
; Hardware: ATMEGA328p
; Created: 30/01/2024 18:11:46
; Author : Moises Rosales
;******************************************************************************
 */ 
// DEFINICIÓN DE LA FRECUENCIA DE OPERACIÓN DEL MICROCONTROLADOR
#define F_CPU 16000000

// INCLUSIÓN DE BIBLIOTECAS NECESARIAS PARA LA CONFIGURACIÓN Y OPERACIÓN DEL ATMEGA328P
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include "PWM1/PWM1.h"
#include "PWM2/PWM2.h"
#include "ADC/ADC.h"

// VARIABLES VOLÁTILES PARA CONTROL DE ADC, PWM Y ESTADOS
volatile uint8_t valorADC = 0;
volatile uint8_t PuertoADC = 3;
volatile uint16_t top = 0;
volatile uint16_t valor = 0;

volatile char opcion;
volatile char anterior = '\0';
volatile char buffer[10];
volatile uint8_t buffer_index = 0;
volatile uint8_t data_received = 0;

volatile uint16_t valor_reescalado1 = 0;
volatile uint16_t valor_reescalado2 = 0;
volatile uint16_t valor_reescalado3 = 0;
volatile uint16_t valor_reescalado4 = 0;

volatile uint8_t estado = 0;
volatile uint8_t eprom_pos = 1;

#define POSE_COUNT 4
#define SERVO_COUNT 4

// DEFINICIÓN DE LA MEMORIA EEPROM PARA ALMACENAR LAS POSICIONES DE LOS SERVOS
uint16_t EEMEM ee_poses[POSE_COUNT][SERVO_COUNT] = {0};

#define BUTTON1_PIN   PC2
#define BUTTON2_PIN   PC1
#define BUTTON3_PIN   PC0

// PROTOTIPOS DE FUNCIONES PARA UART Y OPERACIONES
void initUART9600(void);
void writeUART(char caracter);
void writeTextUART(char* texto);

// FUNCIÓN PARA CONFIGURAR PINES
void init_pines(void) {
	// CONFIGURAR PINES PC0, PC1, PC2 Y PC3 COMO ENTRADAS Y HABILITAR RESISTENCIAS DE PULL-UP
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1) & ~(1 << DDC2) & ~(1 << DDC3) & ~(1 << DDC4) & ~(1 << DDC5);
	PORTC = 0x1F;

	// CONFIGURAR PINES DE SALIDA PARA PORTD Y PORTB
	DDRD = 0xFF;
	DDRB = 0x3F;

	// HABILITAR INTERRUPCIONES POR CAMBIO DE ESTADO EN PC0, PC1, PC2 Y PC3
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11);
}

// FUNCIÓN PARA REESCALAR VALORES DE ADC A UN RANGO DESEADO
uint16_t reescalar(uint8_t valor, uint8_t max_origen, uint16_t max_destino) {
	uint16_t valor_reescalado = (float)valor / max_origen * max_destino;
	return valor_reescalado;
}

int main() {
	// CONFIGURAR EL RELOJ DEL SISTEMA
	CLKPR = CLKPCE;
	CLKPR = CLKPS0;

	// INICIALIZAR PINES, UART Y ADC
	init_pines();
	initUART9600();
	init_ADC(0, 0, 128);

	// CONFIGURACIÓN DEL PWM
	top = 155;
	int preescaler = 1024;

	init_PWM1A(0, 6, preescaler, top);
	init_PWM1B(0);
	init_PWM2A(0, 3, preescaler);
	init_PWM2B(0);

	// HABILITAR INTERRUPCIONES GLOBALES
	sei();

	while (1) {
		// CONTROLAR ESTADOS Y LEER ADC
		if (estado == 0 || estado == 1) {
			if (PuertoADC == 3) {
				valor = readADC(4);
				valor_reescalado1 = reescalar(valor, 255, 42);
				duty_cycle2A(valor_reescalado1);
				PuertoADC++;
				} else if (PuertoADC == 4) {
				valor = readADC(5);
				valor_reescalado2 = reescalar(valor, 255, 42);
				duty_cycle1B(valor_reescalado2);
				PuertoADC++;
				} else if (PuertoADC == 5) {
				valor = readADC(6);
				valor_reescalado3 = reescalar(valor, 255, 42);
				duty_cycle1A(valor_reescalado3);
				PuertoADC++;
				} else if (PuertoADC == 6) {
				valor = readADC(7);
				valor_reescalado4 = reescalar(valor, 255, 42);
				duty_cycle2B(valor_reescalado4);
				PuertoADC++;
				} else {
				PuertoADC = 3;
			}
		}
	}
	return 0;
}

// INICIALIZACIÓN DE UART A 9600 BAUDIOS
void initUART9600(void) {
	DDRD &= ~(1 << DDD0);  // RXD (PD0) COMO ENTRADA
	DDRD |= (1 << DDD1);   // TXD (PD1) COMO SALIDA

	UCSR0A = 0;
	UCSR0A |= (1 << U2X0);  // DOBLE VELOCIDAD

	UCSR0B = 0;
	UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);  // HABILITAR RX, TX E INTERRUPCIÓN RX

	UCSR0C = 0;
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);  // MODO 8 BITS

	UBRR0 = 207;  // CONFIGURAR BAUDIOS A 9600
}

// FUNCIÓN PARA ESCRIBIR UN CARACTER POR UART
void writeUART(char caracter) {
	while (!(UCSR0A & (1 << UDRE0)));  // ESPERAR HASTA QUE EL BUFFER ESTÉ VACÍO
	UDR0 = caracter;  // ENVIAR CARACTER
}

// FUNCIÓN PARA ESCRIBIR UNA CADENA DE TEXTO POR UART
void writeTextUART(char* texto) {
	uint8_t i;
	for (i = 0; texto[i] != '\0'; i++) {
		while (!(UCSR0A & (1 << UDRE0)));  // ESPERAR HASTA QUE EL BUFFER ESTÉ VACÍO
		UDR0 = texto[i];  // ENVIAR CARACTER
	}
}

// FUNCIÓN PARA CONTROLAR LOS LEDS SEGÚN EL ESTADO
void leds() {
	if (estado != 0 && estado != 3) {
		if (eprom_pos == 1) {
			PORTB |= (1 << PINB0);
			PORTD &= ~(1 << PIND7) & ~(1 << PIND6) & ~(1 << PIND5);
			writeTextUART("Pose 1\n");
			} else if (eprom_pos == 2) {
			PORTD |= (1 << PIND7);
			PORTD &= ~(1 << PIND6) & ~(1 << PIND5);
			PORTB &= ~(1 << PINB0);
			writeTextUART("Pose 2\n");
			} else if (eprom_pos == 3) {
			PORTD |= (1 << PIND6);
			PORTD &= ~(1 << PIND7) & ~(1 << PIND5);
			PORTB &= ~(1 << PINB0);
			writeTextUART("Pose 3\n");
			} else {
			PORTD |= (1 << PIND5);
			PORTD &= ~(1 << PIND7) & ~(1 << PIND6);
			PORTB &= ~(1 << PINB0);
			writeTextUART("Pose 4\n");
		}
		} else {
		PORTD &= ~(1 << PIND7) & ~(1 << PIND6) & ~(1 << PIND5);
		PORTB &= ~(1 << PINB0);
	}
}

// FUNCIÓN PARA MANEJAR LOS ESTADOS Y LOS LEDS
void estados() {
	if (estado == 0 || estado == 1) {
		if (estado == 1) {
			PORTD |= (1 << PIND4);
			writeTextUART("estado1\n");
			} else {
			PORTD &= ~(1 << PIND4);
			PORTD &= ~(1 << PIND2);
			writeTextUART("estado0\n");
		}
		} else if (estado == 2) {
		PORTD &= ~(1 << PIND4);
		PORTD |= (1 << PIND2);
		writeTextUART("estado2\n");
		} else {
		PORTD |= (1 << PIND4);
		writeTextUART("estado3\n");
	}
}

// FUNCIONES PARA GUARDAR Y CARGAR POSICIONES EN EEPROM
void save_pose(uint8_t pose_index) {
	if (pose_index < POSE_COUNT) {
		eeprom_update_word(&ee_poses[pose_index][0], valor_reescalado1);
		eeprom_update_word(&ee_poses[pose_index][1], valor_reescalado2);
		eeprom_update_word(&ee_poses[pose_index][2], valor_reescalado3);
		eeprom_update_word(&ee_poses[pose_index][3], valor_reescalado4);
	}
}

void load_pose(uint8_t pose_index) {
	if (pose_index < POSE_COUNT) {
		valor_reescalado1 = eeprom_read_word(&ee_poses[pose_index][0]);
		duty_cycle2A(valor_reescalado1);
		valor_reescalado2 = eeprom_read_word(&ee_poses[pose_index][1]);
		duty_cycle1B(valor_reescalado2);
		valor_reescalado3 = eeprom_read_word(&ee_poses[pose_index][2]);
		duty_cycle1A(valor_reescalado3);
		valor_reescalado4 = eeprom_read_word(&ee_poses[pose_index][3]);
		duty_cycle2B(valor_reescalado4);
	}
}

// SECUENCIAS DE MOVIMIENTOS PARA LOS SERVOS
void secuencia1() {
	duty_cycle2A(34);
	duty_cycle1B(22);
	duty_cycle1A(25);
	duty_cycle2B(15);
	_delay_ms(1000);
	duty_cycle2A(13);
	duty_cycle1B(22);
	duty_cycle1A(20);
	duty_cycle2B(25);
	_delay_ms(1000);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(18);
	duty_cycle2B(25);
	_delay_ms(1000);
	duty_cycle2A(25);
	duty_cycle1B(12);
	duty_cycle1A(21);
	duty_cycle2B(26);
	_delay_ms(1000);
	duty_cycle2A(25);
	duty_cycle1B(18);
	duty_cycle1A(21);
	duty_cycle2B(21);
	_delay_ms(250);
	duty_cycle2A(25);
	duty_cycle1B(12);
	duty_cycle1A(21);
	duty_cycle2B(26);
	_delay_ms(250);
	duty_cycle2A(25);
	duty_cycle1B(18);
	duty_cycle1A(21);
	duty_cycle2B(21);
	_delay_ms(250);
	duty_cycle2A(25);
	duty_cycle1B(12);
	duty_cycle1A(21);
	duty_cycle2B(26);
}

void secuencia2() {
	duty_cycle2A(25);
	duty_cycle1B(12);
	duty_cycle1A(21);
	duty_cycle2B(26);
	_delay_ms(1000);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(25);
	duty_cycle2B(15);
	_delay_ms(1000);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(18);
	duty_cycle2B(15);
	_delay_ms(150);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(25);
	duty_cycle2B(15);
	_delay_ms(150);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(18);
	duty_cycle2B(15);
	_delay_ms(150);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(25);
	duty_cycle2B(15);
	_delay_ms(150);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(18);
	duty_cycle2B(15);
	_delay_ms(150);
	duty_cycle2A(25);
	duty_cycle1B(22);
	duty_cycle1A(25);
	duty_cycle2B(15);
}

// INTERRUPCIÓN POR CAMBIO DE ESTADO EN PINES PC0, PC1 Y PC2
ISR(PCINT1_vect) {
	// COMPROBAR SI EL BOTÓN DE INCREMENTO (PC2) ESTÁ PRESIONADO
	if (!(PINC & (1 << BUTTON1_PIN))) {
		if (estado == 3) {
			estado = 0;
			} else {
			estado++;
		}
		estados();
		leds();
	}

	if (estado == 1 || estado == 2) {
		// COMPROBAR SI EL BOTÓN DE NAVEGACIÓN (PC1) ESTÁ PRESIONADO
		if (!(PINC & (1 << BUTTON2_PIN))) {
			if (eprom_pos == 4) {
				eprom_pos = 1;
				} else {
				eprom_pos++;
			}
			leds();
		}

		// COMPROBAR SI EL BOTÓN DE SELECCIÓN (PC0) ESTÁ PRESIONADO
		if (!(PINC & (1 << BUTTON3_PIN))) {
			if (estado == 1) {
				save_pose(eprom_pos - 1);
				} else {
				load_pose(eprom_pos - 1);
			}
		}
	}
}

// INTERRUPCIÓN POR RECEPCIÓN DE DATOS UART
ISR(USART_RX_vect) {
	opcion = UDR0;

	// PROCESAR COMANDOS RECEBIDOS POR UART
	if (anterior == '6' || anterior == '7' || anterior == '8' || anterior == '9') {
		if (estado == 3) {
			if (opcion == 'F') {
				buffer[buffer_index] = '\0';  // TERMINAR LA CADENA
				data_received = 1;
				buffer_index = 0;
				} else if (buffer_index < 9) {
				buffer[buffer_index++] = opcion;
			}
			} else {
			anterior = opcion;
		}
		} else {
		if (opcion == '1') {
			if (estado == 3) {
				estado = 0;
				} else {
				estado++;
			}
			estados();
			leds();
		}

		if (estado == 3) {
			if (opcion == '4') {
				secuencia1();
				} else if (opcion == '5') {
				secuencia2();
			}
		}
		anterior = opcion;
	}

	if (estado == 1 || estado == 2) {
		if (opcion == '2') {
			if (eprom_pos == 4) {
				eprom_pos = 1;
				} else {
				eprom_pos++;
			}
			leds();
		}

		if (opcion == '3') {
			if (estado == 1) {
				save_pose(eprom_pos - 1);
				} else {
				load_pose(eprom_pos - 1);
			}
		}
	}

	if (data_received == 1) {
		if (anterior == '6') {
			duty_cycle2A(atoi(buffer));
			} else if (anterior == '7') {
			duty_cycle1B(atoi(buffer));
			} else if (anterior == '8') {
			duty_cycle1A(atoi(buffer));
			} else if (anterior == '9') {
			duty_cycle2B(atoi(buffer));
		}
		data_received = 0;
		anterior = opcion;
	}
}
