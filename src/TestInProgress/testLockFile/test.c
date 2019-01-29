#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <assert.h>


int write_descriptor=0;
int read_descriptor=0;

#define LOCKFILE "lockTest2.txt"
#define LOCK_FAIL 1
#define LOCK_SUCCESS 0
#define DB_FAIL 1
#define DB_SUCCESS 1

int lock_write() {
	assert ((read_descriptor == 0) && (write_descriptor == 0));
    if((write_descriptor = open(LOCKFILE, O_RDWR|O_CREAT,0644))<0) {
        return LOCK_FAIL;
    }

    if(flock(write_descriptor, LOCK_EX)<0) {
        close(write_descriptor);
        write_descriptor = 0;
        return LOCK_FAIL;
    }
    return LOCK_SUCCESS;
}

int unlock_write() {
    if(!write_descriptor) {
        // sanity: try to unlock before lock.
        return LOCK_FAIL;
    }

    if(flock(write_descriptor,LOCK_UN)<0) {
        // doing nothing because even if unlock failed, we
        // will close the fd anyway to release all locks.
    }
    close(write_descriptor);
    write_descriptor = 0;
    return LOCK_SUCCESS;
}


int lock_read() {
	assert ((read_descriptor == 0) && (write_descriptor == 0));
    if((read_descriptor = open(LOCKFILE,O_RDONLY))<0) {
        return LOCK_FAIL;
    }

    if(flock(read_descriptor, LOCK_SH)<0) {
	    close(read_descriptor);
	    read_descriptor = 0;
	    return LOCK_FAIL;
	}
    return LOCK_SUCCESS;
}

int unlock_read() {
    if(!read_descriptor) {
        // sanity : try to unlock before locking first.
        return LOCK_FAIL;
    }

    if(flock(read_descriptor, LOCK_UN)<0) {
        // doing nothing because even if unlock failed, we
        // will close the fd anyway to release all locks.
    }
    close(read_descriptor);
    read_descriptor = 0;
    return LOCK_SUCCESS;
}


int read_db() {
    if(lock_read() != LOCK_SUCCESS) {
        return DB_FAIL;
    }
    // read from db
    if(unlock_read() != LOCK_SUCCESS) {
        // close fd also unlock - so we can fail here (can i assume that ?)
    }
}

int write_db() {
    if(lock_write() != LOCK_SUCCESS) {
        return DB_FAIL;
    }
    //write to db.
    if(unlock_write() != LOCK_SUCCESS) {
        // close fd also unlock - so we can fail here (can i assume that ?)
    }
}

int main(int argc, char const *argv[])
{
	printf("%d\n", lock_read());
	printf("%d\n", read_db());
	/*
	printf("%d\n", unlock_read());
	printf("%d\n", read_db());
	printf("%d\n", lock_read());*/
	return 0;
}