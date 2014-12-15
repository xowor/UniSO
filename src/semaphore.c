/* Decrements semaphore */
int sem_p (int semid, int semnum) {
    struct sembuf cmd;
    cmd.sem_num = semnum;
    cmd.sem_op = -1;
    cmd.sem_flg = 0;
    semop(semid, &cmd, 1);
}

/* Increments semaphore */
int sem_v (int semid, int semnum) {
    struct sembuf cmd;
    cmd.sem_num = semnum;
    cmd.sem_op = 1;
    cmd.sem_flg = 0;
    semop(semid, &cmd, 1);
}
