#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

sem_t salle_attente, mutex_medecin, mutex_medecin2, mutex_radio, protege_priorite, prive_radio;
int priorite = 0; // No one in the radio room initially

// Thread function for each patient
void *patient_thread(void *arg) {
    int patient_id = *(int *)arg;
    int patient_attent_sontour = rand() % 10 + 1; // Random waiting time for the patient

    sem_wait(&salle_attente); // Enter the waiting room (limit to N patients)

    printf("Patient %d is in the waiting room\n", patient_id);
    sleep(patient_attent_sontour); // Patient waits for their turn
    sem_wait(&mutex_medecin); // Access the general practitioner's consultation room
    printf("Patient %d entered the general practitioner's room\n", patient_id);

label:
    sleep(patient_attent_sontour); // Duration of the consultation
    int choix_cons = rand() % 3 + 1; // General practitioner's decision (1: leave, 2: radio, 3: specialist)

    sem_wait(&protege_priorite); // Protect priority handling
    if (priorite == 0) { // No one in the radio room
        sem_post(&mutex_medecin); // Release the general practitioner's consultation room
        sem_post(&protege_priorite); // Allow others to access priority
    } else {
        sem_post(&prive_radio); // Give priority to the radio room
    }

    // If the decision is to leave with a prescription
    if (choix_cons == 1) {
        printf("Patient %d leaves with a final prescription\n", patient_id);
        pthread_exit(NULL); // Exit the thread
    }
    // If the decision is to go to the radio room
    else if (choix_cons == 2) {
        sem_wait(&mutex_radio); // Request access to the radio room
        sleep(3); // Duration for the X-ray
        priorite = 1; // Set priority for the radio room
        sem_post(&mutex_radio); // Release the radio room
        sem_wait(&prive_radio); // Wait for the consultation room to be free and prioritize
        goto label; // Return to the consultation step
    }
    // If the decision is to go to the specialist
    else if (choix_cons == 3) {
        printf("Patient %d entered the specialist's room\n", patient_id);
        sem_wait(&mutex_medecin2); // Access the specialist's consultation room

label1:
        sleep(patient_attent_sontour); // Duration of the consultation
        int choix_cons_spec = rand() % 2 + 1; // Specialist's decision (1: leave, 2: radio)

        sem_wait(&protege_priorite); // Protect priority handling
        if (priorite == 0) { // No one in the radio room
            sem_post(&mutex_medecin2); // Release the specialist's consultation room
            sem_post(&protege_priorite); // Allow others to access priority
        } else {
            sem_post(&prive_radio); // Give priority to the radio room
        }

        // If the decision is to leave with a prescription
        if (choix_cons_spec == 1) {
            printf("Patient %d leaves with a final prescription from the specialist\n", patient_id);
            pthread_exit(NULL); // Exit the thread
        }
        // If the decision is to go to the radio room
        else {
            sem_wait(&mutex_radio); // Request access to the radio room
            sleep(3); // Duration for the X-ray
            priorite = 1; // Set priority for the radio room
            sem_post(&mutex_radio); // Release the radio room
            sem_wait(&prive_radio); // Wait for the consultation room to be free and prioritize
            goto label1; // Return to the consultation step with the specialist
        }
    }

    sem_post(&salle_attente); // Release the waiting room
    pthread_exit(NULL); // End the thread
}

int main() {
    int N = 15; // Maximum number of patients allowed in the waiting room
    int max_client = 40; // Simulate the number of threads (patients)
    
    // Initialize semaphores
    sem_init(&salle_attente, 0, N); // Initialize waiting room with N slots
    sem_init(&mutex_medecin, 0, 1); // Lock access to the general practitioner's consultation
    sem_init(&mutex_radio, 0, 1); // Allow only one person in the radio room at a time
    sem_init(&protege_priorite, 0, 1); // Protect access to the priority variable
    sem_init(&prive_radio, 0, 0); // Wait for the patient in consultation to leave and give priority to those coming from the radio room
    sem_init(&mutex_medecin2, 0, 1); // Lock access to the specialist's consultation room
    
    // Create threads for each patient
    pthread_t threads[max_client];
    int patient_ids[max_client];

    for (int i = 0; i < max_client; i++) {
        patient_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, patient_thread, &patient_ids[i]); // Create patient threads
    }

    // Wait for all patient threads to finish
    for (int i = 0; i < max_client; i++) {
        pthread_join(threads[i], NULL); // Join patient threads
    }

    // Clean up semaphores
    sem_destroy(&salle_attente);
    sem_destroy(&mutex_medecin);
    sem_destroy(&mutex_radio);
    sem_destroy(&protege_priorite);
    sem_destroy(&prive_radio);
    sem_destroy(&mutex_medecin2);

    return 0; // Exit the program
}
