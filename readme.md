# Automatyczna konewka

For english scroll below



### PL:

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
- Obsługa transmisji UART, I2C
- Implementacja maszyny stanów
- Implementacja bufora pierścieniowego

#### 4. Wygląd zbudowanego układu:

#### 5. Schemat działania programu:


Działanie programu:

Gotowość:
	A: Ustaw godzinę
	B: Log:
			A: Wyślij log
			B: Anuluj
			ENC: Skasuj log
	ENC: Ustaw parametry pracy: (docelowa wilgotność, czas załączenia pompki, okres pomiaru)
		ENC: przejdź dalej
		
Praca:
ENC3s: wróć do stanu gotowości
	
ENC3s: Wyłącz układ


<details>
<summary>Kilka komentarzy</summary>

Docelowo ma być to dwustrefowy sterownik do szklarni, stąd tyle wyprowadzeń i definicji pinów w pliku defines.h.
W celu przejrzystości kodu i jego hermetyzacji występuje tutaj sporo funkcji "get" i "set", które są niewydajne w porównaniu do zmiennych globalnych.
</details>


<details>
<summary>Na temat układu elektronicznego</summary>

Układ oparty na mikrokontrolerze Atmega64A, płytki PCB zaprojektowane przez autora.
W obudowie znajduje się płytka z mikrokontrolerem, płytka zewnętrzna to układ zasilający.
Zasilanie z 12V, docelowo możliwość podłączenia pod ogniwo fotowoltaiczne z akumulatorem żelowym.
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
	- Zawiera podstawową obsługę maszyny stanów
	- Oparte na wskaźnikach na funkcję

- program_states.c
	- Zawiera główne funkcje pełniące jako stany maszyny.
	- W przypadku dodania kolejnych stanów należałoby podzielić na osobne pliki

- period_task.c
	- Timer działający w trybie CTC o zmiennym czasie wywołania
	- Obsługa "zadań okresowych" wymagających "działania w tle", ale nie wymagających natychamiastowej reakcji, np.: pomiary na DHT22, pomiary ADC, wysyłka danych do wyświetlacza
	- Zadania kolejkowane w buforze
	- Im więcej zadań tym częstsze wywołanie przerwania
	- wszystkie funkcje z przyrostkiem "_task" są napisane dla powyższego timera

- uart_service.c
	- Dwa bufory pierścieniowe
	- Możliwość wysyłki łańcucha znaków
	- Jak i wysyłka jednego bajta blokująco procesor

- twi_hw.c
	- Transmisja oparta na maszynie stanów, gdzie eventem jest status TWI
	- Zastosowano bufor pierścieniowy

- twi_eeprom.c
	- służy do komunikacji z 24LC128-i
	- należało zastosować stronicowanie łańcucha znaków w celu poprawnego zapisu.


- rtc.c
	- Obsługa zegarka
	- Programowy timer (tick co 0,25sekundy)

	
- log.c
	- Obsługa logowania działania programu
	- dane na temat loga są zapisane na pierwszej stronie pamięci EEPROM, w razie braku takich danych są one tworzone na nowo

- hdd44780.c
	- podstawowe funkcje obsługi wyświetlacza. Biblioteka napisana przez Joerga Wunscha.

- lcd_service.c
	- Zawiera podstawową obsługę wyświetlacza, takie jak wyświetlenie łańcucha znaków, czyszczenie wyświetlacza.
	- Oparte na buforze pierścieniowym

- lcd_gui.c
	- Zamiana danych na tekst, parsowanie tekstu i wyświetlanie go
	- Tekst na wyświetlaczu zamienia się co kilka sekund, dane do wyświetlenia zostały podzielone na strony (stan, czas, temperatura, wilgotność powietrza) - znajdują się w pierwszej linii
	(wilgotność zadana, wilgotność zmierzona) - znajdują się w drugiej linii 
	- dla ustawienia czasu i eksportu logu strony się nie wyświetlają

- hmi.c
	- Obsługa przycisków, enkodera, brzęczyka i LED
	- LEDy i brzęczyk mogą być włączane okresowo, całość dzieje się "w tle" (przy pomocy timera period_task)
	- w celu dodania różnych możliwości odczytu przycisku (długość wciśnięcia) funkcje wyglądają na dość skomplikowane
	- tu kod można zoptymalizować używając bezpośrednio rejestrów, a nie makrodefinicji "SET()" "CLEAR()" - zostawiono tak dla przejrzystości kodu

- adc.c
	- obsługa przetwornika ADC
	- docelowo wszystkie kanały mają być wykorzystane, w danym projekcie wykorzystuje się tylko jeden.
	- Pomiar odbywa się przy pomocy timera "period_task", nie ma automatycznego wyzwalania pomiaru w celu uniknięcia przesunięcia pomiaru (zmiana kanału następuje po kolejnym dokonanym pomiarze)

- hg_sensor.c
	- obsługa czujnika wilgotności gleby (konwersja danych z adc na zrozumiałą wartość)

- dht_sensor.c
	- obsługa czujnika dht22
	- transmisja opiera się na timerze i przerwaniu "input_capture"
	- zawiera podstawowe sprawdzenie poprawności danych
	- zakłada się, że transmisja nie zostanie przerwana, docelowo należy dodać kontrolę przebiegu transmisji
	- Pomimo tego, że transmisja przebiega jednym pinem, wyzwalanie termometru odbywa się przy pomocy innego pinu niż przy odbiorze danych. Jest to poniekąd wada tego typu transmisji (do input_capture są podłączone 3 czujniki DHT22) lub zły projekt układu (należałoby wybrać uC, który ma na każdym pinie przerwania)
	


</details>

### ENG:
