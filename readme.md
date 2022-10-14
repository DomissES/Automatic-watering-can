# Automatyczna konewka

For english scroll below



### PL:

Program, który pomaga automatycznie nawadniać rośliny w czasie nieobecności.
Wystarczy włożyć sondę do ziemi, przygotować pojemnik z wodą i ustawić odpowiednie parametry. Układ wtedy sam będzie sprawdzał czy należy podlać roślinę i w razie potrzeby podleje.

#### 1. Główne zadania:
- Automatyczny pomiar wilgotności podłoża
- Załączenie pompki nawadniającej w razie potrzeby

#### 2. Dodatkowe zadania:
- Wyświetlanie godziny, temperatury i wilgotności powietrza, wilgotności podłoża
- Logowanie błędów, godziny nawadniania
- Wysyłka loga do PC przy pomocy UART
- Gaszenie ekranu w czasie bezczynności

#### 3. W projekcie zawarto:
- Obsługa czujnika temperatury i wilgotności powietrza DHT22
- Obsługa zewnętrznej pamięci EEPROM 24LC128-i
- Pomiar przy pomocy przetwonika ADC
- Obsługa timerów sprzętowych, w tym RTC
- Obsługa wyświetlacza na sterowniku HD44780
- Obsługa transmisji UART(19200 8N1), I2C
- Implementacja maszyny stanów
- Implementacja bufora pierścieniowego

#### 4. Wygląd zbudowanego układu:

![Built system](blob/master/working_system_edit.jpg)

#### 5. Schemat działania programu:

![App flow](blob/master/App_flow.png)


<details>
<summary>Kilka komentarzy</summary>

Docelowo ma być to dwustrefowy sterownik do szklarni, stąd tyle wyprowadzeń i definicji pinów w pliku defines.h.
W celu przejrzystości kodu i jego hermetyzacji występuje tutaj sporo funkcji "get" i "set", które są niewydajne w porównaniu do zmiennych globalnych.
</details>


<details>
<summary>Na temat układu elektronicznego</summary>

Układ oparty na mikrokontrolerze Atmega64A, płytki PCB zaprojektowane przez autora.
W obudowie znajduje się płytka z mikrokontrolerem, płytka zewnętrzna to układ zasilający.
Zasilanie z 12V, docelowo możliwość podłączenia pod ogniwo fotowoltaiczne z akumulatorem AGM.
</details>


<details>
<summary>Opis poszczególnych plików</summary>

- circ_buffer.c
	- Implementacja bufora pierścieniowego

- system.c
	- Zawiera podstawową obsługę błędów
	
- memory.c
	- Zawiera funkcje sprawdzające stan pamięci RAM oraz funkcje "malloc" i "free" będące non reentrant

- state_machine.c
	- Implementacja maszyny stanów
	- Oparte na wskaźnikach na funkcję

- program_states.c
	- Zawiera główne funkcje pełniące jako stany maszyny.
	- W przypadku dodania kolejnych stanów należałoby podzielić na osobne pliki

- period_task.c
	- Timer działający w trybie CTC o zmiennym czasie wywołania
	- Obsługa "zadań okresowych" wymagających "działania w tle", ale nie wymagających natychamiastowej reakcji, np.: pomiary na DHT22, pomiary ADC, wysyłka danych do wyświetlacza
	- Zadania kolejkowane w buforze
	- Im więcej zadań tym częstsze wywołanie przerwania
	- Wszystkie funkcje z przyrostkiem "_task" są napisane dla powyższego timera

- uart_service.c
	- Dwa bufory pierścieniowe
	- Możliwość wysyłki i odbioru łańcucha znaków
	- Możliwość wysyłki jednego bajta (blokuje procesor) - dla potrzeb debugowania

- twi_hw.c
	- Transmisja oparta na maszynie stanów, gdzie eventem jest status TWI (przy pomocy switch case)
	- Zastosowano bufor pierścieniowy

- twi_eeprom.c
	- Służy do komunikacji z 24LC128-i
	- Ze względu na stronicowanie pamięci zaimplementowano odpowiednie parsowanie danych


- rtc.c
	- Obsługa zegarka
	- Programowy timer (tick co 0,25 sekundy)

	
- log.c
	- Obsługa logowania działania programu
	- Dane na temat loga są zapisane na pierwszej stronie pamięci EEPROM, w razie braku takich danych są one tworzone na nowo
	- Wysyłka loga przez UART

- hdd44780.c
	- Podstawowe funkcje obsługi wyświetlacza. Biblioteka napisana przez Joerga Wunscha.

- lcd_service.c
	- Zawiera podstawową obsługę wyświetlacza, takie jak wyświetlenie łańcucha znaków, czyszczenie wyświetlacza.
	- Oparte na buforze pierścieniowym

- lcd_gui.c
	- Zamiana danych na tekst, parsowanie tekstu i wyświetlanie go
	- Tekst na wyświetlaczu zamienia się co kilka sekund, dane do wyświetlenia zostały podzielone na strony (stan, czas, temperatura, wilgotność powietrza) - znajdują się w pierwszej linii
	(wilgotność zadana, wilgotność zmierzona) - znajdują się w drugiej linii 
	- Dla ustawienia czasu i eksportu logu strony się nie wyświetlają

- hmi.c
	- Obsługa przycisków, enkodera, brzęczyka i LED
	- LEDy i brzęczyk mogą być włączane okresowo, całość dzieje się "w tle" (przy pomocy timera period_task)
	- W celu dodania różnych możliwości odczytu przycisku (długość wciśnięcia) funkcje wyglądają na dość skomplikowane
	- Tu kod można zoptymalizować używając bezpośrednio rejestrów, a nie makrodefinicji "SET(<bit>)" "CLEAR(<bit>)" - zostawiono tak dla przejrzystości kodu

- adc.c
	- Obsługa przetwornika ADC
	- Docelowo wszystkie kanały mają być wykorzystane, w danym projekcie wykorzystuje się tylko jeden.
	- Pomiar odbywa się przy pomocy timera "period_task", nie ma automatycznego wyzwalania pomiaru w celu uniknięcia przesunięcia pomiaru (zmiana kanału następuje po kolejnym dokonanym pomiarze)

- hg_sensor.c
	- Obsługa czujnika wilgotności gleby (konwersja danych z adc na zrozumiałą wartość)

- dht_sensor.c
	- Obsługa czujnika dht22
	- Transmisja opiera się na timerze i przerwaniu "input_capture"
	- Zawiera podstawowe sprawdzenie poprawności danych
	- Zakłada się, że transmisja nie zostanie przerwana, docelowo należy dodać kontrolę przebiegu transmisji
	- Pomimo tego, że transmisja przebiega jednym pinem, wyzwalanie termometru odbywa się przy pomocy innego pinu niż przy odbiorze danych. Jest to poniekąd wada tego typu transmisji (do input_capture są podłączone 3 czujniki DHT22) lub zły projekt układu (należałoby wybrać uC, który ma na każdym pinie przerwania)
	


</details>



### ENG:

This program lets you help watering a plant when you are away from home.
Just put the moisture sensor into the soil, preapre container with water and set the settings. The system will then check if the plant needs water and it waters if necessary.

#### 1. Main tasks:
- Periodical measurement of soil moisture
- Toggling water pump on if the moisture is too low

#### 2. Additional tasks:
- Time, air temperature and relative humidity, soil moisture display
- Error and watering time logging
- Sending log to PC via UART
- Turning backlight off if there is human inactivity

#### 3. In the project you can find:
- Temperature and %RH measurement with DHT22 sensor
- R/W communication with 24LC128-i EEPROM
- ADC measurements
- Use of timers, in which there is RTC timer
- Displaying text on a display with HD44780 driver
- Communication via UART(19200 8N1) and I2C
- Implementation of a state machine
- Implementation of a ring buffer

#### 4. How the physical controller looks like:

Check Point 4. in PL section

#### 5. Manual of the program:

Check Point 5. in PL section


<details>
<summary>Few comments</summary>

The controller was built to be a two-zone glasshouse controller. That's why there is so many pin definitions in file defines.h
For better clarity and hermetization of the code there is a lot of "get" and "set" functions, which are not as efficient as using global variables.
</details>


<details>
<summary>Few words about the physical controller</summary>

The main device is a Atmega64A microcontroller, the PCBs are designed by the author.
The box has the main PCB, the external PCB is a power supply PCB.
To power the controller you need a 12V power supply. In the future there would be support for photovoltaic cell with an AGM accumulator.
</details>


<details>
<summary>Detailed file description</summary>

- circ_buffer.c
	- Implementation of a ring buffer

- system.c
	- It includes basic error support
	
- memory.c
	- RAM check functions and non reentrant "malloc" and "free" function implementation

- state_machine.c
	- Implementation of a state machine
	- It is done using function pointers

- program_states.c
	- Main functions which are called by the state machine
	- In case of adding more states, the file should be split into smaller ones

- period_task.c
	- "Tasks" are something like services or similar, they should be running in background without the need for immediate execution e.g. DHT22 or ADC measures, sending data to LCD display
	- "Tasks" are function pointers are stored in a ring buffer
	- CTC timer with variable interrupt request periods pulls tasks from the buffer
	- The more tasks requested, the more frequent execution
	- All functions in project with "_task" suffix are developed for this purpose

- uart_service.c
	- Two ring buffers
	- Support for transmitting and receiving data strings
	- Support for transmitting one byte blocking the processor - for debugging purposes

- twi_hw.c
	- Transmission is implemented with help of a state machine in which TWI statuses are the state events (switch case realization)
	- One ring buffer for both receiving and transmitting data

- twi_eeprom.c
	- Support for 24LC128-i communication
	- Due to memory paging the data is parsed for proper operation
	

- rtc.c
	- Clock realization
	- Software timer (with 0,25s tick)

	
- log.c
	- Logging the program flow
	- Log metadata is stored on first page of EEPROM, in case of lack of this data there are made new ones
	- Exporting log is made via UART interface


- hdd44780.c
	- Basic LCD display support. The code is written by Joerg Wunsch


- lcd_service.c
	- Basic use of LCD display like displaying text string, clearing the LCD
	- Data to be sent is stored in a ring buffer


- lcd_gui.c
	- Data - text conversion, data parsing and displaying it then
	- Display has few pages (state, time, temperature, %RH) on first line and (target moisture, current moisture) on second line which are displayed one by one changing every few seconds
	- For setting the time or log export the pages are not displayed


- hmi.c
	- Support for buttons, encoder, buzzer and LEDs
	- LED and buzzer could be toggled periodically, everything is executed "in background" (with help of period_task)
	- The buttons could be read in different ways (e.g. how low the button was pressed), that's why the functions are quite complicated
	- The code could be more optimized by using registers instead of "SET(<bit>)" "CLEAR(<bit>)" - left in this way for better code clarity


- adc.c
	- ADC support
	- In the future all channels will be used, in this project is used only one
	- The measurement is executed by "period_task" timer, there is no auto triggering the measurement. It is done for avoiding measure shift (channel change is made after completing actual measurement)


- hg_sensor.c
	- Support for moisture sensor (mainly data conversion for better readability)
	

- dht_sensor.c
	- DHT22 support
	- Transmission is executed with help of a timer and "input_capture" interrupt request
	- Basic data and checksum control
	- It is assumed that the transmission is not interrupted, in the future there should be transmission control
	- Even if the transmission is done on one pin, triggering the sensor is done with other pin. It could be said it is disadvantage of this transmission type (input capture pin has connected three such sensors) or a project fault (there should be another microcontroller with input interrupt on every pin)
	


</details>
