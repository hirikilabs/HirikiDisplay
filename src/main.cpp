#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <ESP32WebServer.h>
#include <ESPmDNS.h>

// Constants
/////////////////////////////////////////////////////////////////
const char* ssid     = "HIRIKILABS";
const char* password = "XXXXXXXXXX";

#define NUM_MENSAJES 8
const char* mensajes[NUM_MENSAJES] = {
	"JASOTSERA!!!",
	"RECUERDA APAGAR LAS MAQUINAS",
	"GOGORATU MAKINAK ITZALTZEA",
	"CUIDA EL MATERIAL",
	"MATERIALA ZAINDU",
	"GORA HIRIKILABS",
	"CARLOS BAJA LA VOZ",
	"CARLOS JAITSI AHOTSA"
};

const char* ayuda = \
"Display de HIRIKILABS\n---------------------\n\n" \
"El display acepta comandos por HTTP en las siguientes direcciones:\n" \
"\t/pacman - hace una animación de pacman\n" \
"\t/invader - hace una animación de un space invader\n" \
"\t/blink - hace un parpadeo rápido 10 veces\n" \
"\t/blink?pausa=X&veces=Y - Hace un parpadeo, con una velocidad X e Y veces\n" \
"\t/invert - invierte el color de todos los pixeles\n" \
"\t/center?msg=Texto - Muestra el mensaje 'Texto' centrado en el display\n" \
"\t/scroll?msg=Texto&pausa=X - Hace un scroll con el mensaje 'Texto' a la velocidad X\n" \
"\t/alarm?msg=Texto&pausa=X&veces=Y - Muestra un mensaje de alarma con el 'Texto' centrado y sonido a velocidad X e Y veces\n" \
"\t/music?pausa=X&nota1=Y&nota2=Z ... &notaN=N - Reproduce las notas de frecuencia Y, Z ... N con una duración y pausa X entre ellas\n" \
"\n" \
"Ejemplo: \n" \
"\t IP/scroll?msg=Hola Mundo&pausa=25\n\n";

const uint8_t pacman1[] = {
	B00111100,
	B01111110,
	B11111011,
	B11111111,
	B11101111,
	B11101111,
	B01101110,
	B00101100
};

const uint8_t pacman2[] = {
	B00111100,
	B01111110,
	B11111011,
	B11111111,
	B11111111,
	B11101111,
	B01000110,
	B00000000
};

const uint8_t invader1a[] = {
	B00011110,  // First frame of invader #1
	B00111100,
	B11101111,
	B00111100,
	B00111100,
	B11101111,
	B00111100,
	B00011110
};

const uint8_t invader1b[] = {
	B01111000, // Second frame of invader #1
	B00111101,
	B11101110,
	B00111100,
	B00111100,
	B11101110,
	B00111101,
	B01111000
};

const int8_t wave[16] = {0, -1, -1, -2, -2, -2, -1, -1, 0, 1, 1, 2, 2, 2, 1, 1 };

// Variables and defines
/////////////////////////////////////////////////////////////////////////////
#define SCREENSAVER_TIMEOUT   30000

#define	MAX_DEVICES	32
#define PIXELS_W  256
#define	CHAR_SPACING	1

#define	CS_PIN		14  // or SS

#define SPK_PIN	12

// passed time
uint64_t last_millis = 0;

// Matrix display
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
// Hardware SPI
// Data = MOSI D23, CLK = SCK D18
MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);

// Wifi Server
ESP32WebServer server(80);

// Functions
/////////////////////////////////////////////////////////////////////////////


// Utilities
void setIntensity(int intensity)
{
	if (intensity >= 0 && intensity <= MAX_INTENSITY)
		mx.control(MD_MAX72XX::INTENSITY, intensity);
}

void scrollLeft()
{
	mx.transform(MD_MAX72XX::TSL);
}

void scrollRight()
{
	mx.transform(MD_MAX72XX::TSR);
}

void invert()
{
	mx.transform(MD_MAX72XX::TINV);
}

// Text related
void printTextCol(int col, const char* text)
{
	uint8_t text_len = strlen(text);
	uint8_t buffer[8];
	uint8_t char_len = 0;

	if (text_len <= 0)
		return;

	for (int i=text_len-1; i>=0; i--)
	{
		// get character
		char_len = mx.getChar(text[i], 8, buffer);
		for (int j=char_len-1; j>=0; j--)
		{
			mx.setColumn(col, buffer[j]);
			col++;
		}
		mx.setColumn(col, 0);
		col++;
	}
}


void printTextCentered(const char* text)
{
	uint8_t buf[8];
	// get string length (in pixels)
	uint16_t msg_pixels = 0;
	uint8_t msg_len = strlen(text);

	if (!(msg_len>0))
		return;

	for (int i=0; i<msg_len; i++)
	{
		uint16_t char_len = mx.getChar(text[i], 8, buf);
		msg_pixels += char_len;
	}
	// and space between chars
	msg_pixels += msg_len-1;

	// Calculate position
	uint8_t col = (PIXELS_W-msg_pixels) / 2;

	// print message there
	printTextCol(col, text);

}


void printTextScroll(const char* text, uint8_t pause)
{
	uint8_t buf[8];
	uint16_t scroll_size = 0;

	// get string length (in pixels)
	uint16_t msg_pixels = 0;
	uint8_t msg_len = strlen(text);

	if (!(msg_len>0))
		return;

	Serial.print("msg len: ");
	Serial.println(msg_len);
	for (int i=0; i<msg_len; i++)
	{
		uint16_t char_len = mx.getChar(text[i], 8, buf);
		msg_pixels += char_len;
	}

	// and space between chars
	msg_pixels += msg_len-1;

	// ok, scroll size?
	scroll_size = PIXELS_W + (msg_pixels);
	Serial.print("scroll size: ");
	Serial.println(scroll_size);

	// create text buffer
	uint8_t* text_buffer = (uint8_t*) malloc ((msg_pixels+1) * sizeof(uint8_t));
	// and fill it
	uint16_t col = 0;
	for (uint16_t i=0; i<msg_len; i++)
	{
		// get character
		uint16_t char_len = mx.getChar(text[i], 8, buf);
		for (uint8_t j=0; j<char_len; j++)
		{
			text_buffer[col] = buf[j];
			col++;
		}
		// char spacing
		text_buffer[col] = 0;
		col++;
	}

	// make scroll
	mx.clear();

	for (uint16_t i=0; i<scroll_size; i++)
	{
		// put new col
		if (i<msg_pixels)
		{
			mx.setColumn(1, text_buffer[i]);
		}
		else
		{
			mx.setColumn(1, 0);
		}

		delay(pause);
		scrollLeft();
	}

	free(text_buffer);
}

void printTextScrollWave(const char* text, uint8_t pause)
{
	uint8_t buf[8];
	uint16_t scroll_size = 0;
	uint8_t wave_counter = 0;

	// get string length (in pixels)
	uint16_t msg_pixels = 0;
	uint8_t msg_len = strlen(text);

	if (!(msg_len>0))
		return;

	for (int i=0; i<msg_len; i++)
	{
		uint16_t char_len = mx.getChar(text[i], 8, buf);
		msg_pixels += char_len;
	}

	// and space between chars
	msg_pixels += msg_len-1;

	// ok, scroll size?
	scroll_size = PIXELS_W + (msg_pixels);

	// create text buffer
	uint8_t* text_buffer = (uint8_t*) malloc ((msg_pixels+1) * sizeof(uint8_t));
	// and fill it
	uint8_t col = 0;
	for (int i=0; i<msg_len; i++)
	{
		// get character
		uint16_t char_len = mx.getChar(text[i], 8, buf);
		for (int j=0; j<char_len; j++)
		{
			text_buffer[col] = buf[j];
			col++;
		}
		// char spacing
		text_buffer[col] = 0;
		col++;
	}

	// make scroll
	mx.clear();

	for (int i=0; i<scroll_size; i++)
	{
		// put new col
		if (i<msg_pixels)
		{
			mx.setColumn(1, text_buffer[i]);
		}
		else
		{
			mx.setColumn(1, 0);
		}

		delay(pause);
		Serial.print(i);
		Serial.print(",");
		scrollLeft();
		// wave
		if (wave[wave_counter] > 0)
		{
			mx.transform(MD_MAX72XX::TSU);
		}
		else if (wave[wave_counter] < 0)
		{
			mx.transform(MD_MAX72XX::TSD);
		}
		wave_counter++;
		if (wave_counter>15)
			wave_counter = 0;

	}

	free(text_buffer);
}


// Sprites
void printSpriteCol(uint16_t col, const uint8_t* sprite)
{
	if (col < PIXELS_W-8)
	{
		for (int i=7; i>=0; i--)
		{
			mx.setColumn(col+i, sprite[i]);
		}
	}
	else
	{
		for (int i=PIXELS_W-col-1; i>=0; i--)
		{
			mx.setColumn(col+i, sprite[i]);
		}
	}
}

// Complex
void blink(int pause, int times) {
	for (int i=0; i<times; i++)
	{
		for (int j=0; j<MAX_INTENSITY; j++)
		{
			setIntensity(j);
			delay(pause);
		}
		for (int j=MAX_INTENSITY-1; j>=0; j--) {
			setIntensity(j);
			delay(pause);
		}
	}
	setIntensity(MAX_INTENSITY-1);
}

void alarm(const char* s, uint8_t pause, uint8_t times)
{
	if (strlen(s) > 0 && pause > 0)
	{
		mx.clear();
		printTextCentered(s);

		for (int i=0; i<times; i++) 
		{
			// brightness is 0-15
			for(int j = MAX_INTENSITY; j >= 0; j--)
			{
				setIntensity(j);
				ledcWriteTone(0, (j*38)+440);
				delay(pause);
			}
			for(int j = 0; j <= MAX_INTENSITY; j++)
			{
				setIntensity(j);
				ledcWriteTone(0, (j*38)+440);
				delay(pause);
			}
		}

		ledcWriteTone(0, 0);

	}
}

void pacman()
{
	for (uint16_t i=0; i<=PIXELS_W; i++)
	{
		if (i%2)
			printSpriteCol(i, pacman1);
		else
			printSpriteCol(i, pacman2);

		mx.setColumn(i-1, 0);
		delay(50);
	}
}

void invader()
{
	for (uint16_t i=0; i<=PIXELS_W; i++)
	{
		if (i%2)
			printSpriteCol(i, invader1a);
		else
			printSpriteCol(i, invader1b);

		mx.setColumn(i-1, 0);
		delay(50);
	}
}

// server callbacks
void rootRequest()
{
	String ayudaString;
	String message = "Hiriki Display - IP ";
	message.concat(WiFi.localIP().toString());
	message.concat("\n\n");
	ayudaString.concat(ayuda);
	ayudaString.replace("IP", WiFi.localIP().toString());
	message.concat(ayudaString);
	server.send(200, "text/plain; charset=UTF-8", message);
}

void scrollRequest()
{
	if (server.args() < 2) 
	{
		server.send(200, "text/plain; charset=UTF-8", "Problema con parámetros\n\n");
		return;
	}
	server.send(200, "text/plain; charset=UTF-8", "Enviado scroll\n\n");
	printTextScroll(server.arg(0).c_str(), server.arg(1).toInt());
	last_millis = millis();
}

void waveRequest()
{
	server.send(200, "text/plain; charset=UTF-8", "Enviado wave\n\n");
	if (server.args() < 2)
		return;
	printTextScrollWave(server.arg(0).c_str(), server.arg(1).toInt());
	last_millis = millis();
}

void centerRequest()
{
	server.send(200, "text/plain; charset=UTF-8", "Enviado texto centrado\n\n");
	if (server.args() < 1)
		return;
	printTextCentered(server.arg(0).c_str());

	last_millis = millis();
}

void blinkRequest()
{
	server.send(200, "text/plain; charset=UTF-8", "Enviado parpadeo\n\n");
	if (server.args() < 2)
		blink(10, 10);
	else 
	{
		int s = server.arg(0).toInt();
		int t = server.arg(1).toInt();
		blink(s, t);
	}
	last_millis = millis();
}

void alarmRequest()
{
	if (server.args() < 3) 
	{
		server.send(200, "text/plain; charset=UTF-8", "Error alarma - pocos parámetros\n\n");
		return;
	}
	else 
	{
		server.send(200, "text/plain; charset=UTF-8", "Enviada alarma\n\n");
		uint8_t p = server.arg(1).toInt();
		uint8_t t = server.arg(2).toInt();
		alarm(server.arg(0).c_str(), p, t);

	}
	last_millis = millis();
}

void musicRequest()
{
	if (server.args() < 2)
	{
		server.send(200, "text/plain; charset=UTF-8", "Error música - pocos parámetros\n\n");
		return;
	}
	else 
	{
		server.send(200, "text/plain; charset=UTF-8", "Enviada música\n\n");

		// pausa
		int pausa = server.arg(0).toInt();

		// bucle de las notas
		for (int i = 1; i<server.args(); i++)
		{
			if (server.argName(i)=="p")
			{
				// pausa
				delay(server.arg(i).toInt());
			}
			else
			{
				int nota = server.arg(i).toInt();
				ledcWriteTone(0, nota);
				delay(pausa);
				ledcWriteTone(0, 0);
				delay(pausa);
			}
		}
	}

	last_millis = millis();
}

void pacmanRequest()
{
	server.send(200, "text/plain; charset=UTF-8", "Enviado Pacman\n\n");
	pacman();
	last_millis = millis();
}

void invaderRequest()
{
	server.send(200, "text/plain; charset=UTF-8", "Enviado Invader\n\n");
	invader();
	last_millis = millis();
}

void invertRequest()
{
	server.send(200, "text/plain; charset=UTF-8", "Enviada Inversión\n\n");
	invert();
	last_millis = millis();
}


void notFoundRequest()
{
	String message = "404 - Not Found\n\n";
	server.send(404, "text/plain; charset=UTF-8", message);
}

// Setup and loop
void setup() {
	Serial.begin(115200);
	mx.begin();
	mx.wraparound(MD_MAX72XX::OFF);
	mx.clear();
	WiFi.begin(ssid, password);

	// connecting
	int i=0;
	int inc=1;
	while (WiFi.status() != WL_CONNECTED) {
		delay(10);
		mx.setColumn(i, inc==1?255:0);
		i=i+inc;
		if (i>PIXELS_W)
		{
			inc = -1;
		}
		if (i==0)
		{
			inc = 1;
		}
	}

	mx.clear();
	// ok, show IP
	printTextScroll(WiFi.localIP().toString().c_str(), 25);

	// start mDNS responder
	if (MDNS.begin("display"));  // http://display.local
	//printTextScroll("http://display.local", 25);

	// start HTTP server
	server.begin();

	// register paths
	server.on("/", rootRequest);
	server.on("/scroll", scrollRequest);
	server.on("/wave", waveRequest);
	server.on("/center", centerRequest);
	server.on("/blink", blinkRequest);
	server.on("/invert", invertRequest);
	server.on("/pacman", pacmanRequest);
	server.on("/invader", invaderRequest);
	server.on("/alarm", alarmRequest);
	server.on("/music", musicRequest);
	server.onNotFound(notFoundRequest);

	// configure speaker
	ledcSetup(0, 2000, 8);
	ledcAttachPin(SPK_PIN, 0);
	digitalWrite(SPK_PIN, LOW);
	ledcWrite(0, 200);
	ledcWriteTone(0, 0);

	// reset time
	last_millis = millis();
}


void loop() {
	server.handleClient();
	// screensaver
	if (millis() > last_millis + 30000)
	{
		int msg = random(NUM_MENSAJES+1);
		if (msg < NUM_MENSAJES)
		{
			printTextCentered(mensajes[msg]);
			delay(1000);
		}
		else
			printTextCentered(WiFi.localIP().toString().c_str());
		pacman();
		last_millis = millis();
	}
}
