# Sistemi Operativi

## Memory Allocators

La memoria è vista dal kernel come un grande array lineare. Esistono due modi per allocare memoria e ognuno dei due deve far fronte a Frammentazione e Tempo.

### Tipi di Allocazione

* ##### Slab Allocator - Memoria Fissa

  Viene utilizzata de ci sono tanti elementi di dimensione fissa da allocare e la memoria viene affettata in tante fette quanti sono gli elementi da allocare.

  Per soddisfare richieste in _O(1)_ potrei costruire uno SLAB formato da una lista di interi, ma a questa struttura è preferibile, poiché non posso utilizzare _malloc()_, l'_Array List_. In questo caso, se conosciamo la dimensione massima di una lista, mappo tale lista in array di _max-size_ elementi. Memorizzo la lista nell'array e:

  * Nel blocco di posizione _i_ metto l'indice dell'elemento successivo in lista
  * Negli altri blocchi metto _-1_

* ##### Buddy Allocator - Memoria Variabile

  Ha un costo maggiore rispetto allo SLAB, ma posso utilizzarlo come una _malloc_. La memoria viene vista come un albero binario con vista in ampiezza e viene ricorsivamente divisa in due a seguito di una richiesta. Un "buddy" di un blocco di memoria è una regione ottenuta partizionando la regione genitore.

  Per gestire al meglio gli elementi dell'albero, utilizzo una bitmap (array contenente soli _0_ e _1_) per far si che sappia facilmente se il nodo è libero oppure no.

### Interrupts & Syscalls

Ogni interazione con il SIstema Operativo viene innescata da Interrupt che può sorgere a causa di eccezioni interne, chiamate esplicite (syscall) o da eventi esterni. Per monitorare una interruzione potrei fare:

* Polling: Interrogo continuamente lo stato tramite un _while(1)_
* Dormo la maggior parte del tempo e poi vengo svegliato solo al verificarsi di un evento.

#### Interrupt Vector e Tabella Syscall

+ L'Interrupt vector è un vettore di puntatori a funzione che rappresentano _ISR_ che a loro volta gestiranno i vari interrupt.

+ La tabella delle Syscall invece contiene in ogni locazione il puntatore a funzione che gestisce quella determinata syscall. Alla tabella viene associato anche un vetto che contiene il numero di paramentri e l'ordine dei parametri che la determinata syscall prende in input.

### Processi

La creazione di un processo necessita delle Syscall _fork_. Mediante questa vengono create due istanze dello stesso processo (il figlio eredita il PCB del padre). Un processo muore solo con la chiamata esplicita _wait_, ovvero solo quando qualcuno legge il suo valore di ritorno.

* _fork()_: La memoria del padre viene completamente copiata nel figlio. Se l'istruzione successiva è una _exec()_ potremmo risparmiarci tale copia e utilizzare la syscall
* _vfork()_: Non copia la memoria del padre nella creazione del figlio. Se l'istruzione successiva NON è una _exec()_ potrei riscontrare un comportamento anomalo.

#### PCB - Process Control Block

Contiene:

+ PID
+ UID
+ Stato del programma (ready, running,...)
+ Stato della CPU (registri)
+ Informazioni di scheduling
+ Informazioni sulla memoria
+ Informazioni di I/O (Descrittori di file aperti)

#### Context Switch

È un particolare stato del SO durante il quale avviene il cambiamento del processo correntemente in esecuzione sulla CPU, permettendo a più processi di utilizzare la stessa CPU eseguendo più programmi contemporaneamente. Durante un cambio di contesto, il SO;

* Salva lo stato del processo P1 correntemente in esecuzione e carica il PCB2
* Salva lo stato del processo P2 e carica PCB1

#### CPU Scheduling

L'esecuzione di un processo può essere suddivisa in due:

* Esecuzione di Istruzioni - CPU BURSTS
* Attesa di Eventi - I/O BURSTS

Lo scheduler è un modulo che ha lo scopo di decidere quale processo dovrà andare in esecuzione, vedendo la coda di ready. Viene invocato solamente quando vi è una richiesta di I/O da parte di un processo. Esistono due tipi di Scheduler:

1. Preemptive: Permette di interrompere il processo in esecuzione a favore di un altro processo. Lo seleziona e lo lascia in esecuzione per un certo periodo di tempo; se allo scadere del tempo esso è ancora in esecuzione, verrà prelazionato lasciando spazio ad un altro processo
2. Non Preemptive: Non da la possibilità di interrompere l'esecuzione del processo, lasciandolo in esecuzione finché non si blocca e viene ricevuto un I/O

Esistono varie politiche di Scheduling:

* <u>FCFS</u> (First Come First Served): I processi vengono schedulati nell'ordine in cui giungono al sistema.  
* <u>SJF</u> (Shortest Job First): Seleziona il processo che utilizzerà per minor tempo la CPU. Impossibile da implementare poiché dovrei conoscere a priori il tempo di arrivo dei processi ed ha un problema di _starvation_ in quanto un processo potrebbe rimanere in attesa per troppo tempo prima che venga schedulato.
  * Esiste una variante dello SJF che però è _preemptive_: in questo caso se arriva un processo con CPU Burst < della CPU Burst di quello correntemente in esecuzione, quest'ultimo verrà interrotto lasciando il controllo della CPU a quello con lavoro minore.
* <u>Priority Scheduler</u>: Ad ogni processo è associato un numeretto che indica la priorità con cui esso dovrà essere schedulato. Più piccolo è il numeretto e maggiore sarà la sua priorità.
* <u>RR (Round Robin)</u>: Ad ogni processo viene assegnato un quanto di tempo, durante il quale al processo è assegnato l'uso della CPU. Per scandire i quanti, alla fine di ognuno di essi viene generato un timer interrupt. Lo scheduler manterrà una coda di processi ready e selezionerà il primo processo in coda; quando scade il quanto, il processo viene messo in fondo alla coda.
* <u>Real Time Scheduling</u>: Devono essere rispettate delle scadenze, altresì chiamate deadline. Suddivisi in Soft/Hard Real Time.
* <u>Multilivello</u>: Questo tipo di scheduler prevede la creazione di _classi_ di processi, che verranno categorizzati in base alle loro caratteristiche simili. Per ognuna di queste classi, verrà assegnata una priorità e potrei utilizzare una diversa politica di scheduling a seconda della classe che sto analizzando.
  * Cerco prima la classe con priorità massima con almeno un processo in ready
  * Continuo a schedulare seguendo la politica della classe di riferimento
  * La coda con priorità più bassa avrà il quanto più grande
* <u>Multiple Processor Scheduling</u>: Se ho a disposizione più CPU, lo scheduler drovrà seguire una di queste politiche
  * Homogeneous Processors: Tutti i core eseguono lo stesso set di istruzioni
  * Asymmetric Multiprocessing: Ho solo una CPU che esegue codice kernel e le interruzioni sono gestite da un solo core
  * Symmetric Multiprocessing: Tutti i processi sono in una coda di ready comune e questi sonno self-scheduling
  * Processor Affinity: L'esecuzione di un programma viene forzata su un particolare core della macchina

#### Valutazione Dello Scheduler Migliore - Queuing Models

Simulo i processi in base ad una distribuzione probabilistica esponenziale e faccio simulazioni attraverso la **Legge di Little**:
$$
n = λ\cdot W
$$
_n_ = Lunghezza della coda; _λ_ = Tempo medio di arrivo dei processi; _W_ = Tempo medio di attesa dei processi in coda