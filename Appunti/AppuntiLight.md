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
_n_ = Lunghezza della coda; _λ_ = Tempo medio di arrivo dei processi; _W_ = Tempo medio di attesa dei processi in coda.



### Main Memory

La CPU può accedere direttamente solo a memoria e a registri e la cache è posizionata tra i due con lo scopo di garantire un accesso più veloce quando si fa riferimento ad una istruzione o a un dato.

#### Allocazione Statica e Dinamica della Memoria - Address Binding

Il Dynamic Linker si occuperà di rimpiazzare gli indirizzi simbolici (le wildcard del file '.o') con indirizzi fisici (indirizzi del file sorgente). 

+ Early Binding = Avviene prima dell'esecuzione
+ Late Binding = Avviene durante l'esecuzione

Il kernel memorizza le informazioni relative alle aree di memoria allocate al processo _P_ in una tabella e le rende disponibili alla _MMU_ (circuito situato tra CPU e memoria), la quale si occuperà dell'effettiva traduzione di indirizzi da logici a fisici.

##### Metodi Per fare MMU

* Reallocation and Limit: Basta prendere l'indirizzo logico, sommarlo al mio indirizzo fisico e se va oltre il comparatore (<) avviene una trap.

  Indirizzo Fisico = Reallocation + Logico

* Contigous Allocation: Ciascun processo è contenuto in una singola sezione contigua della memoria, la quale può anche essere ricompattata copiandola nella parte alta.

* Multiple Partition Allocation: Divisione della memoria in tanti pezzi e ne do un pezzo a ciascun processo

#### Allocazione Memoria di un Processo

Un processo ha bisogno di memoria contigua e dovrà fare i conti con problemi quali frammentazione, se non vi è abbastanza spazio contiguo, e protezione, facendo in modo che la memoria tra due processi non si sovrapponga.

<u>Protezione della Memoria</u>: Gestita da due registri della CPU...

* Base: Indirizzo di partenza della memoria allocata
* Limit: Size della memoria allocata

... con lo scopo di assicurarsi che ogni processo abbia uno spazio di memoria separato che non interferisca con quello di un altro processo.

Per garantire ciò:

1. Verifica che l'indirizzo sia >= di base
2. Verifica che l'indirizzo sia < di (base + limit)
3. Se tutto è rispettato avviene il caricamento in memoria

#### Compattazione della Memoria

Viene utilizzata per limitare o evitare la frammentazione esterna, spostando tutte le aree di memoria libere verso una estremità della memoria a tempo di compilazione.

#### Allocazione Non Contigua di Memoria - Approcci

* **Segmentazione**: Un processo viene suddiviso in un insieme di segmenti di differenti dimensioni sparsi in memoria; tale memoria conterrà una <u>Tabella Dei Segmenti</u> contente:

  1. Base: Indirizzo fisico in memoria
  2. Limit: Lunghezza del segmento
  3. Bit di Validità: 0 == !Valit; 1 == Valid
  4. Permessi _rwx_

* **Paginazione**: L'OS suddivide il processo in parti di dimensione fissa da lui stabilite (sono potenze di 2) chiamate pagine e la memoria è partizionata in aree, chiamate frames, di grandezza uguale a quella di una pagina. La CPU genera indirizzi suddivisi in:

  * Numero di Pagina: Contiene indirizzi di base del programma
  * Offset di Pagina: Spiazzamento pagina

  Base + Offset vengono inviati all'_MMU_, la quale si occuperà di generare l'indirizzo fisico. Viene utilizzato un <u>TLB</u> = Array contenente indirizzo a cui accedo più spesso; rappresenta un dizionario con chiave numero di pagina e valore il valore della pagina.

#### Differenze tra Segmentazione e Paginazione

Per la corretta implementazione, entrambe gli approcci hanno bisogno di strutture hardware per farli funzionare efficientemente.

* Ogni <u>segmento</u> ha dimensione variabile e perciò è identificato da indirizzo di base e indirizzo di limite. Tali informazioni sono contenute nella tabella dei segmenti e il sistema può decidere di non rendere accessibili alcuni segmenti tramite l'aggiunta di un bit di controllo. La frammentazione però porta a segmentazione esterna e ha la necessita di ricompattare i segmenti.
* Architetture moderne usano la <u>paginazione</u> per virtualizzare l'_address space_ suddividendo la memoria in frames (pagine), di dimensione uguale. La parte alta dell'indirizzo verrà usata come "_chiave_" per la tabella delle pagine, evidenziando la pagina corrente, e la parte restante dell'indirizzo verrà utilizzato come offset all'interno della pagina. Può essere implementata anche in maniera gerarchica e anch'essa permette di proteggere zone di memoria attraverso l'utilizzo di bit di validità. 

#### TLB e MMU

Rappresenta un Array presente che risiede nell'MMU contenente gli elementi più utilizzati di recente dalla tabella delle pagine. Memorizza alcune informazioni principali, come numero di pagina, numero di frame, bit di modifica e il campo per la protezione. Per quanto riguarda il funzionamento, l'MMU cerca l'indirizzo arrivatogli da tradurre nel TLB, se presente lo usa, altrimenti legge dalla tabella delle pagine e lo inserisce nel TLB.

**Tempo di Accesso Effettivo a Memoria** = $p_hit(T_TLB + T_RAM)+ p_fault(2T_TLB+2T_RAM)$

L'MMU gestisce anche il <u>Page Protection</u> tramite l'utilizzo di bit che indicano se l'area di memoria è accessibile in lettura o sia lettura che scrittura. In più contiene bit di validità atti a determinare se la pagina associata è nello spazio di indirizzamento logico del processo oppure no (Valid/Invalid).

#### Condivisione di Memoria tra Processi

Posso fare in modo che due processi condividano memoria, mappando lo stesso frame su due tabelle diverse (<u>Shared Pages</u>).

- Shared Code: Copia in lettura
- Private Code and Data: Ogni processo mantiene una copia separata di codice e dati.

#### Gerarchia Tabella delle Pagine

Le pagine vengono suddivise in strati, in modo tale che possa implementare una paginazione su due livelli. Utilizzo la tecnica dello **Swapping**, che consiste nel caricare interamente in memoria ogni processo, eseguirlo per un certo periodo di tempo al termine del quale verrà spostato sul disco in un'area chiamata <u>Area di Swap</u>. Le operazioni che verranno eseguite saranno dunque due:

- <u>Swap-In</u>: Da disco a memoria
- <u>Swap-Out</u>: Da memoria a disco



### Virtual Memory

La virtual memory è una parte della gerarchia della memoria suddivisa in memoria + disco, infatti alcune componenti di un processo durante l'esecuzione si trovano in memoria, mentre altre risiederanno sul disco e saranno caricate solo quando necessario. In tal modo il kernel implementa l'illusione di una memoria più grande, combinando hardware con softare (MMU con Gestore della Memoria Virtuale). Virual memory può essere implementato tramite:

* **Paginazione Su Richiesta** (Demand Paging): Ogni porzione di indirizzamento prende il nome di pagina e tutte le pagine sono di egual dimensione (in potenza di 2). Viene in tal caso portata una pagina in memoria solo al momento del primo riferimento a una locazione appartenente alla pagina stessa.

  <u>Ottimizzazione Demand Paging</u>: **Copy on Write** (COW): Durante un _fork()_ viene creato un processo figlio come copia della memoria del padre, ma se subito dopo tale istruzione il figlio esegue una _exec()_ tale copia risulterebbe inutile poiché la sua memoria sarà rimpiazzata dai parametri di quest'ultima syscall. A tal proposito viene utilizzata la tecnica <u>COW</u>. Inizialmente entrambe i processi condivideranno le stesse pagine di memoria, marcate come _copy on write pages_ e quando uno dei due processi vorrà modificare una delle pagine, quest'ultima verrà copiata.

* **Segmentazione su Richiesta**: Implementato da sistemi operativi in cui l'hardware disponibile non è sufficiente per implementare la paginazione. La tabella dei segmenti conterrà un bit di validità che serve a verificare se il segmento si trova nella memoria fisica oppure no.

#### Politiche di Sostituzione delle Pagine

Il gestore della memoria dovrà verificare quale pagina dovrà essere sostituita quando si verifica un _page fault_ e non ci sono frames liberi in memoria e quanti frame allocare per ciascun processo tramite 3 algoritmi di sostituzione:

* <u>Politica Ottimale</u>: Sostituzione di tutte le pagine che non si useranno più nel periodo di tempo più lungo. Impossibile da implementare in quanto non possiamo conoscere a priori il comportamento futuro di un processo.
* <u>Politica FIFO</u>: Il sistema operativo mantiene una lista di tutte le pagine in memoria, dove la pagina di testa è la più vecchia e quella di coda è quella arrivata più di recente. La rimozione avviene dunque in testa.
* <u>Politica LRU</u>: Sostituzione della pagina utilizzata meno di recente con la pagina richiesta.

#### Working Set

L'insieme di lavoro, cioè l'insieme di tutte le pagine di un processo, viene detto working set. Ciò consiste nell'assicurarsi che il work di un processo sia caricato totalmente in memoria prima di consentire l'esecuzione di un altro processo. 

Quando parecchi processi iniziano a spendere più tempo per la paginazione che per l'esecuzione, essi inizieranno ad andare in **trashing**. In tal caso il SO aumenterà il grado di multiprogrammazione avviando nuovi processi che cominceranno a loro volta ad andare in trashing a causa della mancanza di frames liberi.



### File System

Risiede su uno storage secondario, sul disco, e fornisce sia un'interfaccia utente per la memorizzazione, mappatura logica a fisica che un'efficiente accesso al disco abilitando la conservazione dei dati.

#### Mount

Tramite l'operazione di mount, il FS viene informato che un nuovo FS è pronto per essere utilizzato. L’operazione, quindi, provvederà ad associarlo con un dato _mount-point_, ovvero la posizione all'interno della gerarchia del file system del sistema dove il nuovo file system verrà caricato. Prima di effettuare questa operazione di attach, ovviamente bisognerà controllare la tipologia e l’integrità del file system. Una volta fatto ciò, il nuovo file system sarà a disposizione del sistema.

#### Directory

Rappresenta nella realtà un insieme di voci di file, che può contenere:

- Hard Link: Puntatori a file che se eliminati tutti, eliminano il file stesso
- Soft Link: Puntatori "soft", non intaccano il contenuto del file
- Altre Directory

#### File Descriptor

È un intero ottenuto come valore di ritorno a seguito della chiamata alla syscall _open()_. A seguito di tale chiamata,  il sistema scandisce il FS in cerca del file e, una volta trovato, il **FCB** è copiato nella tabella globale dei file aperti. Per ogni singolo file aperto, anche se da più processi esiste una sola entry nella tabella globale dei file aperti. Viene, quindi, creata una entry all'interno della
tabella dei file aperti detenuta dal processo, la quale punterà alla relativa entry nella tabella globale. 

#### Devices e Implementazione Funzioni 

Il filesystem _/dev_ fornisce al sistema un aggancio per dispositivi fisici esterni. In Unix ogni device è accessibile come un file e questi vengono categorizzati in:

+ <u>A Blocchi</u>: Come ad esempio file di testo
+ <u>A caratteri</u>: Come ad esempio le telecamere che rappresentano stream

La funzione <u>ioctl</u> permette di interagire con il driver generico e tramite essa è possibile settare e ricavare i parametri di tale device, come ad esempio la risoluzione della webcam. Per configurare devices seriali a caratteri è possibile utilizzare le _API_ racchiuse nella interfaccia <u>termios</u>, che ci permette di avere accesso alle caratteristiche di tale device.

#### Dischi

Un disco è un dispositivo primitivo che può effettuare solo due operazioni basilari:

* **Lettura di un Blocco **
* **Scrittura di un Blocco**

Per dialogare direttamente con il disco, l'OS utilizza il driver del File System che implementa le primitive _open(), read(), write(), ..._ per accedere ai blocchi del disco (grande file binario).

>Continua a Pagina 33 - Struttura dati per la manipolazione di FIleSystem