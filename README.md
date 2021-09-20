# progetto_so

## What:

Il progetto consiste in due programmi:

- serial_sender.c : programma che gira sul computer, si occupa di ottenere in input un comando, lo scrive sulla seriale e legge la risposta.
- arduino.c : programma che andrà caricato sull'arduino, si occupa di leggere il comando, di mandare un messaggio di buona o cattiva trasmissione tramite seriale e di spostare i servo motori

## How:

### serial_sender.c

fa le seguenti operazioni:

1. Da linea di comando richiede come argomenti il filename della seriale e il rispettivo baudrate
2. Setta la comunizione
3. Richiede all'utente in input un comando di posizione del formato x:y 
4. Controlla che l'input corrisponda al formato, in caso contrario richiede l'input
5. Invia sulla seriale il comando, mandando anche il checksum
6. Legge la risposta e comunica se è andato a buon fino meno
7. ripete il processo dal punto 3 (inserire q per terminare)

### arduino.c

fa le seguenti operazioni:

1. Setta le varie porte e il Timer
2. Legge dalla seriale il comando e lo inserisce in bx_buffer: la lettura della seriale è gestita con degli interrupts, quindi avviene solamente se viene scritta qualcosa sulla seriale
3. Controlla se il messaggio è corretto o vi sono stati errori in trasmissione
4. Manda una conferma di buona o cattiva ricezione:
   buona -> rinvia il comando
   cattiva -> invia un numero preimpostato (200) che corrisponde a un messaggio di errore
   La scrittura su serial è gestita con un buffer (tx_buffer) e interrups
5. Esegue il comando: modifica opportunamente i valori di OCR1A e OCR1B in modo da spostare i due servo motori
6. ripete il processo dal punto 2

## How to run:

1. Collegare l'arduino al pc

2. Nella directory avr dentro la directory principale (progetto_so) caricare arduino.c su arduino:

   ```
   make
   make arduino.hex
   ```

3. Nella directory principale (dentro progetto_so):

   ```
   make
   ./serial_sender /dev/ttyAMC0 19200
   ```

   questo compila e avvia il serial_sender, appariranno quindi su terminale le seguenti informazioni:

   ```
   Opening serial device [/dev/ttyACM0] ... Success
   Setting baudrate [19200] ...Success
   
   Please enter new camera position, using the format x:y in degrees from 0 to 180 (e.g. 90:180) or q to close the program.
   
   New position or q to quit: 
   ```

4. Inserire il comando, per cui sul terminale avremo:

   ```
   ./serial_sender /dev/ttyACM0 19200
   Opening serial device [/dev/ttyACM0] ... Success
   Setting baudrate [19200] ...Success
   
   Please enter new camera position, using the format x:y in degrees from 0 to 180 (e.g. 90:180) or q to close the program.
   
   New position or q to quit: 89:56
   Thanks! Trying to send: 89 and 56
   Calculating the checksum...
   Checksum: 185
   Sending...		[Sent]
   Waiting response...	[Arrived]
   Position set: 89 56 
   
   New position or q to quit: 
   ```

   5. Inseriamo q per terminare:

      ```
      ./serial_sender /dev/ttyACM0 19200
      Opening serial device [/dev/ttyACM0] ... Success
      Setting baudrate [19200] ...Success
      
      Please enter new camera position, using the format x:y in degrees from 0 to 180 (e.g. 90:180) or q to close the program.
      
      New position or q to quit: 89:56
      Thanks! Trying to send: 89 and 56
      Calculating the checksum...
      Checksum: 185
      Sending...		[Sent]
      Waiting response...	[Arrived]
      Position set: 89 56 
      
      New position or q to quit: q
      
      Program closed!
      ```

      



