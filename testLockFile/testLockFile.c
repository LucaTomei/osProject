#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

/*
Per avere il lock esclusivo su un file, utilizzerò la system call fcntl
(int fcntl(int fd, int cmd, ...  arg  ) e la struct flock.
-   La prima prende in input un file descriptor di un file aperto e un valore che indica quale operazione
    deve essere eseguita. Questa chiamata a sistema consente ad un processo di inserire un blocco
    in lettura o in scrittura su un file. Solo un processo può contenere il blocco di scrittura su 
    uno stesso file.
-   La struct flock così composta...
        struct flock {
                   ...
                   short l_type;    ---> Tipo di lock (F_RDLCK, F_WRLCK, F_UNLCK)
                   short l_whence;  ---> Come interpretare l_start (SEEK_SET, SEEK_CUR, SEEK_END)              
                   off_t l_start;   ---> Offset di partenza per il lock
                   off_t l_len;     ---> Numero di byte da lockare
                   pid_t l_pid;     ---> Pid del processo bloccante
                   ...
        };
    ... permette di posizionare un blocco su un file. Infatti settando il campo l_type con F_WRLCK e 
    l_whence con SEEK_SET, ottengo il desiderato blocco in scrittura su quel file dall'inizio del file.
    Se un altro processo contiene un blocco che impedisce l'acquisizione di un nuovo blocco,
    blocca i blocchi fcntl fino al suo rilascio.
*/

static int lockfile(const char *const filepath, int *const fdptr)
{
    struct flock lock;
    int used = 0; /* Bits da 0 a 2: stdin, stdout, stderr */
    int fd;

    /* Se i futuro mi servirà il descrittore, lo inizializzo con -1 (non valido) */
    if (fdptr)
        *fdptr = -1;

    /* Path non valido*/
    if (filepath == NULL || *filepath == '\0')  return errno = EINVAL;

    /* Open the file. */
    do {
        fd = open(filepath, O_RDWR | O_CREAT, 0600);    // apro il file in lettura
    } while (fd == -1 && errno == EINTR);
    if (fd == -1) {
        if (errno == EALREADY)  errno = EIO;    // ritorno EIO = Input/output error
        return errno;
    }

    /* sposto il file lontano dai file descriptor standard, creando una copia*/
    while (1){
        if (fd == STDIN_FILENO) {
            used |= 1;
            fd = dup(fd);
        } else
        if (fd == STDOUT_FILENO) {
            used |= 2;
            fd = dup(fd);
        } else
        if (fd == STDERR_FILENO) {
            used |= 4;
            fd = dup(fd);
        } else
            break;
    }

    /* Chiudo i descrittori standard che temporaneamente abbiamo usato. */
    if (used & 1){
        printf("Ciao\n");
        close(STDIN_FILENO);
    }
    if (used & 2){
        printf("Ciao\n");
        close(STDOUT_FILENO);
    }
    if (used & 4){
        printf("Ciao 3\n");
        close(STDERR_FILENO);
    }

    /* Se sono finiti i descrittori  */
    if (fd == -1)   return errno = EMFILE;    

    /* Lock esclusiva su un file, riguarda l'intero file!*/
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(fd, F_SETLK, &lock) == -1) {  // lock fallita, chiudo il file e ritorno l'errore
        close(fd);
        return errno = EALREADY;
    }

    /* Save salvo il descrittore */
    if (fdptr)  *fdptr = fd;

    return 0;
}

int main(int argc, char const *argv[]){
    while(1){
        sleep(1);
        int result;
        result = lockfile("lockTest.txt", NULL);
        if (result == 0)    printf("Lock esclusiva\n");
        else{
            if (result == EALREADY) {
                
                printf("Sono occupato\n");
            } else {
                printf("Impossibile lock file\n");
            }
        }
    }
    return 0;
}