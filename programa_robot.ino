//////////////////////////
// Library Includes
/////////////////////////
#include <Servo.h>
#include <SoftwareSerial.h>

/////////////////////////////////////////////////////////////////////////////////////////
// Constants - Define pins
/////////////////////////////////////////////////////////////////////////////////////////
#define IN1 4    // Input3 conectada al pin 4
#define IN2 12    // Input4 conectada al pin 12
#define ENA  3    // ENA conectada al pin 3 de Arduino

#define IN3 7    // Input3 conectada al pin 7
#define IN4 8    // Input4 conectada al pin 8 
#define ENB 6    // ENB conectada al pin 6 de Arduino

#define TRIG_DEL 11   // Trigger a wave     - Output Pin
#define ECHO_DEL 9    // Receive a echo     - Input Pin  

#define TRIG_TRAS 10   // Trigger a wave    - Output Pin
#define ECHO_TRAS 2    // Receive a echo     - Input Pin 

#define FOTO_IZQ 13  // Fotorreceptores
#define FOTO_DER 0

#define DIR_DEL 0
#define DIR_TRAS 1

#define DIR_UP 0
#define DIR_DOWN 1

const int dist_cajas;  // Needs a value! [jaBote]
 
/////////////////////////////////////////////////////////////////////////////////////////
// Global Variables and Objects
/////////////////////////////////////////////////////////////////////////////////////////
Servo servo;  // create servo object to control a servo

long dist_del;        // ultrasonic sensor variable
long tiempo_del;          // ultrasonic sensor variable
long dist_tras;        // ultrasonic sensor variable
long tiempo_tras;          // ultrasonic sensor variable

long obstaculo = 20;    // Distancia mínima a obstáculo para comenzar a frenar (cm). Pendiente de ajuste.
int variacion = 10;     // Grados de corrección del servo cuando se sale de la línea (º). Rendiente de ajuste
int grados;        // Se inicializa en el programa a 90º
int flag;      // Flag de posicionamiento en el bucle. Se inicializa en el programa a 1

/////////////////////////////////////////////////////////////////////////////////////////
// Function Headers
////////////////////////////////////////////////////////////////////////////////////////
void traccion(int velocidad, int sentido, int tiempo);
void carretilla(int velocidad, int sentido, int tiempo, int nivel);
void direccion(int posicion, int tiempo);
void leerDistanciadelantera();
void leerDistanciatrasera();
void redirecciona();

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void setup() {
	// inicia el protocolo de comunicación a PC para debuggear
	Serial.begin(9600);

	pinMode (ENB, OUTPUT); 
	pinMode (IN3, OUTPUT);
	pinMode (IN4, OUTPUT);

	pinMode (ENA, OUTPUT); 
	pinMode (IN1, OUTPUT);
	pinMode (IN2, OUTPUT);

	servo.attach(5); //Pin 5 para el PWM del servomotor

	pinMode(TRIG_DEL, OUTPUT); /*activación del trigDistancia como salida: para el pulso ultrasónico*/
	pinMode(ECHO_DEL, INPUT); /*activación del ecoDistancia como entrada: tiempo del rebote del ultrasonido*/
	pinMode(TRIG_TRAS, OUTPUT); /*activación del trigDistancia como salida: para el pulso ultrasónico*/
	pinMode(ECHO_TRAS, INPUT); /*activación del ecoDistancia como entrada: tiempo del rebote del ultrasonido*/

	pinMode(FOTO_IZQ,INPUT);
	pinMode (FOTO_DER,INPUT);
}

void loop(){
	// CICLO 1
	//----------------------------------------------------
	// Inicialización para cada ciclo
	grados = 90;
	flag = 1;
	
	// Pasos a seguir:
	// 1. Estamos en el punto 1
	delay(2000); // Esperamos 2 segundos antes de arrancar el código

	// 2. Motor a 90º
	direccion(90, 0);   // Ponemos las ruedas a 90º

	// 3.Seguidor de linea 1
	leerDistanciadelantera();   // lee distancia

	if(dist_del > obstaculo) { // mientras que la distancia sea menor que obstaculo
		traccion(255, DIR_DEL, 0);
		delay(2000);
	}
	
	leerDistanciadelantera();   // lee distancia
	
	// Seguidor
	while(dist_del > obstaculo) { // mientras que la distancia sea menor que obstaculo
		traccion(210,DIR_DEL, 0); // avanza 0.5 segundos hacia delante
		if (digitalRead(FOTO_IZQ) == 0) { // 1 es negro y 0 es blanco.
			grados = (grados - variacion);
			if(grados < 55) {
				grados = 55;
			}
			servo.write(grados);
			delay(1500);
			}
		if (digitalRead(FOTO_DER) == 0) { //
			grados = (grados + variacion);
			if(grados > 135) {
				grados = 135;
				}
			servo.write(grados);
			delay(1500);
		}
		leerDistanciadelantera(); // lee distancia
		// delay(100);
	}
	traccion(0, DIR_DEL, 0); // Frenar

	// Desde aquí nada ha sido probado, solo restructurado [jaBote]
	// Detectamos cajas (Punto A)  //hasta aquí esta hecho
	// 4.Pala en posición A1 (verde)
	while(flag == 1) {
		int sensorValue = analogRead(A1);
		if (sensorValue < 1000) { // Estamos en el punto A?
				carretilla(255, DIR_UP, 0); // No, sígue subiendo. velocidad sentido(0sube) tiempo
			}
		else {
			carretilla(0, 0, 0); //Sí, la para. Pasa
			flag = 2; // Cambiamos el valor de "flag" para que salga del bucle
			delay(2000);
		}
	}
	
	// 5.Avanzamos lo suficiente para poder subir la caja, distancia x
	direccion(90,0);   // ponemos las ruedas rectas (90º)
	leerDistanciadelantera();   // lee distancia
	while(dist_del > dist_cajas) { // mientras que la distancia sea menor que distancia de cajas
		traccion(255, DIR_DEL, 0);
		leerDistanciadelantera();
	}
	// 6. Subimos la caja hasta la posición A0 (lila)
	while(flag == 2) {
		int sensorValue = analogRead(A0);
		if (sensorValue<1000) { //Estamos en el punto A0?
				carretilla(255, DIR_UP, 0); //No, sígue subiendo. velocidad sentido(0sube) tiempo
		}
		else {
			carretilla(0, 0, 0); //Sí, la para. Pasa
			flag = 3; //Cambiamos el valor de "flag" para que salga del bucle
			delay(2000);
		}
	}
	
	// 7.Sensor de presión detecta que hay caja, continuamos.

	// 8.Retrocedemos hasta que el ultrasonido trasero detecta obstáculo

	direccion(90, 0);   //ponemos las ruedas rectas (90º)
	leerDistanciatrasera();   //lee distancia
	while(dist_tras > obstaculo) { //mientras que la distancia sea menor que distancia cajas
		traccion(255,DIR_TRAS,0);
		leerDistanciatrasera();
	}
	
	// 9.=3.Seguimos linea 2 hasta detectar cajas con ultrasonido delantero(Punto B)
	// ---REPETIMOS CÓDIGO SEGUIDOR LINEA 1 ¿CREAR FUNCIÓN?

	// 10.=5.Avanzamos hasta distancia x
	// Bajamos hasta posición A3 (amarillo)
	while(flag == 3) {
		int sensorValue = analogRead(A0);
		if (sensorValue < 1000) { //Estamos en el punto A0?
			carretilla(255, DIR_DOWN, 0); //No, sígue baja. velocidad sentido(0sube) tiempo
		}
		else {
			carretilla(0, 0, 0); //Sí, la para. Pasa
			flag = 4;              //Cambiamos el valor de "flag" para que salga del bucle
			delay(2000);
		}
	}

	//11.=8.Marcha atrás hasta obstáculo
	//SE QUEDA EN LA POSICIÓN INICIAL
	// // // CICLO 2

	// // // Pasos a seguir:
	// // // Estamos en el punto 1
	// // // Motor a 90º
	// // // Seguidor de linea 1
	// // // Detectamos cajas (Punto A)
	// // // Pala en posición A2 (azul)

	// // // Avanzamos lo suficiente para poder subir la caja, distancia x
	// // // Subimos la caja hasta la posición A0 (lila)
	// // // Sensor de presión detecta que hay caja, continuamos.
	// // // Retrocedemos hasta que el ultrasonido trasero detecta obstáculo
	// // // Seguimos linea 2 hasta detectar cajas con ultrasonido delantero(Punto B)
	// // // Avanzamos hasta distancia x
	// // // Bajamos hasta posición A2 (azul)
	// // // Marcha atrás hasta obstáculo

	// // // CICLO 3

	// // // Pasos a seguir:
	// // // Estamos en el punto 1

	// // // Motor a 90º
	// // // Seguidor de linea 1
	// // // Detectamos cajas (Punto A)
	// // // Pala en posición A3 (amarillo)

	// // // Avanzamos lo suficiente para poder subir la caja, distancia x
	// // // Subimos la caja hasta la posición A0 (lila)
	// // // Sensor de presión detecta que hay caja, continuamos.
	// // // Retrocedemos hasta que el ultrasonido trasero detecta obstáculo
	// // // Seguimos linea 2 hasta detectar cajas con ultrasonido delantero(Punto B)
	// // // Avanzamos hasta distancia x
	// // // Bajamos hasta posición A1 (verde)
	// // // Marcha atrás hasta obstáculo
}

/////////////////////////
// RESTO DE FUNCIONES  //
/////////////////////////

void traccion(int velocidad, int sentido, int tiempo) {
	if(sentido == DIR_TRAS){ 
		//Preparamos la salida para que el motor gire en un sentido (hacia atrás)
		digitalWrite (IN1, HIGH);
		digitalWrite (IN2, LOW);
	} else { // DIR_DEL
		//Preparamos la salida para que el motor gire en el otro sentido
		digitalWrite (IN1, LOW);
		digitalWrite (IN2, HIGH);    
	}

	// Aplicamos PWM al pin ENB
	analogWrite(ENA,velocidad);
	
	//Comprobar si hemos dado un valor de tiempo
	if(tiempo > 0) {
	delay(tiempo);
	analogWrite(ENA, 0);
	}
}

void carretilla(int velocidad, int sentido, int tiempo) {
	if(sentido == DIR_DOWN) {
		//Preparamos la salida para que el motor gire en un sentido
		digitalWrite (IN3, HIGH);
		digitalWrite (IN4, LOW);
	}
	else {
		//Preparamos la salida para que el motor gire en el otro sentido
		digitalWrite (IN3, LOW);
		digitalWrite (IN4, HIGH);
	}

	// Aplicamos PWM al pin ENB, haciendo girar el motor
	analogWrite(ENB,velocidad);

	//Comprobar si hemos dado un valor de tiempo
	if(tiempo > 0) {
		delay(tiempo);
		analogWrite(ENB, 0);
	}
}

void direccion(int posicion, int tiempo){
	// Comprobar si el valor de la posicion dada está entre 0 y 180 grados
	if((posicion >= 0) && (posicion <= 180)) {
		servo.write(posicion);
	}

	//Comprobar si hemos dado un valor de tiempo
	if(tiempo > 0) {
		delay(tiempo);
	}
}

void leerDistanciadelantera() {
	digitalWrite(TRIG_DEL, LOW); /* Por cuestión de estabilización del sensor */
	delayMicroseconds(5);
	digitalWrite(TRIG_DEL, HIGH); /* envío del pulso ultrasónico */
	delayMicroseconds(10);
	tiempo_del = pulseIn(ECHO_DEL, HIGH); /* Función para medir la longitud del pulso entrante.
	                                      Mide el tiempo que transcurrido entre el envío
	                                      del pulso ultrasónico y cuando el sensor recibe el rebote */
	dist_del = int(0.017*tiempo_del); /* fórmula para calcular la distancia obteniendo un valor entero */
}

void leerDistanciatrasera() {
	digitalWrite(TRIG_TRAS,LOW); /* Por cuestión de estabilización del sensor */
	delayMicroseconds(5);
	digitalWrite(TRIG_TRAS, HIGH); /* envío del pulso ultrasónico */
	delayMicroseconds(10);
	tiempo_tras = pulseIn(ECHO_TRAS, HIGH); /* Función para medir la longitud del pulso entrante.
	                                        Mide el tiempo que transcurrido entre el envío
	                                        del pulso ultrasónico y cuando el sensor recibe el rebote */
	dist_tras = int(0.017*tiempo_tras);     /* fórmula para calcular la distancia obteniendo un valor entero */
}

void redirecciona() {
	while(digitalRead(FOTO_IZQ) == 0) { //1 es negro 0 es blanco
		grados = (grados-variacion); //mirar signo
		servo.write(grados);
	delay(1500); //tiempo para que vuelva al carril
	}

	while(digitalRead(FOTO_DER) == 0) {
		grados = (grados+variacion);
		servo.write(grados);
		delay(1500);
	}
}

