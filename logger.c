#include "campus_compute.h"

static LogQueue log_q;

static void log_queue_init(LogQueue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->done = false;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

static void log_queue_push(LogQueue *q, const char *msg) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == LOG_QUEUE_CAPACITY) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    strncpy(q->messages[q->tail], msg, LOG_MAX_LINE - 1);
    q->messages[q->tail][LOG_MAX_LINE - 1] = '\0';
    q->tail = (q->tail + 1) % LOG_QUEUE_CAPACITY;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

static bool log_queue_pop(LogQueue *q, char *buffer) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0 && !q->done) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }
    if (q->count == 0 && q->done) {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }
    strncpy(buffer, q->messages[q->head], LOG_MAX_LINE - 1);
    buffer[LOG_MAX_LINE - 1] = '\0';
    q->head = (q->head + 1) % LOG_QUEUE_CAPACITY;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return true;
}

static void* producer_thread_func(void *arg) {
    long id = (long)arg;
    for (int i = 1; i <= 3; i++) {
        char msg[LOG_MAX_LINE];
        snprintf(msg, sizeof(msg), "[LOG-PRODUCER #%ld] Job event notification #%d dispatched", id, i);
        printf(" -> %s\n", msg);
        log_queue_push(&log_q, msg);
        usleep(30000);
    }
    pthread_exit(NULL);
}

typedef struct {
    char filepath[256];
} ConsumerArg;

static void* consumer_thread_func(void *arg) {
    ConsumerArg *c_arg = (ConsumerArg*)arg;
    FILE *fp = fopen(c_arg->filepath, "a");
    if (!fp) {
        perror("fopen log file");
        pthread_exit(NULL);
    }

    char item[LOG_MAX_LINE];
    while (log_queue_pop(&log_q, item)) {
        printf("    <- [LOGGER-CONSUMER] Flushed to disk: %s\n", item);
        fprintf(fp, "%s\n", item);
        fflush(fp);
        usleep(15000);
    }
    fclose(fp);
    pthread_exit(NULL);
}

void run_concurrent_logger_demo(const char *log_filepath) {
    printf("\n[Activity Logger] Initializing bounded queue (capacity=%d)...\n", LOG_QUEUE_CAPACITY);
    log_queue_init(&log_q);

    pthread_t prod1, prod2, cons;
    ConsumerArg c_arg;
    strncpy(c_arg.filepath, log_filepath, sizeof(c_arg.filepath) - 1);

    pthread_create(&cons, NULL, consumer_thread_func, &c_arg);
    pthread_create(&prod1, NULL, producer_thread_func, (void*)1);
    pthread_create(&prod2, NULL, producer_thread_func, (void*)2);

    /* Await producer completion */
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);

    /* Signal completion to consumer */
    pthread_mutex_lock(&log_q.mutex);
    log_q.done = true;
    pthread_cond_signal(&log_q.not_empty);
    pthread_mutex_unlock(&log_q.mutex);

    pthread_join(cons, NULL);

    pthread_mutex_destroy(&log_q.mutex);
    pthread_cond_destroy(&log_q.not_empty);
    pthread_cond_destroy(&log_q.not_full);

    printf("[Activity Logger] Both producer threads and consumer thread terminated cleanly.\n");
    printf("[Activity Logger] Log file written to: %s\n", log_filepath);
}