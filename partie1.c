#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

sem_t salle_attente, mutex_medecin, mutex_radio, protege_priorite, prive_radio;
int priorite = 0; // no one in the radio room

void *patient_thread(void *arg) {
    int patient_id = *(int *)arg;
    int patient_attent_sontour = rand() % 10 + 1; // Random waiting time for the patient
    
    sem_wait(&salle_attente); // The patient enters the waiting room

    printf("Patient %d is in the waiting room\n", patient_id);
    sleep(patient_attent_sontour); // The patient waits for their turn
    sem_wait(&mutex_medecin);

label:
    sleep(patient_attent_sontour); // Duration of the consultation
    int choix_cons = rand() % 2 + 1; // Doctor's decision (1 for leaving, 2 for radio)

    sem_wait(&protege_priorite);
    if (priorite == 0) { // No one in the radio room
        sem_post(&mutex_medecin);
        sem_post(&protege_priorite);
    } else {
        sem_post(&prive_radio); // Give priority to the radio room
    }

    if (choix_cons == 1) { // Leave with a prescription and exit
        printf("Patient %d leaves with a final prescription\n", patient_id);
        pthread_exit(NULL);
    } else { // Direct the patient to the radio room
        sem_wait(&mutex_radio); // Request access to the radio room
        sleep(3); // Time for the X-ray
        priorite = 1; // Set priority for the radio room
        sem_post(&mutex_radio);
        sem_wait(&prive_radio); // Wait for the consultation room to become free and take priority
        goto label; // Go back to the consultation step
    }

    sem_post(&salle_attente); // Free the waiting room
    pthread_exit(NULL);
}

int main() {
    int N = 15;
    int max_client = 40; // Simulate the number of threads (patients)
    sem_init(&salle_attente, 0, N); // Initialize the waiting room with N slots
    sem_init(&mutex_medecin, 0, 1); // Lock access to the doctor's consultation
    sem_init(&mutex_radio, 0, 1); // Allow only one person at a time in the radio room
    sem_init(&protege_priorite, 0, 1); // Protect access to the priority variable
    sem_init(&prive_radio, 0, 0); // Wait for the patient in consultation to leave, giving priority to those coming from the radio room

    // Create threads for each patient
    pthread_t threads[max_client];
    int patient_ids[max_client];

    for (int i = 0; i < max_client; i++) {
        patient_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, patient_thread, &patient_ids[i]);
    }

    // Wait for all patients to finish their work
    for (int i = 0; i < max_client; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up semaphores
    sem_destroy(&salle_attente);
    sem_destroy(&mutex_medecin);
    sem_destroy(&mutex_radio);
    sem_destroy(&protege_priorite);
    sem_destroy(&prive_radio);

    return 0;
}
