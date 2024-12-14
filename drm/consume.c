void* consumer(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        // Attendre si le tampon est vide
        while (count == 0) {
            pthread_cond_wait(&cond_empty, &mutex);
        }

        // Retirer l'élément du tampon
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consommateur: consomme %d\n", item);

        // Signaler qu'il y a de la place disponible
        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);

        sleep(1); // Simule un délai de consommation
    }
    return NULL;
}
